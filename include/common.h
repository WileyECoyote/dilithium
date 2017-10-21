/* common.h
   Common header file for the Dilithium Program.
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
/*!   @file    common.h Common Header file
 *    @brief   Provides inclusions common to multible source files
 *    @author  Wiley Edward Hill
 *    @date    2012.07.04
 */
#ifndef COMMON_COMMON_H         /* Prevent double inclusion */
#define COMMON_COMMON_H

#include "config.h"

#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>         /* gid_t uid_t */
#include <sys/wait.h>          /* needed for struct sigaction in spawner.h */

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <inttypes.h>

#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <sstream>             /* stringstream */
#include <vector>

#include <string.h>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>

#include <pwd.h>               /* needed for pwd & getpwnam_r */

#if defined(linux)
  #include <grp.h>             /* needed for set groups */
#endif

void ErrorMessage(const char *format, ... );
void ShowMessage(const char *format, ... );

int word_count(const char* str);

void Tokenize(const std::string& str, std::vector<std::string>& tokens,
              const std::string& delimiters = " ");
void set_effective_user (passwd *userinfo, char *user);
void setWindowPath( Display *xd );

#endif