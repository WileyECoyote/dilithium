/* privileges.cc
   Component Source file for the Dilithium Program.
*/
/* dilithium - GPL Laucher for FVWM-CRYSTAL
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
 *
 * Note: Dilithium is derived from class Privileges
 */

#include <sys/types.h>        /* gid_t uid_t */

#include <unistd.h>
#include <syslog.h>

#include <pwd.h>           /* needed for getpwnam_r */
#if defined(linux)
  #include <grp.h>         /* needed for set groups */
#endif

#include "global.h"
#include "privileges.h"

/*! \brief Privileges Drop Privileges
 *  \par Function Description
 *  Normally a daemon function drop privileges once the privileges
 *  are no longer needed. We do do this, but only after saving a copy
 *  so that we can restore them later.
 */
bool Privileges::drop_privileges()
{
  char *user;

  gid_t oldgid;
  uid_t olduid;

  gid = getgid();
  uid = getuid();

  if (!DropPrivileges ) {
    if ( DebugMode ) {
      syslog (LOG_NOTICE, "(DEBUG+) Dilithium NOT dropping privileges,");
    }
    return false;
  }

  oldgid = getegid();
  olduid = geteuid();
  user = userinfo->pw_name;

  if ( DebugMode ) {
     syslog (LOG_NOTICE, "(DEBUG+) Dilithium dropping privileges, user=%s", user);
     syslog (LOG_NOTICE, "(DEBUG+) Begin dropping privileges: real gid=%d, uid=%d", gid, uid);
     syslog (LOG_NOTICE, "(DEBUG+) Begin dropping privileges: effective gid=%d, uid=%d", orig_gid, orig_uid);
  }

  if (!olduid && user && (userinfo = getpwnam(user)) && userinfo->pw_uid)
  {
    setgroups(1, &userinfo->pw_gid);

#if !defined(linux)
    setegid(userinfo->pw_gid);
    if (setgid(userinfo->pw_gid) == -1) { return(false); }
#else
    if (setregid(userinfo->pw_gid, userinfo->pw_gid) == -1) { return(false); }
#endif

#if !defined(linux)
    seteuid(userinfo->pw_uid);
    if (setuid(userinfo->pw_uid) == -1) { return(false); }
#else
    if (setreuid(userinfo->pw_uid, userinfo->pw_uid) == -1) { return(false); }
#endif

    if ((setegid(oldgid) != -1) || (getegid() != userinfo->pw_gid)) {
      return(false);
    }
    if ((seteuid(olduid) != -1) || (geteuid() != userinfo->pw_uid)) {
      return(false);
    }
  }
  else
  {
    /* Drop ancillary groups */

    if (!olduid) setgroups(1, &gid);

    if (gid != oldgid)
    {
#if !defined(linux)
      setegid(gid);
      if (setgid(gid) == -1) {
        return (false);
      }
#else
      if (setregid(gid, gid) == -1) {
        return(false);
      }
#endif
    }

    if (uid != olduid)
    {
#if !defined(linux)
      seteuid(uid);
      if (setuid(uid) == -1) {
        return(false);
      }
#else
      if (setreuid(uid, uid) == -1) { return(false); }
#endif
    }

    /* verify that the changes were successful */
    if (gid != oldgid && (setegid(oldgid) != -1 || getegid() != gid)) {
      syslog (LOG_NOTICE, "Dilithium: Error dropping gid, continuing.");
    }
    if (uid != olduid && (seteuid(olduid) != -1 || geteuid() != uid)) {
      syslog (LOG_NOTICE, "Dilithium: Error dropping uid, continuing.");
    }
  }

  gid = gid;
  uid = uid;

  return(true);
}
/*! \brief Privileges Restore Privileges
 *  \par Function Description
 *  This function undoes what the previous function did.
 */
void Privileges::restore_privileges(void) {

   if (!DropPrivileges && DebugMode) {
      syslog (LOG_NOTICE, "(DEBUG+) Dilithium NOT restoring privileges");
      return;
   }

   if ( DebugMode ) {
      syslog (LOG_NOTICE, "(DEBUG+) Dilithium restoring privileges");
   }

   if (geteuid() != orig_uid)
      if (seteuid(orig_uid) == -1 || geteuid() != orig_uid)
         syslog (LOG_NOTICE, "Dilithium: Error setting uid, continuing.");

   if (getegid() != orig_gid)
      if (setegid(orig_gid) == -1 || getegid() != orig_gid)
         syslog (LOG_NOTICE, "Dilithium: Error setting gid, continuing.");

   if (!orig_uid)
      setgroups(orig_ngroups, orig_groups);
}

/*! \brief Privileges Class Constructor
 *  \par Function Description
 *  This is the constructor for the Privileges class. The construction
 *  save the orginal privileges.
 *
 */
void Privileges::initialize (passwd *userdata) {
   userinfo     = userdata;
}

/*! \brief Privileges Class Constructor
 *  \par Function Description
 *  This is the constructor for the Privileges class. The construction
 *  save the orginal privileges.
 *
 */
Privileges::Privileges () {
   orig_gid     = getegid();
   orig_uid     = geteuid();
   orig_ngroups = getgroups(NGROUPS_MAX, orig_groups);
   userinfo     = NULL;
}