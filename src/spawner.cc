
#include "common.h"
#include "global.h"
#include "privileges.h"
#include "dilithium.h"
#include "xlogin.h"
#include "spawner.h"

#include <sys/resource.h>

static char *clientargv[50];
static char *serverargv[100];

volatile int gotSignal = 0;

static void sigCatch(int sig)
{
  /* On system with POSIX signals, just interrupt the system call */
  gotSignal = sig;
}

static void sigIgnore(int sig)
{

}

int Spawner::start_client()
{
  const char *prog;
  const char *dp = "-d";
  char **argv;
  int    argc;
  int    index;

  std::vector<std::string> ca_tokens;

  cid = fork();

  switch(cid) {
  case 0:

    prog = basename(dilithium->xclient);

    if (DebugMode) {
      printf("username    =%s\n", dilithium->user_name.c_str());
      printf("user ID     =%d\n", getuid());
      printf("group ID    =%d\n", getgid());
      printf("xclient     =%s\n", dilithium->xclient );
      printf("xclientargs =%s\n", dilithium->xclientargs);
    }

    dilithium->drop_privileges();

    prog = basename(dilithium->xclient);

    /* +1 for prog, +2 for display , +1 for NULL = 3*/
    argc = word_count(dilithium->xclientargs); //+ 3;
    argv = serverargv;

    index = 0;
    argv[index++] = (char*) prog;
    argv[index++] = (char*) dp;
    argv[index++] = (char*) dilithium->display;

    if ( strlen(dilithium->xclientargs) != 0 ) {
      if ( argc > 1 ) {
        Tokenize(dilithium->xclientargs, ca_tokens);
        for ( std::vector<std::string>::iterator i  = ca_tokens.begin();
                                                 i != ca_tokens.end(); i++ )
        {
          argv[index++] = (char*) i->c_str();
        }
      }
      else {
        argv[index++] = (char *) dilithium->xclientargs;
      }
    }

    argv[index++] = (char *) NULL;

    fflush(NULL);

    execvp(argv[0], argv);

    ErrorMessage("Unable to run program \"%s\"", dilithium->xclient);

    return EXIT_FAILURE;
    break;
  case -1:
    ErrorMessage("Unable to run program \"%s\"", dilithium->xclient);
    break;
  default:
    errno = 0;
  } /* End Select Case */
  return cid;
}

/* return TRUE if we timeout waiting for pid to exit, FALSE otherwise. */
bool Spawner::process_timeout(int timeout, char *string)
{
  int status;
  int    i = 0, pidfound = -1;

  static char    *laststring;

    for (;;) {

        if ((pidfound = waitpid(sid, &status, WNOHANG)) == sid)
            break;
        if (timeout) {
            if (i == 0 && string != laststring)
                fprintf(stderr, "\r\nwaiting for %s ", string);
            else
                fprintf(stderr, ".");
            fflush(stderr);
            sleep(1);
        }
        if (++i > timeout)
            break;
    }
    if (i > 0) fputc('\n', stderr);     /* tidy up after message */
    laststring = string;

    return (sid != pidfound);
}

/*  waitforserver - wait for X server to start up */
bool Spawner::waitforserver( const char *displayNum)
{
  int    ncycles  = BOOT_TIME; /* # of cycles to wait */
  int    cycles;               /* Wait cycle count */

  for (cycles = 0; cycles < ncycles; cycles++) {
    if ((xd = XOpenDisplay(displayNum))) {

      return true;
    }
    else {
      if (!process_timeout(1, "X server to begin accepting connections"))
        break;
    }
  }

  ErrorMessage("giving up");

  return false;
}

static int
ignorexio(Display *dpy)
{
  ErrorMessage("connection to X server lost");
  return EXIT_SUCCESS;
}

int Spawner::start_server()
{
  sid = -1;
  const char *prog;
  char **argv;
  int    argc;
  int    index;

  std::vector<std::string> sa_tokens;

  sigset_t mask, old;

  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigprocmask(SIG_BLOCK, &mask, &old);

  sid = fork();

  switch(sid) {
  case 0:
    /* Unblock */
    sigprocmask(SIG_SETMASK, &old, NULL);

    /* don't hang on read/write to control tty */
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    /* ignore SIGUSR1 in child.  The server will notice this and send
     * SIGUSR1 back to diltihium when ready to accept connections */
    signal(SIGUSR1, SIG_IGN);

    /* +1 for prog, +1 for display , +1 for NULL = 3*/
    argc = word_count(dilithium->xserverargs) + 3;
    argv = serverargv;
    prog = basename(dilithium->xserver);

    index = 0;
    argv[index++] = (char*) prog;
    argv[index++] = (char*) dilithium->display;

    if ( strlen(dilithium->xserverargs) != 0 ) {
      if ( argc > 1 ) {
        Tokenize(dilithium->xserverargs, sa_tokens);
        for ( std::vector<std::string>::iterator i  = sa_tokens.begin();
                                                 i != sa_tokens.end(); i++ )
        {
          argv[index++] = (char*) i->c_str();
        }
      }
      else {
        argv[index++] = dilithium->xserverargs;
      }
    }

    argv[index++] = (char *) NULL;

    fflush(NULL);

    /* prevent server from getting sighup from vhangup()
     * if client is xterm -L */
    setpgid(0,getpid());

    execvp(argv[0], argv);

    ErrorMessage("unable to run server \"%s\"", dilithium->xserver);

    return (EXIT_FAILURE);

    break;
  case -1:
    break;
  default:
    /* don't nice server */
    setpriority(PRIO_PROCESS, sid, -1);

    errno = 0;
    if(! process_timeout(0, "")) {
      sid = -1;
      break;
    }

    /* Avoid race with TCP by giving the server time to set
     * socket options before we try to open it, either use
     * the 15 second timeout, or await SIGUSR1.
     *
     * If your machine is substantially slower than 5 seconds,
     * you can easily adjust this value.
     */
     alarm(SERVER_BOOT_DELAY);

     sigsuspend(&old);
     /* Cancel the alarm signal */
     alarm(0);

     sigprocmask(SIG_SETMASK, &old, NULL);

     if (waitforserver(dilithium->display) == 0) {
       ErrorMessage("unable to connect to X server");
       sid = -1;
     }

     break;
  }

  return(sid);
}

