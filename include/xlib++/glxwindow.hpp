/* glxwindow.hpp
   definition of the xlib::glxwindow class
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

#ifndef _xlib_window_class_
#define _xlib_window_class_

#include <string>
#include <sstream>

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "display.hpp"
#include "window_base.hpp"
#include "event_dispatcher.hpp"
#include "color.hpp"
#include "shapes.hpp"

namespace xlib
{
   const int event_mask = ExposureMask |
   ButtonPressMask |
   ButtonReleaseMask |
   OwnerGrabButtonMask |
   EnterWindowMask |
   LeaveWindowMask |
   PointerMotionMask |
   FocusChangeMask |
   KeyPressMask |
   KeyReleaseMask |
   SubstructureNotifyMask |
   StructureNotifyMask |
   SubstructureRedirectMask;


   int VisData[] = {
     GLX_RENDER_TYPE, GLX_RGBA_BIT,
     GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
     GLX_DOUBLEBUFFER, True,
     GLX_RED_SIZE, 8,
     GLX_GREEN_SIZE, 8,
     GLX_BLUE_SIZE, 8,
     GLX_ALPHA_SIZE, 8,
     GLX_DEPTH_SIZE, 16,
     None
   };

   class glxwindow : public window_base
   {
   public:

      /*  To create a window */
      glxwindow ( event_dispatcher& e, rectangle r = rectangle(point(0,0),300,200) )
      : m_display ( e.get_display() ),
      m_event_dispatcher ( e ),
      m_is_child ( false ),
      m_rect ( r ),
      m_parent ( 0 )
      {
         m_window = 0;
         m_atom[0] = 0;
         show();
      }

      /* For existing windows */
      glxwindow ( event_dispatcher& e, int id ) : m_display ( e.get_display() ),
      // m_background ( e.get_display(), 213, 206, 189 ), // sand
      //m_background ( e.get_display(), 197, 194, 197 ), // grey
      m_event_dispatcher ( e ),
      m_is_child ( false ),
      m_rect ( point(0,0), 0, 0 ),
      m_parent ( 0 ),
      m_window ( id )
      {
         m_atom[0] = 0;
      }

      /* For a child window. 'w' is its parent. */
      glxwindow (  glxwindow& w  ) : m_display ( w.get_event_dispatcher().get_display() ),
      //m_background ( m_display, 213, 206, 189 ),
      m_event_dispatcher ( w.get_event_dispatcher() ),
      //m_border ( m_display, 255, 255, 255 ),
      m_is_child ( true ),
      m_rect ( w.get_rect() ),
      m_parent ( w.id() )
      {
         m_window = 0;
         m_atom[0] = 0;
         show();
      }

      virtual ~glxwindow()
      {
         destroy();
      }

      /* From window_base: */
      virtual void show()
      {
         create();

         XSelectInput ( m_display,
                        m_window,
                        event_mask );

         XMapWindow ( m_display, m_window );
         XFlush ( m_display );
      }

      virtual void on_show(){}

      virtual void hide()
      {
         XUnmapWindow ( m_display, m_window );
         XFlush ( m_display );
      }

      virtual void on_hide()
      {}

      virtual void create()
      {
         if ( ! m_window )
         {
            /* Open GL Initialization */
            int numfbconfigs;
            int attr_mask;

            XSetWindowAttributes attr = {0,};

            Display* dply = m_display.get();
            Xscreen       = DefaultScreen( dply );
            Xroot         = RootWindow(dply, Xscreen);

            fbconfigs = glXChooseFBConfig( dply, Xscreen, VisData, &numfbconfigs);
            fbconfig  = 0;
            for(int i = 0; i<numfbconfigs; i++) {

               visual = (XVisualInfo*) glXGetVisualFromFBConfig(dply, fbconfigs[i]);
               if(!visual)
                  continue;

               pict_format = XRenderFindVisualFormat(dply, visual->visual);
               if(!pict_format)
                  continue;

               fbconfig = fbconfigs[i];
               if(pict_format->direct.alphaMask > 0) {
                  break;
               }
            }

            if(fbconfig) {
        /* Create a colormap - only needed on some X clients, eg. IRIX */
               color_map = XCreateColormap(dply, Xroot, visual->visual, AllocNone);

               attr.colormap = color_map;
               attr.background_pixmap = None;
               attr.border_pixmap = None;
               attr.border_pixel = 0;
               attr.event_mask = event_mask;

               attr_mask = CWBackPixmap |
                           CWColormap |
                           CWBorderPixel |
                           CWEventMask;

               m_window = XCreateWindow ( dply,
                                          Xroot,
                                          m_rect.origin().x(),
                                          m_rect.origin().y(),
                                          m_rect.width(),
                                          m_rect.height(),
                                          0,
                                          visual->depth,
                                          InputOutput,
                                          visual->visual,
                                          attr_mask, &attr);

            }
            if ( m_window == 0 )
            {
               throw create_window_exception
               ( "could not create the window" );
            }

            int glXattr[] = { None };
            glX_window = glXCreateWindow(dply, fbconfig, m_window, glXattr);
            if( !glX_window ) {
               throw create_window_exception
               ( "Could not create GLX window\n" );
            }

            //set_background ( m_background );

            if ( m_is_child )
            {

               // keeps this window in front of its parent at all times
               XSetTransientForHint ( dply,
                                      id(),
                                      m_parent );

               // make sure the app doesn't get killed when this
               // window gets destroyed
               m_atom[0] = XInternAtom ( dply,
                                         "WM_DELETE_WINDOW",
                                         false );

               XSetWMProtocols ( dply,
                                 m_window,
                                 m_atom,
                                 1 );
            }

            on_create();

         }

         m_event_dispatcher.register_window ( this );

      }

      virtual void on_create(){}

      virtual void destroy()
      {
         hide();

         if ( m_window )
         {
            XDestroyWindow ( m_display, m_window );
            m_window = 0;
         }

         m_event_dispatcher.unregister_window ( this );

         on_destroy();
      }

      virtual void set_background ( color& c )
      {
         // hold a ref to the alloc'ed color
         //m_background.set ( c );

         XSetWindowBackground ( m_display,
                                m_window,
                                c.pixel() );

         refresh();

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

         return rectangle ( point(x,y), width, height );
      }

      virtual long id() { return m_window; }

      virtual void on_expose() {}

      virtual void on_left_button_down ( int x, int y ) {}
      virtual void on_right_button_down ( int x, int y ) {}

      virtual void on_left_button_up ( int x, int y ) {}
      virtual void on_right_button_up ( int x, int y ) {}

      virtual void on_mouse_enter ( int x, int y ) {};
      virtual void on_mouse_exit ( int x, int y ) {};
      virtual void on_mouse_move ( int x, int y ) {};

      virtual void on_got_focus() {};
      virtual void on_lost_focus() {};

      virtual void on_key_press ( character c ) {};
      virtual void on_key_release( character c ) {};

      virtual void on_destroy (){}

      display& get_display() { return m_display; }
      virtual event_dispatcher& get_event_dispatcher()
      { return m_event_dispatcher; }

      virtual int get_depth(){ return visual->depth; }
      virtual Colormap get_color_map(){ return color_map; }

      int Xscreen;
      Window Xroot;
      XVisualInfo *visual;

      Window m_window;
      GLXWindow glX_window;

      rectangle m_rect;

      GLXFBConfig fbconfig;

      GLXFBConfig *fbconfigs;

   private:

      /* Not copyable */
      glxwindow ( const glxwindow& );
      void operator = ( glxwindow& );

      display& m_display;

      Window m_parent;

      Colormap color_map;

      //color m_background;
      //color m_border;

      event_dispatcher& m_event_dispatcher;

      Atom m_atom[1];

      XRenderPictFormat *pict_format;

      bool m_is_child;

   };

};

#endif

