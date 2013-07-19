# getpw.m4                                     -*-Autoconf-*-
# serial 1.0

dnl Check GNU getpwnam_r
dnl Copyright (C) 2013  Wiley Edward Hill <wileyhill@gmail.com>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
dnl
dnl Check if getpwnam_r and getpwuid_r are POSIX.1 compatible
dnl POSIX draft version returns 'struct passwd *' (used on Solaris)
dnl NOTE: getpwent_r is not POSIX so we always use getpwent
dnl

AC_DEFUN([AX_GETPW_R_POSIX],
[
  AC_PREREQ([2.60])dnl

  AC_MSG_CHECKING([whether getpwnam_r and getpwuid_r are posix compliant])

  if test "$ac_cv_func_posix_getpwnam_r" = yes; then
	AC_DEFINE(HAVE_GETPW_R_POSIX,1,[Have POSIX function getpwnam_r])
  else
  # The prototype for the POSIX version is:
  # int getpwnam_r(char *, struct passwd *, char *, size_t, struct passwd **)
  # int getpwuid_r(uid_t, struct passwd *, char *, size_t, struct passwd **);
  AC_TRY_LINK([#include <stdlib.h>
	       #include <sys/types.h>
	       #include <pwd.h>],
               [getpwnam_r(NULL, NULL, NULL, (size_t)0, NULL);
                getpwuid_r((uid_t)0, NULL, NULL, (size_t)0, NULL);],
      [AC_DEFINE([HAVE_GETPW_R_POSIX], 1, [Define to 1 if you have getpwnam_r and getpwuid_r that are POSIX.1 compatible.])
       AC_MSG_RESULT(yes)],
      [AC_MSG_RESULT(no)])

  fi
  []dnl
])dnl AX_GETPWNAME_POSIX

AC_DEFUN([AX_GETPW_R_DRAFT],
[
   AC_MSG_CHECKING([whether getpwnam_r and getpwuid_r are posix _draft_ like])
      # The prototype for the POSIX draft version is:
      # struct passwd *getpwuid_r(uid_t, struct passwd *, char *, int);
      # struct passwd *getpwnam_r(char *, struct passwd *,  char *, int);
   AC_TRY_LINK([#include <stdlib.h>
                #include <sys/types.h>
                #include <pwd.h>],
               [getpwnam_r(NULL, NULL, NULL, (size_t)0);
                getpwuid_r((uid_t)0, NULL, NULL, (size_t)0);],
      [AC_DEFINE([HAVE_GETPW_R_DRAFT], 1, [Define to 1 if you have getpwnam_r and getpwuid_r that are draft POSIX.1 versions.])
       AC_MSG_RESULT(yes)],
      [AC_MSG_RESULT(no)])
  []dnl
])dnl AX_GETPW_R_DRAFT
