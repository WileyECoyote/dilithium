/* dilithium.cc
   Main Source file for the Dilithium Program.
*/
/**
 * @par dilithium - GPL Launcher for FVWM-CRYSTAL
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
/** \defgroup Dilithium GPL Launcher for FVWM-CRYSTAL
 *
 * \brief and other display managers
 */
#include "common.h"
#include <fstream>

#include "global.h"
#include "privileges.h"
#include "dilithium.h"
#include "xlogin.h"
#include "daemon.h"
#include "spawner.h"
#include "xauthxx.h"


#include "console.h"

float Version = 1.0;

extern char **environ;

char *user_buffer;  /* Dynamically allocated for strings in pwd  */

std::ofstream logfile;

const char * const shells[] = {

/* Shells */
    "sudo",
    "bash",
    "sh",
    "ash",
    "ksh",
    "zsh",
    "dash",
    NULL
};

const char * const inits[] = {
    "init",
    "start-stop-daemon",
    NULL
};

/*! \brief At Exit Function
 *  \par Function Description
 *  This function is call when the program terminates. The function
 *  releases memory allocated by the main prgram.
 */
void exit_funtion (void)
{
  if (user_buffer != NULL ) {
    free(user_buffer);
    user_buffer = NULL;
  }
}

void Write2Log(const char* str) {
    logfile << str << std::endl;
}
/*! \brief Create X Authority File
 *  \par Function Description
 *  This function creates an X authority file in the user home directory
 *  if such a file does not already exist.
 */
bool make_authority(char *xauthfile, char *sdisplay) {

  int idisplay;

  idisplay = atoi(sdisplay);

  try {
        Xau::XauthList auth_list;
        auth_list.load_from_file();

        Xau::MagicCookie cookie;
        Xau::Display display(idisplay);

        /* Use Compact Scheme */
        auth_list.push_back(Xau::Xauth(Xau::LocalAddress(), idisplay, cookie));
        auth_list.push_back(Xau::Xauth(Xau::InternetAddress(127,0,0,1), idisplay, cookie));

        auth_list.write_to_file(xauthfile);

        Xau::Xauth auth;

  }
  catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      return false;
  }
  return true;
}

/*! \brief Show Help function
 *  \par Function Description
 *  This function displays parameter options.
 */
void ShowHelp(void) {
  printf("\nDilithium: A launcher for fvwm-crystal\n");
  printf("\nDilithium parameter options:\n\n");
  printf("   :, --display Specify the display to be used for the X server\n");
  printf("  -c, --client  Specify the name of the display manager to be passed to xinit\n");
  printf("  -a, --cargs   optional arguments to be passed to the display manager\n");
  printf("  -s, --server  Specify the X server to be passed to xinit\n");
  printf("  -x  --xargs   Optional arguments to be passed to the X server\n");
  printf("  -U, --user    Optional User name will be used to setup the enviroment settings.\n");
  printf("  -h, --help    Show information about the advanced usage of this command.\n");
  printf("  -q  --quite   Suppress output messages\n");
  printf("  -u, --usage   Displays usage examples on usage.\n");
  printf("  -V, --version Displays software version information.\n");
  printf("  -v, --verbose Display additional information during processing\n");
  printf("\nDaemon options:\n\n");
  printf("      --daemon  Transform to daemon mode.\n");
  printf("      --no-drop Do not drop privileges.\n");
  printf("      --no-kill Do not kill xinit process when exiting.\n");
  printf("      --no-log  Do not create a seperate log, use syslog.\n");
  printf("      --logfile Specify the name log file, default <%s>\n", DILITHIUM_LOGFILE);
  printf("      --xinit   Use xinit to launch X with the client.\n");
  printf("\nNote: All parameters and arguments are case sensitive\n\n");
}

/*! \brief Show Usage function
 *  \par Function Description
 *  This function displays command-line syntax.
 */
void ShowUsage(char **argv) {

  printf("usage: dilithium [options] [-c <client> [-a <client arguments> ]] [-s <xserver> [-x <xserver arguments> ]]\n");
  printf("       try dilithium without any arguments or see man (1) for more information about dilithium\n");
  printf("example: dilithium -v --client /usr/bin/fvwm-crystal --server /usr/bin/Xorg '-nolisten tcp vt8' :4\n");
}

