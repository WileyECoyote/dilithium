/* console.h
   Header file for console.cc.
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
 */

/* The following devices are the same on all systems.  */
#define CURRENT_TTY "/dev/tty"
#define DEV_CONSOLE "/dev/console"
#define CURRENT_VC CURRENT_TTY

/* From <linux/vt.h> */
/* We define simple structure and 3 DW here instead of including
 * the header, <linux/vt.h> because this header is included by
 * the main source, dilithium.cc, which would also need to include
 * the kernel headers */
struct vt_stat {
 unsigned short v_active;        /* active vt */
 unsigned short v_signal;        /* signal to send */
 unsigned short v_state;         /* vt bitmask */
};

#define VT_GETSTATE   0x5603     /* get global vt state info */
#define VT_ACTIVATE   0x5606     /* make vt active */
#define VT_WAITACTIVE 0x5607

class Console {

public:

   int  get_active_vt(void);
   int  set_active_vt(int);
   bool save_active_vt(void);
   bool restore_saved_vt(void);

   int  open_a_console(const char *fnam);
   int  get_console_fd(void);

   Console();
   ~Console(){}

private:

  struct vt_stat vtstat;

  int xioctl(int filedes, int command, void *opt);

};

