/* xjpeg.h
   Header file for xjpeg.cc.
*/
/**
 * @par A jpeg decoding utilitity for X Clients
 *
 * Copyright (C) 2008 - 2013 Jonathan Andrews
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

/* This header only exist so libxlogin.cc can see the declaration of
 * jpeg_decode, and is not actually included in xjpeg.cc */

XImage* jpeg_decode (const char filename[], Display *display, int xdepth,
                     int scalenum, int scaledenom, int *didxcreate, int *sucess);