/*! \brief Set X Authority Environment Variable
 *  \par Function Description
 *  This function establishes an authority file and sets the environment
 *  variable XAUTHORITY to point to the file.
 *
 *  \retval true if an XAUTHORITY was established, otherwise false.
 */
bool set_authority ( Dilithium *d ) {

  bool result;

  char *xauthfile;
  char const *dir;
  char const *tmp_dir = "/tmp";

  std::string authority;

  result = true;

  if ( d->userinfo != NULL ) {       /* if there is a user name */
    dir = d->userinfo->pw_dir;       /* then try home folder    */
  }
  else if (getenv("TMPDIR")) { /* else if there is environment variable */
    dir = getenv("TMPDIR");      /* then try environment variable value */
  }
  else {
    dir = tmp_dir;                        /* else just try the constant */
  }

  xauthfile = getenv("XAUTHORITY");

  if ( !xauthfile ) {                           /* 1. Check Environment */
   /* if there is not already an entry then check the comman-line first */
    if ( d->empty(d->xauthority )) {
      authority = dir;
      authority.append("/.Xauthority");     /* 2.folder determine above */
    }
    else {
      authority = d->xauthority;             /* 3. command-line */
    }

    /* 4. Check THE folder determined above and see if Xauthority exist */
    if (access( (char*) authority.c_str(), F_OK) == NO_ERROR) { 
      xauthfile = (char*) authority.c_str();
    }
    else {                                 /* 5. The file did not exist */
      if ( dir == NULL ) {
        if ( chdir(dir) == NO_ERROR ) {   /* 6. Change to THE folder   */
                                          /* 7 Create a new authority file */
          if (make_authority( (char*) authority.c_str(), d->display)) {
            ShowMessage("Created new X-authority");
            xauthfile = (char*) authority.c_str();
            result = true;
          }
          else {
            ShowMessage("make X authority returned: negative response");
            xauthfile = NULL;
          }
        }
        else {
         ShowMessage("Could not change to <%s>", (char*) dir);
        }
      }
      else if ( Verbose || DebugMode ) {
        ShowMessage("<set_authority> No Directory for Xauthority");
      }
    }

    /* How ever we got here if <xauthfile> has a value then it contains the
     * name of an X authority file, so create an entry in the environment */
    if ( xauthfile != NULL ) {
      if ( (setenv("XAUTHORITY",  xauthfile, 1)) != NO_ERROR) {
         ErrorMessage("setting environment variable XAUTHORITY=%s", xauthfile);
         result = false;
      }
      else {
        if ( Verbose || DebugMode ) {
          d->set ( d->xauthority, xauthfile );
          ShowMessage("set environment variable XAUTHORITY=%s", xauthfile);
        }
      }
    }
  }
  else if ( Verbose ) {
    ShowMessage("authority variable already set in environment, <%s>", xauthfile);
  }

  return result;
}

/*! \brief Clear Environment Variables
 *  \par Function Description
 *  This function clears residues variables from the environment.
 */
void clear_environment ( void ) {
   /* Clear environment variables" */
   if ( Verbose && !BeQuite ) {
     ShowMessage("Clearing environment variables");
   }
   setenv("DBUS_SESSION_BUS_ADDRESS", "", 1);
   setenv("SESSION_MANAGER",          "", 1);
}

/*! \brief Set Environment Variables
 *  \par Function Description
 *  This function set environment variables based on command-line
 *  options and the userinfo structure.
 */
void set_environment( passwd *userinfo ) {
   if ( Verbose && !BeQuite ) {
     ShowMessage("Setting environment variables");
   }
   setenv("USER",     userinfo->pw_name,  1);
   setenv("USERNAME", userinfo->pw_name,  1);
   setenv("LOGNAME",  userinfo->pw_name,  1);
   setenv("HOME",     userinfo->pw_dir,   1);
   setenv("SHELL",    userinfo->pw_shell, 1);

}

