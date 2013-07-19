/* label.hpp
   definition of the xlib::label_base class
*/
/**
 * @par xlib++ - X Low level Widget Routines
 *
 * Copyright (C) 2013 Wiley Edward Hill
 *
 * This file was added to Rob Tougher's <robt@robtougher.com> collection
 * of c++ classes for creating widgets using low level X routines, i.e.
 * no dependencies on the G's lib.
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

// definition of the xlib::label_base class

#ifndef _xlib_label_base_class_
#define _xlib_label_base_class_

#include <X11/Xlib.h>
#include "color.hpp"
#include "shapes.hpp"

namespace xlib
{
  class display;

  class label_base
    {
    public:

      virtual long id() = 0;

      virtual void show() = 0;
      virtual void hide() = 0;
      virtual void create() = 0;
      virtual void destroy() = 0;
      virtual void refresh() = 0;

      virtual void set_background ( color& c ) = 0;

      virtual void set_focus() = 0;

      virtual rectangle get_rect() = 0;
      virtual event_dispatcher& get_event_dispatcher() = 0;
      virtual display& get_display() = 0;

      // callbacks
      virtual void on_expose() = 0;

      virtual void on_show() = 0;
      virtual void on_hide() = 0;

      virtual void on_create() = 0;
      virtual void on_destroy() = 0;

    };
};

#endif

