/* privileges.h
   Header file for privileges.cc.
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

#ifndef NGROUPS_MAX
 #define NGROUPS_MAX 32
#endif

class Privileges
{

public:

  gid_t gid;
  uid_t uid;

  struct passwd *userinfo;

  //Privileges(passwd *userdata);
  Privileges();

  void initialize (passwd *userdata);
  bool drop_privileges();
  void restore_privileges(void);

protected:

  int   orig_ngroups;
  gid_t orig_groups[NGROUPS_MAX];

  gid_t orig_gid;
  uid_t orig_uid;

};