/*! \brief Get Process Name
 *  \par Function Description
 *  This function retrieves the name of the process identified
 *  by the qid argument. If successful the result are copies to
 *  the supplied text buffer. If the routine is not successful
 *  the letter "N" is returned and the buffer is not written to.
 *
 *  \param qid       int id of the process to be identifed.
 *  \param buff pointer to string buffer to hold the results.
 *
 */
char get_process_name (pid_t qid, char *buffer)
{

  char str_id[6];
  char name[64];
  char state[48];

  int  n;

  FILE *input;
  std::string filename = "/proc/";

  struct stat sb;

  sprintf (&str_id[0], "%d", qid);
  filename.append(&str_id[0]);
  filename.append("/status");

  if ( stat (filename.c_str(), &sb) != -1) {
    if (( input = fopen( filename.c_str(), "r")) != NULL) {
      n = fscanf(input, "%s ", &name[0]);
      n = fscanf(input, "%s ", &state[0]);
      fclose(input);
      try {
        strcpy(buffer, &state[0]);
      }
      catch (std::exception& e) {
        std::cerr << e.what() << "get_process_name: " << &state[0] << std::endl;
      }
    }
    else {
      if ( DebugMode ) {
        ErrorMessage( "Could not open stat file, <%s>", (char*) filename.c_str());
      }
    }
  }
  else {
    state[0] = 'N'; /* N = No = Not Alive */
  }

  return state[0];
}

/*! \brief Get the Progam Run Mode
 *  \par Function Description
 *  This function seeks to determine how the program was initiated.
 *  The primary method used is based on identifing the process that
 *  initiated the program. If sudo or a shell instantated then a
 *  command-line is assume. If init or start-stop--daemon instantated
 *  then a the program is assume to be stating from an initialization
 *  script if the uptime is less than BOOT_TIME, otherwise a it is
 *  assumed that the run-level is being changed.
 * 
 *  \param dilithium pointer to a Dilithium object.
 *
 */
ProgramRunMode get_run_mode(Dilithium *dilithium ) {

  bool  shell;
  bool  initd;

  char  parent_name[48];
  char *parent;
  char *user;
  const char * const *cpp;

  ProgramRunMode   ret_val;

  pid_t ppid;
  struct sysinfo info;

  parent = &parent_name[0];

  ret_val = EXIT_PROGRAM;

  shell = false;
  initd = false;

  /* get the parent process id */
  if ( (ppid = getppid()) < NO_ERROR) {
    ErrorMessage("unable to identify the parent process, Exit");
  }
  else if ( get_process_name (ppid, parent) == 'N') {
    ErrorMessage("unable to determine parent, Exit");
  }
  else {

    for (cpp = shells; *cpp && !shell; cpp++)
      shell = !strcmp(parent, *cpp) ? true : false;

    if (!shell)
      for (cpp = inits; *cpp && !initd; cpp++)
        strcmp(parent, *cpp);

    if ( !shell && !initd && DebugMode ) {
      ShowMessage("did not recognize parent, <%s>", parent);
    }

    errno = NO_ERROR;
    user = getlogin(); /* Dont care about value, looking for error */

    if (errno == ENOTTY) { /* Run level transition */
      if ( sysinfo(&info) == NO_ERROR ) {
        if (info.uptime > BOOT_TIME ) {
          ret_val = RUN_LEVEL;
        }
        else {
          ret_val = BOOT_DIRECT;
        }
      }
      if ( initd || shell ) {
        ret_val = BOOT_DIRECT;
      }
    }
    else if (shell) {
      ret_val = COMMANE_LINE;
    }
    else if (initd) {
      if ( DebugMode ) {
        ShowMessage("Boot by an initializer, but got user <%s>", user);
      }
      if ( sysinfo(&info) == NO_ERROR ) {
        if (info.uptime > BOOT_TIME ) {
          ret_val = RUN_LEVEL;
        }
        else {
          ret_val = BOOT_DIRECT;
        }
      }
    }
  }

  dilithium->run_mode = ret_val;
  return ret_val;
}

