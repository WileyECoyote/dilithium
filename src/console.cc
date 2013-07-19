/* console.cc
   Component Source file for the Dilithium Program.
*/
/**
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
 *
 */
/*!   @file    console.cc C++ Source file for the Console class
 *    @brief   Console class is used to save and restore the active console
 *    @author  Wiley Edward Hill
 *    @date    2012.07.09
 */ 
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include <sys/ioctl.h>

#include <fcntl.h>
#include <errno.h>

//#include <linux/vt.h> /* for struct vt_stat & VT_GETSTATE = 0x5603 */
#include <linux/kd.h> /* for KDGKBTYPE = 0x4B33 */

#include "console.h"

static int console_num;

Console::Console()
{ console_num = -1; }

/*! \brief Console::xioctl
 *  \par Function Description
 *
 */
int Console::xioctl(int filedes, int command, void *opt)
{
   int r;

   do {
      r = ioctl(filedes, command, opt);
   } while (-1 == r && EINTR == errno);

   return r;
}

/*! \brief Console::open_a_console
 *  \par Function Description
 *
 *  This function is opens a console given it's name.
 *
 */
int Console::open_a_console(const char *fnam)
{
   int filedes;

   /* try read-write */
   filedes = open(fnam, O_RDWR);

   /* if failed, try read-only */
   if (filedes < 0 && errno == EACCES)
      filedes = open(fnam, O_RDONLY);

   /* if failed, try write-only */
   if (filedes < 0 && errno == EACCES)
      filedes = open(fnam, O_WRONLY);

   return filedes;
}

/*! \brief Console::get_console_fd
 *  \par Function Description
 *
 */
int Console::get_console_fd(void)
{
  static const char *const console_names[] = {
    DEV_CONSOLE, CURRENT_VC, CURRENT_TTY
  };

  int filedes;
  int ret_des;
  ret_des = EXIT_FAILURE;
  try {
    for (filedes = 2; filedes >= 0; filedes--) {
      int fd4name;
      int choice_fd;
      char arg;

      fd4name = open_a_console(console_names[filedes]);
      chk_std:
      choice_fd = (fd4name >= 0 ? fd4name : filedes);

      arg = 0;
      if (ioctl(choice_fd, KDGKBTYPE, &arg) == 0) {
         ret_des = choice_fd;
         break;
      }
      if (fd4name >= 0) {
         close(fd4name);
         fd4name = -1;
         goto chk_std;
      }
    }
  }
  catch (std::exception& e) {
      std::cerr << e.what() << ", ";
  }
  if (ret_des == EXIT_FAILURE) {
    perror("can't open console");
  }
  return ret_des;
}

/*! \brief Console::set_active_vt Sets the Active VT Console
 *  \par Function Description
 *  This function set the active console to value provided as
 * an argument.
 *
 */
/*  */
int Console::set_active_vt( int vt_num)
{
   if ( vt_num > 0 && vt_num < 64 ) {
     int filedes = get_console_fd();
     xioctl(filedes, VT_ACTIVATE, (void *)(std::ptrdiff_t)vt_num);
     xioctl(filedes, VT_WAITACTIVE, (void *)(std::ptrdiff_t)vt_num);
   }
   else
     return EXIT_FAILURE;
   return EXIT_SUCCESS;
}

/*! \brief Console::open_a_console Retrieves the Active Console
 *  \par Function Description
 *  This function is return the integer number of the active console.
 *
 */
int Console::get_active_vt(void)
{
  vtstat.v_active = 0;
  xioctl(get_console_fd(), VT_GETSTATE, &vtstat);
  return vtstat.v_active;
}

/*! \brief Console::save_active_vt
 *  \par Function Description
 *  This function uses the previous function to save the active
 * console to the global static variable console_num.
 *
 */
bool Console::save_active_vt(void)
{
  if (( console_num = get_active_vt()) != EXIT_FAILURE); {
    return true;
  }
  return false;
}

/*! \brief Console::restore_saved_vt
 *  \par Function Description
 *
 *  This function set the active console to be the previously
 *  save console.
 *
 */
bool Console::restore_saved_vt(void)
{
  if ( console_num > 0 ) {
    if ( set_active_vt( console_num ) == EXIT_SUCCESS); {
      return true;
    }
  }
  return false;
}










