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

// definition of the xlib::button class

#ifndef _xlib_button_class_
#define _xlib_button_class_

#include <string>
#include "display.hpp"
#include <X11/Xlib.h>
#include <sstream>
#include "color.hpp"
#include "shapes.hpp"
#include "window_base.hpp"
#include "graphics_context.hpp"
#include "pointer.hpp"

namespace xlib
{
   class button_base : public window_base
   {
   public:
      virtual void on_click () = 0;
   };

   class button : public button_base
   {
   public:
      button ( window_base& parent, rectangle rect, std::string name )
      : m_display ( parent.get_display() ),
      m_parent ( parent ),
      m_name ( name ),
      m_background ( parent.get_display(), 197, 194, 197 ), // grey
      m_is_down ( false ),
      m_is_mouse_over ( false ),
      m_has_focus ( false ),
      m_rect ( rect )
      {
         m_foreground      = new color ( m_display, 0, 0, 0 ); // default to Black
         m_border_color    = new color ( m_display, 0, 0, 0 ); // default to Black
         m_shadow_color    = new color ( m_display, 131, 129, 131 ); // default to Black
         m_highlight_color    = new color ( m_display, 255, 255, 255 ); // default White
         m_highlight_pressed  = new color ( m_display, 131, 129, 131 ); // default Grey
         m_selected_color     = new color ( m_display, 131, 129, 131 ); // default Grey

         m_window = 0;
         show();
      }

      virtual ~button()
      {

        delete m_foreground;
        delete m_border_color;
        delete m_highlight_color;
        delete m_highlight_pressed;
        delete m_selected_color;
        delete m_shadow_color;

        destroy();
      }

      /* From window_base */
      virtual void show()
      {
         create();
         XSelectInput ( m_display,
                        m_window,
                        ExposureMask |
                        ButtonPressMask |
                        ButtonReleaseMask |
                        EnterWindowMask |
                        LeaveWindowMask |
                        PointerMotionMask |
                        FocusChangeMask |
                        KeyPressMask |
                        KeyReleaseMask |
                        SubstructureNotifyMask );

         XMapWindow ( m_display, m_window );
         XFlush ( m_display );
      }

      virtual void on_show(){}

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
            throw create_button_exception
            ( "could not create the command button" );
         }

         m_parent.get_event_dispatcher().register_window ( (window_base*) this );
         set_background ( m_background );
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
         m_background.set ( c );

         XSetWindowBackground ( m_display, m_window, c.pixel() );

         XClearWindow ( m_display, m_window );