/*! \brief Get User Info
 *  \par Function Description
 *  This function obtains the user passwd information in structure
 *  memory the string data is dynamically allocated. The atexit
 *  function is set to release the memory.
 *
 *  \note: The _r variant of getpwnam() (with its weird semantics)
 *  is used so that the passwd_entry doesn't potentially get stomped
 *  on by a PAM module
 *
 *  \param user      pointer to user name string.
 *  \param delimitor pointer to pwd structure to hold the results.
 *
 *  \retval userinfo pointer to data stucture that was pass as an argument
 *                   if successful, otherwise NULL is returned.
 */
passwd *get_user_info (const char* user, passwd *pwd)
{
  size_t ubufsize;          /* s&& !BeQuiteize of user info buffer */
  struct passwd *userinfo;  /* pointer to pwd if getpwnam is successful  */

  if (user_buffer != NULL ) {
    free(user_buffer);
  }

  errno = NO_ERROR;
  userinfo = NULL;
  user_buffer = NULL;

  ubufsize  = sysconf( _SC_GETPW_R_SIZE_MAX );

  if (ubufsize < NO_ERROR) {
    ubufsize = PASSWD_BUFFER_SIZE;
  }

  user_buffer = (char*) malloc(ubufsize);

  if (user_buffer == NULL) {
    ErrorMessage( "malloc for userinfo. Exit." );
  }
  else {

    atexit (exit_funtion);

    errno = NO_ERROR;
#ifdef HAVE_GETPW_R_POSIX
    errno = getpwnam_r (user, pwd, user_buffer, ubufsize, &userinfo);
#else
    userinfo = getpwnam_r (user, pwd, user_buffer, ubufsize);
    errno = NO_ERROR;
#endif /* ! HAVE_GETPW_R_POSIX */
    if (errno == EINTR) { /* Then do try again */
      if ( DebugMode ) {
        ErrorMessage("user info indeterminate");
      }
      userinfo = NULL;
#ifdef HAVE_GETPW_R_POSIX
      errno    = getpwnam_r (user, pwd, user_buffer, ubufsize, &userinfo);
#else
      userinfo = getpwnam_r (user, pwd, user_buffer, ubufsize);
      errno = NO_ERROR;
#endif /* ! HAVE_GETPW_R_POSIX */
      if (errno != NO_ERROR) {
        userinfo = NULL;
      }
    }
    else {
      if (errno != NO_ERROR) {
        userinfo = NULL;
      }
    }
  }
  return userinfo;
}

/*! \brief Set Display Parameter
 *  \par Function Description
 *  This function determines the next available display based on
 *  the existence of the lock files in the /tmp directory.
 */
char* get_server() {

  char *server;

  if (access(DEFAULT_SERVER, F_OK) == NO_ERROR) {
    server = DEFAULT_SERVER;
  }
  else {
    ErrorMessage("Did not find X server, is <%s> a valid link?", DEFAULT_SERVER);
    server = NULL;
  }
  return server;
}

/*! \brief Start GUI using Xinit
 *  \par Function Description
 *  This function is utilize xinit to invoke xinit to start the
 *  the server with the client. To use the routine the --use-xinit
 *  option must be given as an argument to the program.
 * 
 *  \param d pointer to a Dilithium object.
 *
 */
int use_xinit( Dilithium *d ) {

  char *argv[100];
  int   argc;
  int index;

  std::vector<std::string> ca_tokens;
  std::vector<std::string> sa_tokens;

  if ( DebugMode ) {
    printf("xinit %s %s -- %s %s %s\n", d->xclient, d->xclientargs,
           d->xserver, d->display, d->xserverargs);
  }
  else {

    index = 0;
    argv[index++] = (char *) "xinit";
    argv[index++] = (char *) d->xclient;

    argc = word_count(d->xclientargs);

    if ( argc != 0 ) {
      if ( argc > 1 ) {
        Tokenize(d->xclientargs, ca_tokens);
        for ( std::vector<std::string>::iterator i  = ca_tokens.begin();
                                                 i != ca_tokens.end(); i++ )
        {
          argv[index++] = (char*) i->c_str();
        }
      }
      else {
        argv[index++] = (char *) d->xclientargs;
      }
    }

    argv[index++] = (char *) "--";
    argv[index++] = (char *) d->xserver;
    argv[index++] = (char*) d->display;

    argc = word_count(d->xserverargs);

    if ( argc != 0 ) {
      if ( argc > 1 ) {
        Tokenize(d->xserverargs, sa_tokens);
        for ( std::vector<std::string>::iterator i  = sa_tokens.begin();
                                                 i != sa_tokens.end(); i++ )
        {
          argv[index++] = (char*) i->c_str();
        }
      }
      else {
        argv[index++] = (char *) d->xserverargs;
      }
    }

    argv[index++] = (char *) NULL;

    fflush(NULL);

    execvp(argv[0], argv);

    ErrorMessage( "Failed to execute xinit, Exit." );
  }
  return -1;
}

