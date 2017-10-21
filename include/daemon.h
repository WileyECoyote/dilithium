/* daemon.h
   Header file for daemon.cc.
*/
/*!
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

#ifndef DAEMON_DAEMON_H             /* Prevent double inclusion */
#define DAEMON_DAEMON_H

#define DAEMON_NAME "dilithium.d"
#define PIDPATH     "/var/run/"
#define PIDFILE     "dilithium.pid"
#define x_seperator "--"

/* Bit-mask values for 'flags' argument of becomeDaemon() */

#define BD_NO_CHDIR           01    /* Don't chdir("/") */
#define BD_NO_CLOSE_FILES     02    /* Don't close all open files */
#define BD_NO_REOPEN_STD_FDS  04    /* Don't reopen stdin, stdout, and
                                       stderr to /dev/null */
#define BD_NO_UMASK0         010    /* Don't do a umask(0) */

#define BD_MAX_CLOSE        8192    /* Maximum file descriptors to close if
                                       sysconf(_SC_OPEN_MAX) is indeterminate */
#define IDLE_SLEEP_SECONDS 60

class Daemon
{

protected:

  bool  privileges;
  Dilithium *dilithium;

private:

  gid_t gid;
  uid_t uid;

  pid_t pid;
  pid_t sid;

  bool created_pidfile;
  std::string pidfile;

  void LogMsg (int, char*, char*, char*);
  void LogMsg (int, char*, int);

  bool CreateIdFile(bool flag);
  void initialize();

  void remove_pidfile();

  int resolve_descriptors ( int flags );
  int becomeDaemon( int flags );

  pid_t spawn_gui(void);

public:

  Daemon (Dilithium *dilithium);

  void Idle();
  char get_process_status (pid_t qid);
  static void signal_handler(int sig);

};

#endif
