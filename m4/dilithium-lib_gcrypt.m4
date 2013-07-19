# dilithium-lib_gcrypt.m4                               -*-Autoconf-*-
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
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
dnl 02110-1301 USA
dnl
dnl Check for libgcrypt

AC_DEFUN([AX_LIB_CRYPTO],[

  AC_CHECK_HEADER([gcrypt.h],
        [
           AC_DEFINE(HAVE_GCRYPT, 1, [Gcrypt support])
           AC_SUBST(HAVE_GCRYPT)
           have_gcrypt="yes"

        ], [
               have_gcrypt="no"
  ])

  if test "x$have_gcrypt" != "xyes" ; then
    GCRYPT_CFLAGS=""
    GCRYPT_LIBS=""
    AC_MSG_WARN([[
***
*** The libgcrypt is required to make the xauthxx and login objects!
***]])
  fi

  AC_SUBST(GCRYPT_CFLAGS)
  AC_SUBST(GCRYPT_LIBS)

##  AC_MSG_RESULT($have_gcrypt)

])