/*! \brief Set Display Parameter
 *  \par Function Description
 *  This function determines the next available display based on
 *  the existence of the lock files in the /tmp directory.
 */
bool set_display( Dilithium *dilithium ) {

  char lockfile[] = "/tmp/.X0-lock";

  while (1) {
    if (access(lockfile, F_OK) == -1)
      break;
    lockfile[7]++;  /* increment the number after the X */
    if ( lockfile[7] == '9') {
      ErrorMessage("Too many locks, <%s>, aborting", &lockfile[0]);
      return false;
    }
  }
  dilithium->display[0] = ':';
  dilithium->display[1] = lockfile[7];

  strcpy (dilithium->lockfile, &lockfile[0]);

  return true;
}

/*! \brief Set Program Options
 *  \par Function Description
 *  This function interrogates each parameter option and fills in
 *  missing options with default values. Before assigning default
 *  for the client, client arguments and the server arguments, the
 *  environment is check for a value and if found the environment
 *  setting is used instead of the default value.
 * 
 */
bool set_options( Dilithium *d ) {

  char *tmp_str;
  bool result;

  result = true;

  if (d->empty( d->display )) {
    if (!set_display(d)) {
      return false;
    }
  }

  if (d->user_name.empty() ) {
    errno = NO_ERROR;
    if ( ( tmp_str = getlogin() ) == NULL) {
      if (errno != ENOTTY) {
        ShowMessage("set_options getlogin error, <%s>",strerror(errno));
      }
    }
    else {
      d->user_name = tmp_str;
    }
  }

  if (d->empty( d->xclient )) {
    if ( getenv("DISPLAY_MANAGER") != NULL ) {
      d->set( d->xserverargs, getenv("DISPLAY_MANAGER"));
    }
    else {
      d->set( d->xclient, DEFAULT_CLIENT);
    }
  }

  if (d->empty( d->xclientargs )) {
    if ( getenv("CLIENT_ARGUMENTS") != NULL ) {
      d->set( d->xserverargs, getenv("CLIENT_ARGUMENTS"));
    }
    else {
      d->set( d->xclientargs, DEFAULT_CLIENT_ARG);
    }
  }

  if (d->empty( d->xserver )) {
    if ( ( tmp_str = get_server() ) == NULL) {
      result = false;
    }
    else {
      d->set(  d->xserver, tmp_str);
    }
  }

  if (d->empty( d->xserverargs )) {
    if ( getenv("SERVER_ARGUMENTS") != NULL ) {
      d->set( d->xserverargs, getenv("SERVER_ARGUMENTS"));
    }
    else {
      d->set( d->xserverargs, DEFAULT_SERVER_ARG);
    }
  }

  if (Verbose) {
    strcat(d->xserverargs, " -verbose -logverbose 7");
  }

  if (d->login_background.empty() ) {
    d->login_background = DEFAULT_LOGIN_BG;
  }

  return result;
}

/*! \brief Check for and Remove Old PID files
 *  \par Function Description
 *  This function utilities the stat function to verify the existence
 *  of pidfiles. If found then an attempt is made to remove the file.
 *  A message is generated for the results of the action.
 *
 *  \param d pointer to a char string containing the filename.
 * 
 *  \retval true if success or false of file could not be deleted.
 */