         XFlush ( m_display );
      }
      virtual void SetBackgroundColor ( short red, short green, short blue )
      {
         m_background.set_color ( red, green, blue );

         XSetWindowBackground ( m_display, m_window, m_background.pixel() );

         XClearWindow ( m_display, m_window );

         XFlush ( m_display );
      }
      virtual void SetShadowColor ( short red, short green, short blue )
      {
         m_shadow_color->set_color ( red, green, blue );
         refresh();
      }
      virtual void SetHighLightColor ( short red, short green, short blue )
      {
         m_highlight_color->set_color ( red, green, blue );
         refresh();
      }
      virtual void SetSelectedColor ( short red, short green, short blue )
      {
         m_selected_color->set_color ( red, green, blue );
         refresh();
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

      virtual void set_focus()
      {
         XSetInputFocus ( m_display,
                          id(),
                          RevertToParent,
                          CurrentTime );
         refresh();
      }

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
         //m_background.set_color(197, 194, 197);
         //set_background ( m_background );

         // draw the button
         rectangle rect = get_rect();

         graphics_context gc ( m_display, id() );

         gc.set_font(helvetica);

         if (m_has_focus) {
           gc.set_foreground ( m_selected_color );
         }
         else {
           gc.set_foreground ( m_foreground );
         }

         // draw the text
         rectangle text_rect = gc.get_text_rect ( m_name );

         if ( m_is_down && m_is_mouse_over )
         {
            gc.draw_text ( point(rect.width()/2 - text_rect.width()/2 + 1,
                                 rect.height()/2 + text_rect.height()/2 + 1 ),
                           m_name );
         }
         else
         {
            gc.draw_text ( point(rect.width()/2 - text_rect.width()/2,
                                 rect.height()/2 + text_rect.height()/2),
                           m_name );
         }

         /* draw the borders */
         if ( ! m_is_down || ! m_is_mouse_over )
         {

            gc.set_foreground ( m_shadow_color );

            // bottom
            gc.draw_line ( line ( point(0,
                                        rect.height()-1),
                                  point(rect.width()-1,
                                        rect.height()-1) ) );
            // right
            gc.draw_line ( line ( point ( rect.width()-1,
                                          0 ),
                                  point ( rect.width()-1,
                                          rect.height()-1 ) ) );

            // Top
            gc.set_foreground ( m_highlight_color );

            gc.draw_line ( line ( point ( 0,0 ),
                                  point ( rect.width()-2, 0 ) ) );
            // left
            gc.draw_line ( line ( point ( 0,0 ),
                                  point ( 0, rect.height()-2 ) ) );

            // bottom
            gc.set_foreground ( m_shadow_color );

            gc.draw_line ( line ( point ( 1, rect.height()-2 ),
                                  point(rect.width()-2,rect.height()-2) ) );
            // right
            gc.draw_line ( line ( point ( rect.width()-2, 1 ),
                                  point(rect.width()-2,rect.height()-2) ) );

         }
         else {

            gc.set_foreground ( m_highlight_color );

            // bottom
            gc.draw_line ( line ( point(1,rect.height()-1),
                                  point(rect.width()-1,rect.height()-1) ) );
            // right
            gc.draw_line ( line ( point ( rect.width()-1, 1 ),
                                  point ( rect.width()-1, rect.height()-1 ) ) );

            gc.set_foreground ( m_border_color );

            // top
            gc.draw_line ( line ( point ( 0,0 ),
                                  point ( rect.width()-1, 0 ) ) );
            // left
            gc.draw_line ( line ( point ( 0,0 ),
                                  point ( 0, rect.height()-1 ) ) );


            gc.set_foreground ( m_highlight_pressed );

            // top
            gc.draw_line ( line ( point ( 1, 1 ),
                                  point(rect.width()-2,1) ) );
            // left
            gc.draw_line ( line ( point ( 1, 1 ),
                                  point( 1, rect.height()-2 ) ) );
         }
      }

      virtual void on_left_button_down ( int x, int y )
      {
         m_is_down = true;
         refresh();
      }

      virtual void on_right_button_down ( int x, int y ) {}

      virtual void on_left_button_up ( int x, int y )
      {
         if ( m_is_down && m_is_mouse_over )
         {
            on_click();
         }

         m_is_down = false;
         refresh();
      }

      virtual void on_right_button_up ( int x, int y ) {}


      virtual void on_mouse_enter ( int x, int y )
      {
         m_is_mouse_over = true;

         pointer p(m_display);

         if ( p.is_left_button_down() && m_is_down )
         {
            refresh();
         }
         else if ( ! p.is_left_button_down() && m_is_down )
         {
            m_is_down = false;
         }
      };

      virtual void on_mouse_exit ( int x, int y )
      {
         m_is_mouse_over = false;
         if ( m_is_down )
            refresh();
      };

      virtual void on_mouse_move ( int x, int y ) {};

      virtual void on_got_focus()
      {
         m_has_focus = true;
         refresh();
      };

      virtual void on_lost_focus()
      {
         m_has_focus = false;
         refresh();
      };

      virtual void on_key_press ( character c ) {};
      virtual void on_key_release( character c ) {};

      virtual void on_destroy (){}

      /* button_base */
      virtual void on_click () {}

      virtual display& get_display() { return m_display; }
      virtual event_dispatcher& get_event_dispatcher()
      { return m_parent.get_event_dispatcher(); }

   private:

      button ( const button& );
      void operator = ( button& );

      Window m_window;
      display& m_display;
      window_base& m_parent;
      color m_background;
      bool m_is_down, m_is_mouse_over, m_has_focus;
      std::string m_name;
      rectangle m_rect;

      color *m_foreground;
      color *m_border_color;
      color *m_highlight_color;
      color *m_shadow_color;
      color *m_highlight_pressed;
      color *m_selected_color;


   };

};

#endif

