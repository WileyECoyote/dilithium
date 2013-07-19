//
//
// Copyright 2002 Rob Tougher <robt@robtougher.com>
//
// This file is part of xlib++.
//
// xlib++ is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// xlib++ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with xlib++; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//


// definition of the display class

#ifndef _xlib_display_class_
#define _xlib_display_class_

#include <string>
#include <sstream>
#include <iostream>
#include <X11/Xlib.h>
#include "exceptions.hpp"

namespace xlib
{

   class display
   {
   public:
      display ( std::string name )
      {
         m_display = XOpenDisplay ( name.c_str() );

         if ( ! m_display )
         {
            std::ostringstream ost;
            ost << "Could not open display '" << name << "'. Is X server running?";
            throw open_display_exception ( ost.str() );
         }
      }

      ~display()
      {
         if ( m_display )
         {
            XCloseDisplay ( m_display );
            m_display = 0;
         }
      }

      void close()
      {
         delete(this);
      }
      Display* get()
      {
         return m_display;
      }
      operator Display*() { return m_display; }

   private:

      Display* m_display;
   };
};

#endif