int remove_pid_file (const char *filename)
{
   struct stat sb;
   int ret_val;

   errno = 0;
   if (stat(filename, &sb) != -1) {
     if ( (ret_val = remove ( filename )) =! 0 ) {
       ErrorMessage("Could not delete stale PID File <%s>.", filename);
     }
     else {
       if ( Verbose ) {
         ShowMessage("Removed stale PID file");
       }
     }
   }

   return ret_val;
}

/*! \brief Check for and Remove Old PID files
 *  \par Function Description
 *  This function is used prior to becomming a daemon to see if there
 *  is already an instence running. If there are no instence in memory
 *  Then the routine checks any stale pid files. If a stale PID file
 *  is detected then the remove_pid_file function is called to attempt
 *  removal of the file.
 *
 * \param d pointer to a char string containing the name of the program.
 * 
 * \retval result boolean = false if a previous instance is running
 *                          otherwise result = true
 */
bool Expell_Old_Daemon (char *name) {

    bool result;
    int isSysInit;
    int isRuning;

    pid_t pid;

    std::stringstream command;

    pid = getpid(); /* get this process pid */

    command << "ps -C " << name << " | grep " << name << " | grep -v " << pid;
    isRuning = system(command.str().c_str());

    if ( isRuning == NO_ERROR) {
        std::cout << "Another process is already running. exiting." << std::endl;
        result = false;
    }
    else {

      result = true;

      std::string idfile;

      idfile = PIDPATH;
      idfile.append ( PIDFILE );

      /* Check for pid file in /var/run */
      if (access(idfile.c_str(), F_OK) == 0) {
        ShowMessage("Expell old daemon, detected: <%s>", idfile.c_str());
        result = remove_pid_file (idfile.c_str());
      }

      /* Check for pid file in ~/ */
      idfile = getenv("HOME");
      idfile.append ( "/" );
      idfile.append ( PIDFILE );;

      if (access(idfile.c_str(), F_OK) == 0) {
        ShowMessage("Expell old daemon, detected: <%s>", idfile.c_str());
        result += remove_pid_file (idfile.c_str());
      }

      if ( Verbose && !result) {
        ErrorMessage( "ignoring previous status, continuing.");
        result = true;
      }
    }

    return result;
}

/*! \brief Parse Command-Line
 *  \par Function Description
 *  This function interrogates each parameter on the command-line
 *  and fills any discovered information into the option structure,
 *  or sets the corresponding global directive variable. This
 *  function also handles the help, usage and version arguments.
 *
 * Return: 0 exit, 1 continue
 * 
 */
