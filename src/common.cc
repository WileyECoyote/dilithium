/* common.cc
   Common source file for the Dilithium Program.
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

#include <stdarg.h>

#include "common.h"
#include "privileges.h"
#include "dilithium.h"
#include "daemon.h"
#include "ascii.h"

char  output_buffer[256];
char  message_buffer[128];
char *ptr_output_buffer  = &output_buffer[0];
char *ptr_message_buffer = &message_buffer[0];

bool  BeQuite        = false;
bool  DebugMode      = false;
bool  DaemonMode     = false;
bool  DropPrivileges = true;
bool  KillX          = true;
bool  UseXinit       = false;
bool  Verbose        = false;
bool  DilithiumLog   = true;


/*! \brief Error Message function
 *  \par Function Description
 *  This function Display error messages including 'errno' diagnostic,
 *  but does NOT terminates the process. If daemon mode is active then
 *  messages are directed to the system log.
 */
void ErrorMessage(const char *format, ...)
{
  int ecode = errno;
  va_list args;
  va_start (args, format);
  vsnprintf ( ptr_message_buffer, sizeof(message_buffer), format, args);
  va_end (args);

  strcpy(ptr_output_buffer, "Dilithium Error: ");
  strcat(ptr_output_buffer, ptr_message_buffer);

  if (ecode != 0) {
    strcat ( ptr_output_buffer, " ");
    strcat ( ptr_output_buffer, strerror(ecode));
  }

  if (DaemonMode) {
    openlog(DAEMON_NAME, LOG_DAEMON | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
    syslog(LOG_NOTICE, "%s", ptr_output_buffer );
  }
  else {
    std::cerr << ptr_output_buffer << std::endl;
  }

  if ( DilithiumLog ) {
    Write2Log(ptr_output_buffer);
  }

  closelog();
}

/*! \brief General Message function
 *  \par Function Description
 *  This function Display messages appropriate to the --quite and
 *  --verbose options. If daemon mode is active then messages are
 *  only directed to the system log.
 */
void ShowMessage(const char *format, ...) {

  std::string message;

  va_list args;
  va_start (args, format);
  vsnprintf ( ptr_message_buffer, sizeof(message_buffer), format, args);
  va_end (args);

  message = "Dilithium: ";
  message.append(ptr_message_buffer);

  if (DaemonMode) {
    openlog("Dilithium", LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
      syslog(LOG_NOTICE, "%s", message.c_str());
    closelog();
  }
  else if (!BeQuite) {
    puts( message.c_str() );
  }
  else if (Verbose) {
    puts( message.c_str() );
  }

  if ( DilithiumLog ) {
    Write2Log(message.c_str());
  }

}

/*! \brief Return Word Count
 *  \par Function Description
 *  This function returns the number of white-spaces or TAB characters
 *  + 1. In effect, the number of words in a char string.
 *
 * \param *string pointer to char string to be interrogated
 *
 * \retval count integer value
 */
int word_count(const char* str) {
    int   count = 0;
    const char* orig  = str;

    while ( *str != ASCII_NUL) {
      if (*str == ASCII_SPACE ) ++count;
      if (*str == ASCII_TAB )   ++count;
      ++str;
    }
    count = ( count > 0 ) ? count + 1 : ( orig == str ) ? 0 : 1;
    return  count;

}

/*! \brief Create Vector Array of Words in String
 *  \par Function Description
 *  This function locate delimited strings and adds each group to a
 *  a string vector array. The default delimiter is a white space.
 *
 * \param string    to be tokenized
 * \param vector    vector<string> to receive tokens
 * \param delimitor optional, default to a single white-space
 */
void Tokenize(const std::string& str, std::vector<std::string>& tokens,
              const std::string& delimiters)
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

/*! \brief Set Effective User function
 *  \par Function Description
 *  This function sets the user and group id's.
 */
void set_effective_user (passwd *userinfo, char *user) {

  pid_t   gid;
  pid_t   uid;

  gid = getgid();
  uid = getuid();

  if (strcmp(user,"root") != 0) {

#if !defined(linux)
    setegid(userinfo->pw_gid);
    if (setgid(userinfo->pw_gid) == -1) {
      ErrorMessage("could not set group id for user <%s>", user);
    }
#else
    if (setregid(userinfo->pw_gid, gid) == -1) {
      ErrorMessage("could not set group id for user <%s>", user);
    }
#endif

#if !defined(linux)
    seteuid(userinfo->pw_uid);
    if (setuid(userinfo->pw_uid) == -1) {
      ErrorMessage("could not set user id for user <%s>", user);
    }
#else
    if (setreuid(userinfo->pw_uid, uid) == -1) {
      ErrorMessage("could not set user id for user <%s>", user);
    }
#endif
  }
}

void setWindowPath( Display *xd )
{
    /* setting WINDOWPATH for clients */
  Atom prop;
  Atom actualtype;

  int actualformat;

  unsigned long nitems;
  unsigned long bytes_after;
  unsigned char *buf;

  const char *windowpath;
  char *newwindowpath;

  unsigned long num;
  char nums[10];
  int numn;

  size_t len;

  prop = XInternAtom(xd, "XFree86_VT", False);

  if (prop == None) {
    ShowMessage("Unable to intern XFree86_VT atom");
    return;
  }

  if (XGetWindowProperty(xd, DefaultRootWindow(xd), prop, 0, 1,
    False, AnyPropertyType, &actualtype, &actualformat, &nitems,
    &bytes_after, &buf)) {
    ShowMessage("No XFree86_VT property detected on X server, WINDOWPATH won't be set");
    return;
  }

  if (nitems != 1) {
    ShowMessage("XFree86_VT property unexpectedly has %lu items instead of 1", nitems);
    XFree(buf);
    return;
  }

  switch (actualtype) {
    case XA_CARDINAL:
    case XA_INTEGER:
    case XA_WINDOW:
        switch (actualformat) {
        case  8:
            num = (*(uint8_t  *)(void *)buf);
            break;
        case 16:
            num = (*(uint16_t *)(void *)buf);
            break;
        case 32:
            num = (*(uint32_t *)(void *)buf);
            break;
        default:
            ShowMessage("XFree86_VT property has unexpected format %d", actualformat);
            XFree(buf);
            return;
        }
        break;
    default:
        ShowMessage("XFree86_VT property has unexpected type %lx", actualtype);
        XFree(buf);
        return;
  }
  XFree(buf);
  windowpath = getenv("WINDOWPATH");
  numn = snprintf(nums, sizeof(nums), "%lu", num);
  if (!windowpath) {
    len = numn + 1;
    newwindowpath = (char*) malloc(len);
    if (newwindowpath == NULL) {
           return;
    }
    snprintf(newwindowpath, len, "%s", nums);
  }
  else {
    len = strlen(windowpath) + 1 + numn + 1;
    newwindowpath = (char*) malloc(len);
    if (newwindowpath == NULL) {
           return;
    }
    snprintf(newwindowpath, len, "%s:%s", windowpath, nums);
  }

  if (setenv("WINDOWPATH", newwindowpath, true) == -1) {
    ErrorMessage("unable to set WINDOWPATH");
  }

  free(newwindowpath);
}
