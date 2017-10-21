/* xlogin.h
   Header file for xlogin.cc.
*/
/**
 * @par Xlogin - GPL Login Module for FVWM-CRYSTAL
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

#ifndef SPAWNER_XLOGIN__H             /* Prevent double inclusion */
#define SPAWNER_XLOGIN__H

#ifndef MAXUSERNAMESIZE
#define MAXUSERNAMESIZE 27
#endif
#ifndef MAXPASSWORDSIZE
#define MAXPASSWORDSIZE 40
#endif

#define DEFWIDTH 366
#define DEFHEIGHT 200

enum WhatDoNext {
    Login,
    Quit,
    Reboot,
    Shutdown
};

class Xlogin
{
  Dilithium *dilithium;

  Display   *m_display;            /* server connection */
  Window     window;
  GC         image_gc;

  XFontStruct* font_info1;       /* Font structure */
  std::string  font_name;

  std::string  datetimestring;
  std::string  backgroundimage;

// Colors
  long int colorcursor;

// Globals
  int screen;
  int xdepth;
  int width, height;
  int xwidth, xheight;
  int THISWINX;          /* Top left position of the our window */
  int THISWINY;

  XSetWindowAttributes window_attributes;
  unsigned long window_mask;

  int result;

  /* Member Functions */
  void setup_display();
  void get_dimensions();
  int  load_background();

public:

  Xlogin(Dilithium *d);
  void close();
  int  login();

};
#endif