int parse_command_line(int argc, char *argv[], Dilithium *d ) {

  int ret_val;
  int i;

  ret_val = 1;

  for ( i = 1 ; i < argc ; i++) {
    if (strncmp(argv[i],":", 1)==0) {
      if ( strlen(argv[i]) == 2) {
        d->display[0] = ':';
        d->display[1] = argv[i][1];
      }
      else {
        ErrorMessage("ignoring bad argument <%s>.", argv[i]);
      }
    }
    else if (strcmp(argv[i],"--display")==0) {
      if ( strlen(argv[i]) == 2) {
        d->display[0] = ':';
        d->display[1] = argv[i+1][0];
      }
      else {
        ErrorMessage("ignoring bad argument <%s>.", argv[i]);
      }
    }
    else if ((strcmp(argv[i],"-U")==0) ||
             (strcmp(argv[i],"--user")==0)) {
              d->user_name = argv[++i];
              i++; /* increment past the user name */
    }
    else if ((strcmp(argv[i],"-c")==0) ||
             (strcmp(argv[i],"--client")==0)) {
              d->set (d->xclient, argv[++i]);
              i++; /* increment past the client */
    }
    else if ((strcmp(argv[i],"-a")==0) ||
             (strcmp(argv[i],"--cargs")==0)) {
               d->set (d->xclientargs, argv[++i]);
               i++; /* increment past the client arguments */
    }
    else if ((strcmp(argv[i],"-s")==0) ||
             (strcmp(argv[i],"--server")==0)) {
               d->set (d->xserver, argv[++i]);
               i++; /* increment past the server argument */
    }
    else if ((strcmp(argv[i],"-x")==0) ||
             (strcmp(argv[i],"--xargs")==0)) {
               d->set (d->xserverargs, argv[++i]);
               i++; /* increment past the server arguments */
    }
    else if (strcmp(argv[i],"--auth")==0) {
               d->set (d->xauthority, argv[++i]);
               i++; /* increment past the authority file name */
    }
    else if ((strcmp(argv[i],"-q")==0) ||
             (strcmp(argv[i],"--quite")==0)) {
           BeQuite = true;
    }
    else if ((strcmp(argv[i],"-u")==0) ||
             (strcmp(argv[i],"--usage")==0)) {
           ShowUsage(argv);
           ret_val = 0;
           break;
    }
    else if ((strcmp(argv[i],"-v")==0) ||
             (strcmp(argv[i],"--verbose")==0)) {
           Verbose = Verbose ? false : true;
    }
    else if ((strcmp(argv[i],"-h")==0) ||
             (strcmp(argv[i],"--help")==0)) {
           ShowHelp();
           ret_val = 0;
           break;
    }
    else if ((strcmp(argv[i],"-V")==0) ||
             (strcmp(argv[i],"--version")==0)) {
           printf("d Version %2.2f\n", Version);
           ret_val = 0;
           break;
    }
    else if (strcmp(argv[i],"--no-log")==0) {
           DilithiumLog = false;
    }
    else if (strcmp(argv[i],"--logfile")==0) {
           d->log_file_name = argv[++i];
              i++; /* increment past the log file name */
    }
    else if (strcmp(argv[i],"--daemon")==0) {
           DaemonMode = true;
    }
    else if (strcmp(argv[i],"--no-drop")==0) {
           DropPrivileges = false;
    }
    else if (strcmp(argv[i],"--no-kill")==0) {
           KillX = false;
    }
    else if (strcmp(argv[i],"--xinit")==0) {
           UseXinit = true;
    }
    else if ((strcmp(argv[i],"-d")==0) ||
             (strcmp(argv[i],"--debug")==0)) {
           DebugMode = true;
    }
    else if ( d->empty( d->unknown )) {
       d->set (d->unknown, argv[i]);
    }
    else {
      d->append(d->unknown, ", ");
      d->append(d->unknown, argv[i]);
    }
  }
  return ret_val;
}

int main(int argc, char *argv[]) //,char** envp
{

  Dilithium dilithium;

  Console  C;
  Daemon  *D;
  Spawner *S;

  char    *me;
  int      exit_code;

  me = basename(argv[0]);

  exit_code = EXIT_SUCCESS;

  if (strcmp( me, DAEMON_NAME) == 0) {
      DaemonMode = true;
  }

  if(argc > 1) {
    if ( parse_command_line(argc, argv, &dilithium) == EXIT_PROGRAM ) {
      /* Means option resolved, like --help */
      return EXIT_SUCCESS;
    }
    else if ( !dilithium.empty( dilithium.unknown ) ) {
      ErrorMessage(" unknown arguments detected: %s", dilithium.unknown);
      ErrorMessage(" multiple server or client arguments must be enclosed with single quotes");
    }
  }

  if ( DilithiumLog ) {
    if( dilithium.log_file_name.empty() ) {
      dilithium.log_file_name = DILITHIUM_LOGFILE;
    }
    logfile.open(dilithium.log_file_name.c_str());
  }

  if ( !Expell_Old_Daemon (me) ) {
    ErrorMessage("expelling daemons, Exit");
    exit_code = EALREADY;
  }
  else if ( !set_options (&dilithium)) {
    exit_code = EALREADY;
  }
  else if (( get_run_mode(&dilithium)) == EXIT_PROGRAM) {
    exit_code = EXIT_FAILURE;
  }
  else if ( (exit_code = dilithium.initialize_user()) != EXIT_SUCCESS ) {
    ErrorMessage("Dilithium Terminating,");
  }
  else if ( !set_authority(&dilithium) ) {
    ErrorMessage("There was a problem setting Xauthority. Is an Xserver already running?");
  }
  else {

    C.save_active_vt();

    //dilithium.user_name.clear();  /* Used during debugging to force a login */

    if (DaemonMode) {
      D = new Daemon(&dilithium);
    }

    if ( UseXinit && DaemonMode ) {
      D->Idle();
    }
    else if ( UseXinit ) {
        use_xinit(&dilithium);
    }
    else {
      S = new Spawner(&dilithium);
      exit_code = S->do_spawn();
    }

    C.restore_saved_vt();
  }     /* End if setuid */

  if ( !dilithium.empty( dilithium.lockfile )) {
    if (access( dilithium.lockfile, F_OK) == NO_ERROR) {
      if ( remove(dilithium.lockfile) != NO_ERROR ) {
        ErrorMessage("Could not lockfile <%s>.", dilithium.lockfile);
      }
    }
  }

  if ( DilithiumLog ) {
    logfile.close();
  }
  return exit_code;
}