/*! \brief Spawner Shutdown X
 *  \par Function Description
 *  This is function is used to terminate the display manager and
 *  the X server.
 */
int Spawner::shutdown() {

  if (cid > 0) {

    XSetIOErrorHandler(ignorexio);

    /* HUP all local clients to allow them to clean up */
    if (killpg(cid, SIGHUP) < 0 && errno != ESRCH) {
      ErrorMessage("can't send HUP to process group %d", cid);
    }
  }

  if (sid < 0) {
    return sid;
  }
  else if (killpg(sid, SIGTERM) < 0) {
    if (errno == ESRCH)
      return EXIT_FAILURE;
    ErrorMessage("can't kill X server");
  }

  if (!process_timeout(SERVER_BOOT_DELAY, "X server to shut down")) {
    return EXIT_SUCCESS;
  }

  ErrorMessage("X server slow to shut down, sending KILL signal");

  if (killpg(sid, SIGKILL) < 0) {
    if (errno == ESRCH) {
      return EXIT_FAILURE;
    }
    ErrorMessage("can't SIGKILL X server");
  }

  if (process_timeout(2, "server to die")) {
     ErrorMessage("X server refuses to die");
  }

  return EXIT_SUCCESS;
}

int Spawner::do_spawn (bool wait_for_kill) {

  bool done;
  int exit_code;
  int pid;

  if (( sid = start_server()) > 0 ) {
    if ( dilithium->user_name.empty() ) {
      done = false;
      Xlogin *dialog = new Xlogin(dilithium);
      while (!done) {
        switch ( dialog->login()) {
          case Login:
            dilithium->run_mode == XLOGIN;
            if ( dilithium->initialize_user() == EXIT_SUCCESS ) {
              if (( cid = start_client()) > 0 ) {
                pid = -1;
                while (pid != cid && pid != sid && gotSignal == 0 ) {
                  pid = wait(NULL);
                }
              }
            }
            else {
               ErrorMessage("Error initializing user, Dilithium Terminating");
               done = true;
            }
            break;
          case Quit:
            done = true;
            break;
          case Reboot:
            exit_code = system("shutdown -r now");
            done = true;
            break;
          case Shutdown:
            exit_code = system("shutdown -h now");
            done = true;
            break;
          default:
            ShowMessage("login_widget, unknown response from Login dialog");
            done = true;
            break;
        }
      }
      delete dialog;
      exit_code = shutdown();
    }
    else if (( cid = start_client()) > 0 ) {

      if ( wait_for_kill && Verbose ) {
        syslog (LOG_NOTICE, "Waiting to kill server pid=<%d> and client pid=<%d>", sid, cid);
      }
      else {
        syslog (LOG_NOTICE, "X server pid=<%d> and client pid=<%d>", sid, cid);
      }

      if ( wait_for_kill ) {
        pid = -1;
        while (pid != cid && pid != sid && gotSignal == 0 ) {
          pid = wait(NULL);
        }
        exit_code = shutdown();
      }
    }
    else
      exit_code = cid;
  }
  else {
    exit_code = sid;
    fprintf(stderr, "server exited with exit code =<%d>\n", exit_code);
  }

  return exit_code;
}

void Spawner::set_up_signals( ) {

  signal(SIGCHLD, SIG_DFL);    /* Insurance */

  /* Let those signal interrupt the wait() call in the main loop */
  memset(&sa, 0, sizeof sa);
  sa.sa_handler = sigCatch;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;    /* do not set SA_RESTART */

  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  sigaction(SIGINT,  &sa, NULL);
  sigaction(SIGHUP,  &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);

  memset(&si, 0, sizeof(si));
  si.sa_handler = sigIgnore;
  sigemptyset(&si.sa_mask);
  si.sa_flags = SA_RESTART;

  sigaction(SIGALRM, &si, NULL);
  sigaction(SIGUSR1, &si, NULL);

}

/*! \brief Spawner Class Initialization
 *  \par Function Description
 *  This function is just a sub-routine for the contructor to set
 *  the inital value of all variables, including off-loading the
 *  values in the structure options to class members.
 *
 */
void Spawner::initialize() {

  setlogmask(LOG_UPTO(LOG_NOTICE));
  openlog(THIS_PROGRAM, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);

  xd = NULL;
}

Spawner::~Spawner () {
  signal(SIGTERM, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGINT,  SIG_IGN);
  signal(SIGHUP,  SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
}

/*! \brief Daemon Class Constructor
 *  \par Function Description
 *  This is the constructor for the Daemon class. The construction calls
 * initialize() to set variables, then becomeDaemon. If becomeDaemon is
 * not successful the program is terminated. If program successful transforms
 * the a PID file is created and the signal handle is setup.
 *
 */
Spawner::Spawner (Dilithium *d) {

  dilithium = d;

  initialize();

  set_up_signals();

}
