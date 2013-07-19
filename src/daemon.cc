/* daemon.cc
   Component Source file for the Dilithium Program.
*/
/**
 * @par dilithium - GPL Laucher for FVWM-CRYSTAL
 * 
 * Copyright (C) 2013 Wiley Edward Hill
 * Copyright (C) 2013 dilithium Contributors (see ChangeLog for details)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110
 * -1301 USA
 */

#include "common.h"

#include <sys/resource.h>     /* resolve_descriptors: rlimit RLIMIT_* */
#include <fcntl.h>            /* resolve_descriptors: open, O_RDWR*/

#include <fstream>

#include <spawn.h>
#include <signal.h>

#include "global.h"
#include "privileges.h"
#include "dilithium.h"
#include "xlogin.h"
#include "daemon.h"

static sig_atomic_t g_eflag;
static sig_atomic_t g_hupflag;

char  daemon_buffer[256];
char *ptr_daemon_buffer = &daemon_buffer[0];

/*! \brief Daemon Log Wrapper
 *  \par Function Description
 *  This function is the only daemon class function to write to the system
 *  log. The function opens and close the log to insure the syslog is not
 *  open when accessing shared library routines.
 */
void Daemon::LogMsg (int priority, char* message, char* str1=NULL, char* str2=NULL)
{
  /* Set our Logging Mask and open the Log */
  setlogmask(LOG_UPTO(LOG_INFO));

  openlog(DAEMON_NAME, LOG_DAEMON | LOG_SYSLOG | LOG_PERROR | LOG_PID, LOG_USER);

  if (str2 != NULL) {
    syslog (priority, message, str1, str2);
  }
  else if (str1 != NULL) {
    syslog (priority, message, str1);
  }
  else {
    syslog (priority, "%s", message);
  }

  closelog ();
}

/*! \brief Daemon Log Wrapper Over-load for Integer Formatting 
 *  \par Function Description
 *  This function applies the format for in interger argument using
 *  the snprintf function and passes the resulting composite string
 *  to the main LogMsg function.
 */
void Daemon::LogMsg (int priority, char* message, int num) {
  snprintf ( ptr_daemon_buffer, sizeof(ptr_daemon_buffer), message, num);
  LogMsg(priority, ptr_daemon_buffer);
}

/*! \brief Daemon Resolve File Descriptors
 *  \par Function Description
 *  Normally a daemon function ditches all of it's file descriptors.
 *  The main reason for this seems to be insecurity.
 *
 *  Unfortunately, we don't use this. The posix_spawn function used to
 *  spawn xinit is compelled to have our child inherit our descriptors.
 *  This might be "handy" if we were trying to control xinit but other
 *  wise prevents us from modifying our descriptors, aka sucks.
 *
 * \retval EXIT_SUCCESS
 */
int Daemon::resolve_descriptors  (int flags) {

  struct rlimit       rl;
  int                 i, maxfd, fd;

  if (!(flags & BD_NO_CLOSE_FILES)) { /* Close all open files */
    /* Get maximum number of file descriptors. */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
      maxfd = sysconf(_SC_OPEN_MAX);
      if (maxfd == -1)                /* Limit is indeterminate... */
          maxfd = BD_MAX_CLOSE;       /* so take a guess */

      for (fd = 0; fd < maxfd; fd++)
           close(fd);
    }
    else { /* Close all open file descriptors. */

      if (rl.rlim_max == RLIM_INFINITY)
          rl.rlim_max = BD_MAX_CLOSE;

      for (i = 0; i < rl.rlim_max; i++)
          close(i);
    }
  }

  if (!(flags & BD_NO_REOPEN_STD_FDS)) {
    close(STDIN_FILENO);            /* Reopen standard fd's to /dev/null */

    fd = open("/dev/null", O_RDWR);

    if (fd != STDIN_FILENO)         /* 'fd' should be 0 */
      LogMsg (LOG_NOTICE, "Dilithium: bad descriptor for stdin, continuing.");
    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
      LogMsg (LOG_NOTICE, "Dilithium: bad descriptor for stdout, continuing.");
    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
      LogMsg (LOG_NOTICE, "Dilithium: bad descriptor for stderr, continuing.");
  }

  return EXIT_SUCCESS;
}