/******************************************************************************/

/*! \brief Dilithium Initialize Class Object
 *  \par Function Description
 * 
 *  This function isjust setup pointer to char arrays and clears the
 * memory assocated with the arrays.
 *
 */
void Dilithium::initialize()
{
    lockfile      = &string_lockfile[0];
    unknown       = &string_unknown[0];
    xauthority    = &string_xauthority[0];
    xclient       = &string_xclient[0];
    xclientargs   = &string_xclientargs[0];
    xserver       = &string_xserver[0];
    xserverargs   = &string_xserverargs[0];
    display       = &string_display[0];

    memset (lockfile,    0, sizeof(string_lockfile));
    memset (unknown,     0, sizeof(string_unknown));
    memset (xclient,     0, sizeof(string_xclient));
    memset (xclientargs, 0, sizeof(string_xclientargs));
    memset (xserver,     0, sizeof(string_xserver));
    memset (xserverargs, 0, sizeof(string_xserverargs));
    memset (display,     0, sizeof(string_display));

}

void debug_environment() {

  char** env;

  for (env = environ; *env != 0; env++)
  {
    char* thisEnv = *env;
    logfile <<  thisEnv << std::endl;
  }

}
/*! \brief Dilithium Initialize USer Info Class Object
 *  \par Function Description
 *
 *  This function attempts to obtain the user information
 *  based on the Run Mode.
 *
 */
int Dilithium::initialize_user()
{
  char *user;
  int ret_val = EXIT_SUCCESS;

  logfile << "Begin Dilithium::initialize_user: run_mode=" << run_mode << std::endl;
  if ( run_mode > RUN_LEVEL ) {
    user = getenv("SUDO_USER");
    if ( user ) {

      if ( user_name.empty() ) {
        logfile << "Dilithium::initialize_user: user_name is empty, set to SUDO_USER=" << user << std::endl;
        user_name = user;
      }
      else
        logfile << "Dilithium::initialize_user: user_name=" << user << std::endl;

      logfile << std::endl;

    }
    else {
      if (user_name.empty()) {
        logfile << "Dilithium::initialize_user: NO USER NAME" << std::endl;
      }
      else {
        logfile << "Dilithium::initialize_user: NO SUDO_USER, user_name=" <<  user_name << std::endl;
      }
    }
  }
  //debug_environment();

  if (!user_name.empty()) {
      //clear_environment();
    if ( (userinfo = ::get_user_info(user_name.c_str(), &pwd)) == NULL) {
      ShowMessage( "Bad user environment" );
      ret_val = EXIT_FAILURE;
    }
    else if ( initgroups(userinfo->pw_name, userinfo->pw_gid) == -1) {
      ShowMessage("Cannot initgroups, do you have permission?");
      ret_val = EXIT_FAILURE;
    }

    set_environment( userinfo);

    if ((chdir(userinfo->pw_dir) != NO_ERROR) && Verbose ) {
      ErrorMessage("could not change directory to <%s>, continuing",
                   userinfo->pw_dir);
    }
  }

  Privileges::initialize (userinfo);

  return ret_val;
}

