/* label.hpp
   definition of the xlib::label class
*/
/**
 * @par xlib++ - X Low level Widget Routines
 *
 * Copyright (C) 2013 Wiley Edward Hill
 *
 * This file was added to Rob Tougher's <robt@robtougher.com> collection
 * of c++ classes for creating widgets using low level X routines, i.e.
 * no dependencies on the G or K lib's.
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

#ifndef _xlib_label_class_
#define _xlib_label_class_

#include <string>
#include "display.hpp"
#include <X11/Xlib.h>
#include <sstream>
#include "color.hpp"
#include "shapes.hpp"
#include "label_base.hpp"
#include "window_base.hpp"
#include "graphics_context.hpp"
#include "pointer.hpp"

namespace xlib
{
   class label : public label_base
   {
   public:
      label ( window_base& parent, rectangle rect, std::string text)
      : m_display ( parent.get_display() ),
      m_parent ( parent ),
      m_text ( text ),
      //m_background ( parent.get_display(), 197, 194, 197 ), // grey
      m_rect ( rect )
      {
         draw_border = false;
         text_alignment = JUSTIFY_LEFT;
         text_xoffset = 5;
         text_yoffset = 0;
         m_window = 0;
         show();
      }

      virtual ~label()
      {
         destroy();
      }

      // From label_base:

      virtual void show()
      {
         create();
         XSelectInput ( m_display,
                        m_window,
                        ExposureMask |
                        SubstructureNotifyMask );

         XMapWindow ( m_display, m_window );
         XFlush ( m_display );
      }

      virtual void on_show(){ on_expose(); }

      virtual void hide()
      {
         XUnmapWindow ( m_display, m_window );
         XFlush ( m_display );
      }

      virtual void on_hide(){}

      virtual void create()
      {
         if ( m_window ) return;

         m_window = XCreateSimpleWindow ( m_display,
                                          m_parent.id(),
                                          m_rect.origin().x(),
                                          m_rect.origin().y(),
                                          m_rect.width(),
                                          m_rect.height(),
                                          0,
                                          WhitePixel((void*)m_display,0),
                                          WhitePixel((void*)m_display,0));

         if ( m_window == 0 )
         {
            throw create_edit_exception
            ( "could not create label" );
         }

         m_parent.get_event_dispatcher().register_window ( (window_base*) this );
         //set_background ( m_background );
         on_expose();
      }

      virtual void destroy()
      {
         if ( m_window )
         {
            XDestroyWindow ( m_display, m_window );
            m_window = 0;
         }

         m_parent.get_event_dispatcher().unregister_window ( (window_base*) this );
      }

      virtual void set_background ( color& c )
      {
         m_background = &c;

         XSetWindowBackground ( m_display, m_window, c.pixel() );

         XClearWindow ( m_display, m_window );

         XFlush ( m_display );
      }

      virtual rectangle get_rect()
      {
         Window root;
         int x = 0, y = 0;
         unsigned int width = 0, height = 0, border_width = 0, depth = 0;
         XGetGeometry ( m_display,
                        m_window,
                        &root,
                        &x,
                        &y,
                        &width,
                        &height,
                        &border_width,
                        &depth );

         return rectangle ( point(0,0), width, height );
      }

      virtual void set_focus() {}

      virtual void refresh ()
      {

         XClearWindow ( m_display,
                        m_window );

         XFlush ( m_display );

         on_expose();
      }

      /* window_base */
      virtual long id() { return m_window; }

      virtual void on_create() {}

      virtual void on_expose()
      {
         /* draw the label */
         rectangle rect = get_rect();

         graphics_context gc ( m_display, id() );

         gc.set_font(helvetica);

         color black ( m_display, 0, 0, 0 );
         color white ( m_display, 255, 255, 255 );
         color gray ( m_display, 131, 129, 131 );
         color ltgray (m_display, 197, 194, 197 );

         gc.set_foreground ( &black );

         /* draw the text */
         int x, y;

         rectangle text_rect = gc.get_text_rect ( m_text );

         y = rect.height()/2 + text_rect.height()/2;

         switch ( text_alignment ) {
            case JUSTIFY_CENTER:
               x = rect.width()/2 - text_rect.width()/2;
               break;
            case JUSTIFY_RIGHT:
               x = rect.width() - text_rect.width();
               break;
            case JUSTIFY_LEFT:
            default:
               x = 0;
         }

         x = x + text_xoffset;
         y = y + text_yoffset;

         gc.draw_text ( point(x, y), m_text );

         if (draw_border) {
            gc.set_foreground ( &ltgray );

            /* draw the borders */
            gc.draw_line ( line ( point(0,
                                        rect.height()-1),
                                  point(rect.width()-1,
                                        rect.height()-1) ) );
            // right
            gc.draw_line ( line ( point ( rect.width()-1,
                                          0 ),
                                  point ( rect.width()-1,
                                          rect.height()-1 ) ) );

            gc.set_foreground ( &black ); //was white

            // top
            gc.draw_line ( line ( point ( 0,0 ),
                                  point ( rect.width()-2, 0 ) ) );
            // left
            gc.draw_line ( line ( point ( 0,0 ),
                                  point ( 0, rect.height()-2 ) ) );

            gc.set_foreground ( &gray );

            // bottom
            gc.draw_line ( line ( point ( 1, rect.height()-2 ),
                                  point(rect.width()-2,rect.height()-2) ) );
            // right
            gc.draw_line ( line ( point ( rect.width()-2, 1 ),
                                  point(rect.width()-2,rect.height()-2) ) );
         }
      }

      virtual void on_destroy (){}

      /* label_base */
      virtual display& get_display() { return m_display; }
      virtual event_dispatcher& get_event_dispatcher()
      { return m_parent.get_event_dispatcher(); }

      const char* get_text() { return m_text.c_str(); }
      void set_text(std::string new_text) { m_text = new_text.c_str(); }

      Text_Alignment text_alignment;
      int text_xoffset;
      int text_yoffset;

      bool draw_border;

   private:

      std::string m_text;

      label ( const label& );
      void operator = ( label& );

      Window m_window;
      display& m_display;
      window_base& m_parent;

      color *m_background;
      rectangle m_rect;

   };

};

#endif