/*! \brief Daemon Spawn XINIT passing collated paramaters
 *  \par Function Description
 *  This function compiles all the command-line parameters for the xinit
 *  process and starts xinit. Unlike us, xinit's parameters are very much
 *  positional and not a single parameter group can be out of order. Both
 *  the client and the server must begin with a forward slash. argv's are
 *  interpreted literally so no 2 strings can be together, For example
 *  "logverbose 7" must be two separate arguments, additionally the server
 *  arguments must be delimited by a leading "--" with no extra spaces any
 *  where, it ain't Bash -- it's posix_spawn. The arguments could vary from
 *  none to many. X11 currently accept 106 different command-line switches.
 *  Hence, we create vector arrays for both the client and server arguments
 *  then we linearize all arguments to a dynamically allocated array of
 *  strings, and give our child 10 seconds to initialize before we free the
 *  memory. Say ugly!
 *
 * \retval r_pid pid_t, aka unsigned integer id of the new process
 */
pid_t Daemon::spawn_gui(void) {

  pid_t r_pid;
  int status;
  int index;
  int i;
  int number_of_client_args;
  int number_of_server_args;
  int number_of_buffers;
  char **argv;

  std::vector<std::string> ca_tokens;
  std::vector<std::string> sa_tokens;

  number_of_client_args = word_count( dilithium->xclientargs);
  number_of_server_args = word_count( dilithium->xserverargs);

  number_of_buffers = 6 + number_of_client_args + number_of_server_args ;

  argv = (char**) malloc(number_of_buffers * sizeof(char *));

  if ( argv == NULL ) {
    syslog (LOG_NOTICE, "(Memeory allocation error, aborting");
    return -1;
  }

  index = 0;
  argv[index++] = (char *) "xinit";
  argv[index++] = (char *) dilithium->xclient;

  if ( !dilithium->empty(dilithium->xclientargs )) {
    if ( number_of_client_args > 1 ) {
      Tokenize(dilithium->xclientargs, ca_tokens);
      for ( std::vector<std::string>::iterator i  = ca_tokens.begin();
                                               i != ca_tokens.end(); i++ ) {
       argv[index++] = (char*) i->c_str();
      }
    }
    else {
      argv[index++] = (char *) dilithium->xclientargs;
    }
  }

  argv[index++] = (char *) "--";

  argv[index++] = dilithium->xserver;

  argv[index++] = dilithium->display;

  if ( !dilithium->empty(dilithium->xserverargs) ) {
    if ( number_of_server_args > 1 ) {
      Tokenize(dilithium->xserverargs, sa_tokens);
      for ( std::vector<std::string>::iterator i  = sa_tokens.begin();
                                               i != sa_tokens.end(); i++ ) {
       argv[index++] = (char *) i->c_str();
      }
    }
    else {
      argv[index++] = (char *) dilithium->xserverargs;
    }
  }

  argv[index++] = (char *) NULL;

  fflush(NULL);

  if ( DebugMode ) {
    syslog (LOG_NOTICE, "(DEBUG+) Dilithium posix_spawn: 0=%s 1=%s 2=%s 3=%s 4=%s =5%s 6=%s",
                        argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
  }

  dilithium->drop_privileges();

  status = posix_spawnp(&r_pid, "xinit", NULL, NULL, argv, environ);

  if ( status != 0) {
     syslog (LOG_NOTICE, "posix_spawn: %s", strerror(status));
     r_pid = 0;
  }
  else {
     if ( DebugMode || Verbose ) {
       syslog (LOG_NOTICE, "(DEBUG+) Dilithium posix_spawn returned 0");
     }
  }

  sleep(SERVER_BOOT_DELAY);    // Sleep for 10 seconds

  free (argv);  // Free the array of command-line arguments

  return r_pid;
}

/*! \brief Daemon Remove PID File
 *  \par Function Description
 *  This function deletes the PID file that was created when we started. This
 *  is done before the daemon terminates.
 *
 *  \note: The global function remove_pid_file defined above is used to delete
 *  stale PID files, this function delete a specific file: Daemon::pidfile.
 */
void Daemon::remove_pidfile() {
  struct stat sb;
  if (stat(pidfile.c_str(), &sb) != -1) {
    if (remove ( pidfile.c_str() ) != 0) {
      syslog (LOG_NOTICE, "Dilithium: Error removing PID File, %s", strerror(errno));
    }
  }
}

/*! \brief Daemon Get Process Status
 *  \par Function Description
 *  This function reads the state field in the file /proc/PID/stat and returns
 *  the associated char to the caller. If  the file does not exist or there is
 *  and error reading the file we return De 'N' ada, to be interpreted as X is
 *  Dead.
 *
 *  \note: The is another reason we do not want to give-up our file
 *  descriptors. We would otherwise have to stream commands to a shell on
 *  regular intervals in order to get this information.
 *
 *  \retval state = One character from the string "RSDZTW" where R is running,
 *  S is sleeping in an interruptible wait, D is waiting in uninterruptible
 *  disk sleep, Z is zombie, T is traced or stopped (on a signal), and W is
 *  paging, or N as previously described.
 */
char Daemon::get_process_status (pid_t qid)
{

  char str_id[6];

  int  pid;
  char state;
  int  n;
  char name[64];

  FILE *input;
  std::string filename = "/proc/";

  struct stat sb;

  sprintf (&str_id[0], "%d", qid);
  filename.append(&str_id[0]);
  filename.append("/stat");

  if ( stat (filename.c_str(), &sb) != -1) {
    if (( input = fopen( filename.c_str(), "r")) != NULL) {
      n = fscanf(input, "%d ", &pid);
      n = fscanf(input, "%s ", &name[0]);
      n = fscanf(input, "%c ", &state);
      fclose(input);
    }
    else {
      syslog (LOG_NOTICE, "Could not open stat file, <%s>", filename.c_str());
    }
  }
  else {
    state = 'N'; /* N = No = Not Alive, X is Dead */
  }

  return state;
}

/*! \brief Daemon Idle
 *  \par Function Description
 *  This function calls the spawn function and then enters an infinite loop.
 *  The loop is only terminated when either the SIGTERM signal is received
 *  or the get_proccess_status function reports X is dead or is a zombie.
 *  The loop sleeps for (60) IDLE_SLEEP_SECONDS, and pools for the status of
 *  xinit. Once out of the loop, this procedure will kill the xinit process,
 *  which will also kill X, if the KillX variable is true and xinit is alive,
 *  as indicated by a non-zero pid. remove_pidfile is called to remove the
 *  current PID file and then the deamon thread is terminated with the final
 *  statement of this procedure.
 *
 */
void Daemon::Idle() {

  pid_t x_pid;
  pid_t c_pid;
  char status;
  int state;

  LogMsg(LOG_NOTICE, "Entering Daemon Mode");

  x_pid = spawn_gui();
  fflush(NULL);

  if (x_pid > 0) {

    LogMsg(LOG_NOTICE, "Dilithium Daemon spawned process <%d>", x_pid);

    /* ---------------- Main Process ----------------*/
    while(!g_eflag) {
      if(g_hupflag) {
         g_hupflag = 0;
         LogMsg(LOG_INFO, "Dilithium Daemon got hupflag, nothing to do");
      }

      status = get_process_status(x_pid);
      switch(status) {
      case 'S':
      case 'R':
        if ( DebugMode ) {
          LogMsg (LOG_NOTICE, "(DEBUG+) Dilithium X is Alive");
        }
        break;
      case 'Z':
        if ( DebugMode ) {
          ShowMessage ("(DEBUG+) Dilithium X is a Zombie");
        }
        // Look Mom, No breaks!
      case 'N':
        LogMsg(LOG_NOTICE, "(DEBUG+) Dilithium X is dead");
        x_pid = 0;
        g_eflag = true;
        break;
      default:
        if ( DebugMode || Verbose ) {
          ShowMessage("(DEBUG+) Dilithium get_process_status unknown=%c", status);
        }
        break;
      }
      if (!g_eflag) {
        sleep(IDLE_SLEEP_SECONDS);    //Sleep for 60 seconds
      }
    }
  }

  if ( KillX && ( x_pid > 0 )) {
    if (kill(x_pid, SIGTERM) == 0) {
      LogMsg(LOG_NOTICE, "Dilithium: terminated X");
    }
    else {
      LogMsg(LOG_NOTICE, "Dilithium: could not terminate the X server");
    }
  }

  if ( DebugMode || Verbose ) {
    ShowMessage("(DEBUG+) Dilithium: Begin termination");
  }

  if (created_pidfile ) {
    remove_pidfile();
  }

  if ( DebugMode || Verbose ) {
    ShowMessage("(DEBUG+) Dilithium: exit");
  }

  delete this;
}

/*! \brief Daemon Signal Handler
 *  \par Function Description
 *  Is the ONLY static member of the class and as such is not allowed
 *  to use, access, or refer to any other non-static members of the
 *  class, seems like a lame restriction imposed by C++, which assumes
 *  there could be another instance of the class. We let our daemon
 *  classmates know signals were received by using global variables
 *  because these particular variables are of type sig_atomic_t, which,
 *  by nature, MUST also be static. So they can not be members of the
 *  class or the rest of the class would not be able to access them.
 *  The best plans of Men and Mice, mime's name Brain, he's an escapee.
 */
void Daemon::signal_handler(int sig)
{
    switch(sig)
    {
    case SIGHUP:
        g_hupflag = 1;
        break;
    case SIGINT:
    case SIGTERM:
        g_eflag = 1;
        break;
   }
}

/*! \brief Daemon Become a Daemon
 *  \par Function Description
 *  This function creates a new process and forks the program from the
 *  original process.
 *
 *  \retval EXIT_FAILURE is ambiguously returned to the original process
 *                       if the fork is unsuccessful.
 *  \retval EXIT_SUCCESS is also returned to the new process if a new
 *                       process was created.
 */
int Daemon::becomeDaemon(int flags) {   /* Returns 0 on success, -1 on error */

    int maxfd, fd;

    switch ((pid = fork())) {           /* Become background process */
    case -1: return EXIT_FAILURE;
    case 0:  break;                     /* Child falls through... */
    default: _exit(EXIT_SUCCESS);       /* while parent terminates */
    }

    sid = setsid();             /* Create a new Signature Id for our child */

    if (sid < 0) {
     exit(EXIT_FAILURE);
    }

    switch (fork()) {                   /* Ensure we are not session leader */
    case -1: return EXIT_FAILURE;
    case 0:  break;
    default: _exit(EXIT_SUCCESS);
    }

    if (!(flags & BD_NO_UMASK0))
        umask(0);                       /* Clear file mode creation mask */

    if (!(flags & BD_NO_CHDIR)) {       /* Change to root directory */

      /* Change the current working directory to the root so
       * we won't prevent file systems from being unmounted. */
      if (chdir("/") < 0)
      {
        LogMsg(LOG_NOTICE, "Dilithium, Can't change directory to /");
      }
    }

    return EXIT_SUCCESS;
}

/*! \brief Daemon Create PID File
 *  \par Function Description
 *  This function first attempts to creates a new file in the /var/run
 *  directory, if this is not successful and attempt is made to create
 *  the file in the current user's home directory. The PID information
 *  is written to the file.
 *
 *  \retval true if a PID file was created, otherwise false
 *
 */
bool Daemon::CreateIdFile(bool flag) {

   bool result;

   uid_t tp_uid;
   gid_t tp_gid;

   std::ofstream out;

   if (flag == true) {
     tp_uid = geteuid();
     tp_gid = getegid();
   }
   else {
     tp_uid = getuid();
     tp_gid = getgid();
   }

   result  = true;

   out.open(pidfile.c_str());
   if (!out) {
     LogMsg (LOG_NOTICE, "Dilithium: Could not create pid file, <%s> %s",
             (char*) pidfile.c_str(), strerror(errno));
     pidfile = getenv("HOME"); /* Switch to the home folder*/
     pidfile.append ( "/" );
     pidfile.append ( PIDFILE );
     out.open(pidfile.c_str());
     if (!out) {
       LogMsg (LOG_NOTICE, "Dilithium: Could not create a pid file in user directory, <%s> %s",
                            (char*) pidfile.c_str(), strerror(errno));
       result  = false;
     }
   }

   if (out) {
       chmod(pidfile.c_str(), 0644);

       if (chown(pidfile.c_str(), tp_uid, tp_gid) != 0) {
         LogMsg (LOG_NOTICE, "Dilithium: Could not change owner of pid file");
       }
       out << getpid() << '\n';
       out.close();
   }

  return result;
}

/*! \brief Daemon Class Initialization
 *  \par Function Description
 *  This function is just a sub-routine for the contructor to set
 *  the inital value of all variables, including off-loading the
 *  values in the structure options to class members.
 *
 */
void Daemon::initialize() {

  setlogmask(LOG_UPTO(LOG_NOTICE));
  openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);

  sig_atomic_t g_eflag   = 0;
  sig_atomic_t g_hupflag = 0;

  pid    = -1;
  sid    = -1;

  pidfile = PIDPATH;
  pidfile.append ( PIDFILE );

}

/*! \brief Daemon Class Constructor
 *  \par Function Description
 *  This is the constructor for the Daemon class. The construction calls
 * initialize() to set variables, then becomeDaemon. If becomeDaemon is
 * not successful the program is terminated. If program successful transforms
 * the a PID file is created and the signal handle is setup.
 *
 */
Daemon::Daemon (Dilithium *d) {

  struct sigaction sa;
  dilithium = d;

  initialize();

  if (becomeDaemon(0) == -1) {
     ErrorMessage("can not become a Daemon");
     delete this;
  }
  created_pidfile = CreateIdFile(false);

  sig_atomic_t g_hupflag = 1;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags   = SA_RESTART;
  sa.sa_handler = signal_handler;

  signal(SIGINT,  signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGHUP,  signal_handler);
  signal(SIGPIPE, SIG_IGN);

  if (sigaction(SIGHUP, &sa, NULL) == -1) {
     if (created_pidfile ) {
         remove_pidfile();
     }
     ErrorMessage("can not setup signal handler");
     delete this;
  }

}
