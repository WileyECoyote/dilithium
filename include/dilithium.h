/* dilithium.h
   Header file for dilithium.cc.
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

#ifndef DILITHIUM_H             /* Prevent double inclusion */
#define DILITHIUM_H

#pragma GCC diagnostic ignored "-Wwrite-strings"

#define THIS_PROGRAM "Dilithium"

#ifndef NO_ERROR
#define NO_ERROR 0
#endif

#define DILITHIUM_LOGFILE  "/var/log/dilithium.log"

#define BOOT_TIME 120     /* seconds */
#define XAUTHORITY_FILE    "Xauthority"

#define MAX_USER  32
#define MAX_PATH  256
#define MAX_ARG   1024

#define DEFAULT_CLIENT     "/usr/bin/fvwm-crystal"
#define DEFAULT_CLIENT_ARG ""
#define DEFAULT_SERVER     "/usr/bin/X"
#define DEFAULT_SERVER_ARG "-br -novtswitch -nolisten tcp"
#define SERVER_BOOT_DELAY  5 /* seconds */

#ifndef PASSWD_BUFFER_SIZE
#define PASSWD_BUFFER_SIZE 2048
#endif

#define DEFAULT_LOGIN_BG "/usr/share/fvwm-crystal/fvwm/wallpapers/dark-login-366x200.jpg"

enum ProgramRunMode {
    EXIT_PROGRAM,
    BOOT_DIRECT,
    RUN_LEVEL,
    COMMANE_LINE,
    XLOGIN,

};

class Dilithium : public Privileges
{

protected:

  char string_unknown[64];
  char string_lockfile[15];
  char string_xauthority[MAX_PATH];
  char string_xclient[MAX_PATH];
  char string_xclientargs[MAX_ARG];
  char string_xserver[MAX_PATH];
  char string_xserverargs[MAX_ARG];
  char string_display[3];

private:

  struct passwd pwd;        /* User Data structure filled-in by getpwnam */

  void initialize();
  passwd *get_user_info (char* user, passwd *pwd);

public:

  std::string log_file_name;
  std::string user_name;

  ProgramRunMode run_mode;

  char *lockfile;
  char *unknown;
  char *xauthority;
  char *xclient;
  char *xclientargs;
  char *xserver;
  char *xserverargs;
  char *display;

  std::string login_background;

  struct passwd *userinfo;  /* pointer to pwd if getpwnam is successful  */

  Dilithium()
  {
    initialize();
  }

  int initialize_user();

  char* set( char* target, char* source) {
    if ( source != NULL ) {
      strcpy( target, source);
      return target;
    }
    else {
      return NULL;
    }
  }

  bool empty  (char *str) { return (*str == '\0') ? true : false; }
  bool append (char *str1, char *str2) {
        return ((str1 == NULL) ||  (str2 == NULL)) ? NULL : strcat(str1, str2); }

};

void Write2Log(const char* str);
bool set_display( Dilithium *dilithium );

#endif
