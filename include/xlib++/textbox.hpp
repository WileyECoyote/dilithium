/* textbox.hpp
   definition of the xlib::text_box class
*/
/**
 * @par xlib++ - X Low level Widget Routines
 *
 * Copyright (C) 2013 Wiley Edward Hill
 *
 * This file was added to Rob Tougher's <robt@robtougher.com> collection
 * of c++ classes for creating widgets using low level X routines, with
 * out depending on G or K libs.
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

#ifndef _xlib_text_box_class_
#define _xlib_text_box_class_

#include "display.hpp"
#include <X11/Xlib.h>
#include <sstream>
#include <string>
#include "color.hpp"
#include "shapes.hpp"
#include "window_base.hpp"
#include "graphics_context.hpp"
#include "pointer.hpp"

enum Text_Alignment {
    JUSTIFY_LEFT,
    JUSTIFY_CENTER,
    JUSTIFY_RIGHT
};

namespace xlib
{
   class text_box_base : public window_base
   {
   public:
      virtual void on_click () = 0;
   };

   class text_box : public text_box_base
   {
   public:
      text_box ( window_base& parent, rectangle rect, std::string text)
      : m_display ( parent.get_display() ),
      m_parent ( parent ),
      m_text ( text ),
      m_background ( parent.get_display(), 255, 255, 255 ),
      m_is_down ( false ),
      m_is_mouse_over ( false ),
      m_has_focus ( false ),
      m_rect ( rect )
      {
         password_char[0] = '*';
         password_char[1]= '\0';
         mask_input = false;

         text_alignment = JUSTIFY_LEFT;
         text_xoffset = 7;
         text_yoffset = 0;

         position = 0;
         capslock = false;

         m_window = 0;
         show();
      }

      virtual ~text_box()
      {
         destroy();
      }

      // From window_base:
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
            throw create_edit_exception
            ( "could not create the command text_box" );
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

      void draw_cursor() {

         color black ( m_display, 200, 0, 0 );
         graphics_context gc ( m_display, id() );
         int x, y, y1, y2;

         gc.set_foreground ( &black );
         gc.set_font(helvetica);

         rectangle rect = get_rect();
         /* create temp str with chars upto position*/
         if ( position > 0 ) {
           std::string tmp_str = m_text.substr(0, position );
           x = gc.get_text_width(tmp_str) + text_xoffset;
         }
         else {
           x = text_xoffset;
         }

         y = rect.height()/2;
         y1 = y - ( y / 2 );
         y2 = y + ( y / 2 );

         gc.draw_line ( line ( point ( x, y1 ), point( x, y2) ) );
      }

      virtual void set_focus()
      {
         m_has_focus = true;
         XSetInputFocus ( m_display,
                          id(),
                          RevertToParent,
                          CurrentTime );
         refresh();
      }

      virtual void refresh ()
      {
         XClearWindow ( m_display, m_window );
         XFlush ( m_display );
         on_expose();
      }

      /* window_base */
      virtual long id() { return m_window; }

      virtual void on_create() {}

      virtual void on_expose()
      {

         /* draw the text_box */
         rectangle rect = get_rect();

         graphics_context gc ( m_display, id() );

         color black ( m_display, 0, 0, 0 );
         color white ( m_display, 255, 255, 255 );
         color gray ( m_display, 131, 129, 131 );
         color ltgray (m_display, 197, 194, 197 );

         if (!m_text.empty()) {
           gc.set_font(helvetica);

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

           if ( mask_input ) {
             std::string phony;
             for(std::string::size_type i = 0; i < m_text.size(); ++i) {
               phony += password_char;
             }
             gc.draw_text ( point(x, y), phony );
           }
           else {
             gc.draw_text ( point(x, y), m_text );
           }
         }
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
         if ( m_has_focus ) {
           draw_cursor();
         }
      }

      virtual void on_left_button_down ( int x, int y )
      {
         /* Need to set the cursor */
         m_is_down = true;
         set_focus();
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

      virtual void on_key_press ( character c ) {

         if ( c.is_End_key() ) {
           if (position < m_text.length() ) {  // If the string is 1 or more chars long
             position = m_text.length();
           }
         }
         else if ( c.is_Home_key() ) {
           if (position > 0) {  // If the string is 1 or more chars long
             position = 0;
           }
         }
         else if ( c.is_Delete_key() ) {
           if ( position < m_text.length() ) {  // If the string is 1 or more chars long
             m_text.erase(position,1);          // remove the character
             position--;
           }
         }
         else if ( c.is_Backspace_key() ) {
           if (!m_text.empty()){  // If the string is 1 or more chars long
             m_text.erase(m_text.size() - 1); // remove the last character
             position--;
           }
         }
         else if ( c.is_Left_Arrow_key() ) {
           if (m_text.length() > 0) {  // If the string is 1 or more chars long
             position--;
           }
         }
         else if ( c.is_Right_Arrow_key() ) {
           if ( position < m_text.length() ) {  // If the string is 1 or more chars long
             position++;
           }
         }
         else if ( c.is_printable() ) {
            /*if ( shifted == true) {
               key=toupper(key);        // If shift is on then upper case the key
            } */
           m_text.append(c.get_char());
           position++;
         }
         refresh();
      }

      virtual void on_key_release( character c ) {};

      virtual void on_destroy (){}

      /* text_box_base */
      virtual void on_click () {}

      virtual display& get_display() { return m_display; }
      virtual event_dispatcher& get_event_dispatcher()
      { return m_parent.get_event_dispatcher(); }

      const char* get_text() { return m_text.c_str(); }
      void set_text(std::string new_text) { m_text = new_text.c_str(); }

      void set_password_char(char char_mask) { password_char[0] = char_mask; }

      Text_Alignment text_alignment;
      int text_xoffset;
      int text_yoffset;

      bool mask_input;

   private:

      std::string m_text;

      text_box ( const text_box& );
      void operator = ( text_box& );

      Window m_window;
      display& m_display;
      window_base& m_parent;
      bool m_is_down, m_is_mouse_over, m_has_focus;
      color m_background;
      rectangle m_rect;
      int position;
      int capslock;

      char password_char[2];
   };

};

#endif

