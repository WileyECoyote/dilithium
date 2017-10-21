/* spawner.h
   Header file for spawner.cc.
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

#pragma GCC diagnostic ignored "-Wwrite-strings"

#ifndef SPAWNER_H             /* Prevent double inclusion */
#define SPAWNER__H

class Spawner //: public Xlogin
{

protected:

  Dilithium *dilithium;

  gid_t oldgid;
  uid_t olduid;

private:

  pid_t sid;
  pid_t cid;

  struct sigaction sa;
  struct sigaction si;

  void initialize();

  bool process_timeout(int timeout, char *string);
  bool waitforserver( const char *displayNum);
  void set_up_signals();

  int start_server();
  int start_client();

  Display *xd;            /* server connection */

public:

  Spawner (Dilithium *d);
  ~Spawner();

  int do_spawn(bool wait_for_kill = true);
  int shutdown();

};

#endif