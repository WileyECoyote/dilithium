/* libxlogin.cc
   Convenience library Source file for the Dilithium Program.
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
/*!   @file    libxlogin.cc C++ Source file for the login_window class
 *    @brief   libxlogin provides a login Dialog using low-level xlibs
 *    @author  Wiley Edward Hill
 *    @date    2012.07.04
 */
const char *helvetica = "-*-helvetica-*-r-*-*-14-*-*-*-*-*-*-*";

#define AUTHNAME "shadow\0"
#define _XOPEN_SOURCE           /* define before 1st #include */

#include "config.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <pwd.h>                 /* needed for pwd & getpwnam_r */
#include "upap.h"
#include <syslog.h>

#include <shadow.h>

#include <X11/cursorfont.h>

#include "xlib++/display.hpp"
#include "xlib++/glxwindow.hpp"
#include "xlib++/event_dispatcher.hpp"
#include "xlib++/graphics_context.hpp"
#include "xlib++/button.hpp"
#include "xlib++/textbox.hpp"
#include "xlib++/color.hpp"

#include "privileges.h"
#include "dilithium.h"
#include "colors.h"
#include "xjpeg.h"
#include "xlogin.h"

static int login_answer;
static char user[MAXUSERNAMESIZE];

using namespace xlib;

/**
 * \class login_window
 *
 * \ingroup Dilithium
 *
 * \brief Implementation of the login_window class.
 *
 * The login_window class provides an object oriented login Dialog,
 * implemented using ONLY low-level X11 routines. This is done so that
 * Dilithium is not dependent on GTK or KDE libraries, which may not be
 * installed on a fvwm machine. (Typically distros would require one of
 * them because fvwm does not provide a means to log-in users.)
 * This module over-comes that obstacle by providing such a means. The
 * dialog object is instantiated by the Xlogin class, implemented later
 * in this file. Dilithium utilized the login dialog only if the user
 * information can not otherwise be obtained, such as when Dilithium is
 * started as a daemon from an init.d script during boot-up. This is not
 * really for security; the user information is needed by both the server
 * and the client for user specific customization files. No login is
 * required if the --user switch was given as a parameter. The computer
 * might just be an insecure "one-user" machine that boots straight into
 * a GUI.
 */

class login_window;

/*!@par 2 Text Input Boxes derived from xlib++::text_box */
class username_text_box : public text_box
{
public:
  username_text_box ( login_window& w );
  ~username_text_box(){}

  void on_key_release ( character c );
  void on_click(){};

private:
  login_window& m_parent;
};

class password_text_box : public text_box
{
public:
  password_text_box ( login_window& w );
  ~password_text_box(){}
  void on_key_release ( character c );
  void on_click(){};

private:
  login_window& m_parent;
};

/*!@par 4 Buttons derived from xlib++::button */
class okay_button : public button
{
public:
  okay_button ( login_window& w );
  ~okay_button(){}
  void on_key_release ( character c );
  void on_click();

private:
  login_window& m_parent;
};

class quit_button : public button
{
public:
  quit_button ( login_window& w );
  ~quit_button(){}
  void on_key_release ( character c );
  void on_click();

private:
  login_window& m_parent;
};

class reboot_button : public button
{
public:
  reboot_button ( login_window& w );
  ~reboot_button(){}
  void on_key_release ( character c );
  void on_click();

private:
  login_window& m_parent;
};

class shutdown_button : public button
{
public:
  shutdown_button ( login_window& w );
  ~shutdown_button(){}
  void on_key_release ( character c );
  void on_click();

private:
  login_window& m_parent;
};

/*!@par Main Dialog Window Object */
class login_window : public glxwindow
{
public:

  Display* m_display;
  graphics_context *gc;
  Atom wmDeleteMessage;

  XFontStruct* font;       /* Font structure */
  std::string  font_name;

  login_window ( event_dispatcher& e, rectangle r ) : glxwindow ( e, r )
  {
     dispatcher = &e;

     m_display = get_display();

     gc = new graphics_context ( get_display(), m_window );
     gc->set_font(helvetica);

     wmDeleteMessage = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
     XSetWMProtocols(m_display, m_window, &wmDeleteMessage, 1);

     XStoreName( m_display, m_window, "Dilithium Login");

     m_username = new username_text_box ( *this );
     m_password = new password_text_box ( *this );

     m_okay     = new okay_button ( *this );
     m_quit     = new quit_button ( *this );
     m_reboot   = new reboot_button ( *this );
     m_shutdown = new shutdown_button ( *this );

     focus_username(); /*set inital focus */
  }
  ~login_window(){
     delete gc;
     // destroy editboxes
     delete m_username;
     delete m_password;
     // destroy buttons
     delete m_okay;
     delete m_quit;
     delete m_reboot;
     delete m_shutdown;
  }

  void initialize( int screen );

  /* Focus */
  void focus_username() {
     m_username->set_focus();
  }
  void focus_password() {
     m_password->set_focus();
  }

  void focus_okay_button() {
     m_okay->set_focus();
  }
  void focus_quit_button() {
     m_quit->set_focus();
  }
  void focus_reboot_button() {
     m_reboot->set_focus();
  }
  void focus_shutdown_button() {
     m_shutdown->set_focus();
  }

  void exit(int response)
  {
    login_answer = response;
    dispatcher->stop();
  }

  int auth();

  /*!@par Called from siblings when ready */
  bool do_login() {

    username = m_username->get_text();
    if (username.empty() ) {
      m_username->set_focus();
    }
    else {
      password = m_password->get_text();
      if (password.empty() ) {
        m_password->set_focus();
      }
      else {
        if ( auth() == UPAP_AUTHACK) {
          strcpy( &user[0], username.c_str());
          return true;
        }
        else {
          m_password->set_focus();
        }
      }
    }
    return false;
  }

/*! \brief login_window  Hotkey Handler
 *  \par Function Description
 *    Yes, hotkeys. This function is a virtual over-ride of the
 * base window on_key_release in order to implement "Hotkeys".
 * Input is only received when one of the button has focus.
 *
 * \note This function is called before sibling on_key_release
 */
  void on_key_release ( character c )
  {
     char hotkey;
     hotkey = *c.get_char();
     switch (hotkey) {
        case 'l':
        case 'L':
           if(do_login()) {
              exit(Login);
           }
           break;
        case 'q':
        case 'Q':
           exit(Quit);
           break;
        case 'r':
        case 'R':
           exit(Reboot);
           break;
        case 's':
        case 'S':
           exit(Shutdown);
           break;
     }
  }

/*! \brief login_window::on_expose, aka our On_Draw
 *  \par Function Description
 *    Controls are drawn by sibling classes so the only things
 *  we need to put on the dialog are labels for the textboxes.
 *  This is simpler then using the label class as widgets and
 *  allow for a "transparent" effect.
 */
  virtual void on_expose()
  {
     add_label( 30, 45, "Username:");
     add_label( 30, 95, "Password:");
  }

private:

  event_dispatcher *dispatcher;

  std::string username;
  std::string password;

  username_text_box *m_username;
  password_text_box *m_password;

  okay_button     *m_okay;
  quit_button     *m_quit;
  reboot_button   *m_reboot;
  shutdown_button *m_shutdown;

  void add_label ( int x, int y, std::string text );

};

/*! \brief login_window Authorization Validation
 *  \par Function Description
 *  This function attempts to validated the username and password
 *  provided by the user.
 *
 * \retval int either: UPAP_AUTHACK = good to go
 *                     UPAP_AUTHNAK = no bueno
 */
int login_window::auth()
{
  struct passwd *pw;
  struct spwd *sp;

  char *encrypted, *correct;

  int ret_val;

  ret_val = -1;
  if ( username.empty() ) {
    ret_val = UPAP_AUTHNAK;
  }
  else {

    pw = getpwnam(username.c_str());
    endpwent();

    if (!pw) {
      ret_val = UPAP_AUTHNAK; //user doesn't really exist
    }
    else {

      sp = getspnam( username.c_str() );
      endspent();

      correct = sp ? sp->sp_pwdp : pw->pw_passwd;

      encrypted = crypt(password.c_str(), correct);

      ret_val= strcmp(encrypted, correct) ? UPAP_AUTHNAK : UPAP_AUTHACK;
    }
  }
  if ( ret_val == UPAP_AUTHACK) {
    syslog(LOG_INFO, "user %s logged in", username.c_str());
  }

  return ret_val;       /* Return result */
}

/*! \brief login_window Add labels for text edting fields
 *  \par Function Description
 *  This function put the string on the windows are the
 *  given XY location.
 *
 */
void login_window::add_label ( int x, int y, std::string text )
{
  color black ( get_display(), 0, 0, 0 );

  rectangle text_rect = gc->get_text_rect ( text );

  gc->set_foreground ( &black );
  gc->draw_text ( point(x, y), text );

}

/*! @brief Username Text Box */
username_text_box::username_text_box ( login_window& w )
  : text_box ( w, rectangle(point(111,25),180,32 ), "" ),
    m_parent ( w )
{ SetBackgroundColor ( TEST_ENTRY_BG_COLOR ); }

void username_text_box::on_key_release ( character c )
{
  if ( c.is_Cancel_key() || c.is_Escape_key() ) {
    m_parent.exit(Quit);
  }
  else if ( c.is_Return_key() || c.is_Tab_key() ) {
     m_parent.focus_password();
  }
  else { /* We're not interested, pass to base class */
     text_box::on_key_release ( c );
  }
}

/*! @brief Password Text Box */
password_text_box::password_text_box ( login_window& w )
  : text_box ( w, rectangle(point(111,75),180,32 ), "" ),
    m_parent ( w )
{ SetBackgroundColor ( TEST_ENTRY_BG_COLOR ); mask_input = true; }

void password_text_box::on_key_release ( character c )
{
  if ( c.is_Cancel_key() || c.is_Escape_key() ) {
    m_parent.exit(Quit);
  }
  else if ( c.is_Return_key() ) {
     if (m_parent.do_login()) {
        m_parent.exit(Login);
     }
  }
  else if ( c.is_Tab_key() ) {
     m_parent.focus_okay_button();
  }
  else { /* We're not interested, pass to base class */
     text_box::on_key_release ( c );
  }
}

/*!@par convenience macros for the size and positioning of Buttons */
#define BUTTON_X 80
#define BUTTON_Y 32
#define BUTTON_SIZE BUTTON_X, BUTTON_Y
#define BUTTON_SPACING 8

#define BUTTON_X_POS_1 10
#define BUTTON_X_POS_2 BUTTON_X_POS_1 + BUTTON_X + BUTTON_SPACING
#define BUTTON_X_POS_3 BUTTON_X_POS_2 + BUTTON_X + BUTTON_SPACING
#define BUTTON_X_POS_4 BUTTON_X_POS_3 + BUTTON_X + BUTTON_SPACING

/*! @brief Okay button */
okay_button::okay_button ( login_window& w )
  : button ( w, rectangle(point( BUTTON_X_POS_1,155), BUTTON_SIZE ), "Login" ),
    m_parent ( w )
{
  SetBackgroundColor ( BUTTOM_COLOR );
  SetHighLightColor  ( BUTTON_HIGHLIGHT );
  SetShadowColor     ( BUTTON_SHADOW );
  SetSelectedColor   ( BUTTON_SELECTED );
}
void okay_button::on_click() {

  if(m_parent.do_login()) {
     m_parent.exit(Login);
  }
}
void okay_button::on_key_release ( character c )
{
  if ( c.is_Tab_key() ) {
    m_parent.focus_quit_button();
  }
  else if ( c.is_Return_key() ) {
    if(m_parent.do_login()) {
      m_parent.exit(Login);
    }
  }
  else {
    m_parent.on_key_release ( c );
    /* no need to call the base class, it's a button */
  }
}

/*! @brief  Quit button */
quit_button::quit_button ( login_window& w )
  : button ( w, rectangle(point( BUTTON_X_POS_2, 155), BUTTON_SIZE ), "Quit" ),
    m_parent ( w )
{
  SetBackgroundColor ( BUTTOM_COLOR );
  SetHighLightColor  ( BUTTON_HIGHLIGHT );
  SetShadowColor     ( BUTTON_SHADOW );
  SetSelectedColor   ( BUTTON_SELECTED );
}
void quit_button::on_click() { m_parent.exit(Quit); }
void quit_button::on_key_release ( character c )
{
  if ( c.is_Tab_key() ) {
     m_parent.focus_reboot_button();
  }
  else if ( c.is_Return_key() ) {
     m_parent.exit(Quit);
  }
  else {
   m_parent.on_key_release ( c );
  }
}

/*! @brief Reboot button */
reboot_button::reboot_button ( login_window& w )
  : button ( w, rectangle(point( BUTTON_X_POS_3, 155), BUTTON_SIZE ), "Reboot" ),
    m_parent ( w )
{
  SetBackgroundColor ( BUTTOM_COLOR );
  SetHighLightColor  ( BUTTON_HIGHLIGHT );
  SetShadowColor     ( BUTTON_SHADOW );
  SetSelectedColor   ( BUTTON_SELECTED );
}
void reboot_button::on_click() { m_parent.exit(Reboot); }
void reboot_button::on_key_release ( character c )
{
  if ( c.is_Tab_key() ) {
     m_parent.focus_shutdown_button();
  }
  else if ( c.is_Return_key() ) {
     m_parent.exit(Reboot);
  }
  else {
    m_parent.on_key_release ( c );
  }
}

/*! @brief Shutdown button */
shutdown_button::shutdown_button ( login_window& w )
  : button ( w, rectangle(point(BUTTON_X_POS_4, 155), BUTTON_SIZE ), "Shutdown" ),
    m_parent ( w )
{
  SetBackgroundColor ( BUTTOM_COLOR );
  SetHighLightColor  ( BUTTON_HIGHLIGHT );
  SetShadowColor     ( BUTTON_SHADOW );
  SetSelectedColor   ( BUTTON_SELECTED );
}
void shutdown_button::on_click() { m_parent.exit(Shutdown); }
void shutdown_button::on_key_release ( character c )
{
  if ( c.is_Tab_key() ) {
     m_parent.focus_username();
  }
  else if ( c.is_Return_key() ) {
     m_parent.exit(Shutdown);
  }
  else {
    m_parent.on_key_release ( c );
  }
}

/** ************************* Xlogin Class ******************************/
/**
 * \class Xlogin
 *
 * \ingroup Dilithium
 *
 * \brief Implementation of Xlogin class.
 *
 * The Xlogin class provides an wrapper interface for Dilithium to the
 * login_window class. Dilithium does not interact with the dialog object
 * directly. Xlogin performs customization of the object, which attempts
 * to be generic. The entire "gui" portion is encapsulted by an exception
 * handler as a fail-safe measure.
 *
 */

/*!@par Xlogin helper to calculate center positioning of the dialog */
void Xlogin::get_dimensions()
{
  width  = DEFWIDTH;
  height = DEFHEIGHT;

 /* Calculate the window position using the X server
  * dimensions and the window size */
  xwidth=DisplayWidth(m_display,DefaultScreen(m_display));
  xheight=DisplayHeight(m_display,DefaultScreen(m_display));
  THISWINX=(xwidth/2)-(width/2);      /* top left is dead center */
  THISWINY=(xheight/2)-(height/2);
}

/*!@par Xlogin helper to setup the X default environment */
void Xlogin::setup_display()
{
  screen = DefaultScreen (m_display);

  window_attributes.border_pixel = BlackPixel (m_display, screen);
  window_attributes.background_pixel = BlackPixel (m_display, screen);
  window_attributes.override_redirect = 0;
  window_mask = CWBackPixel | CWBorderPixel;

  /* Turn-off the pesky screen saver */
  XSetScreenSaver(m_display, 0, 0, DontPreferBlanking, DefaultExposures);

  XStoreName (m_display, window, THIS_PROGRAM);   // Tell window manager our name

  /* Enable the X left facing cursor */
  Cursor cursor;
  cursor=XCreateFontCursor(m_display, XC_left_ptr);
  XDefineCursor(m_display, window, cursor);
  XFreeCursor(m_display, cursor);

}

int Xlogin::load_background()
{
   int didxcreate = false;
   int sucess     = false;
   const char *filename;

   XImage *xibackground;

   filename = dilithium->login_background.c_str();

   image_gc = XCreateGC (m_display, window, 0, 0);

   if ( access ( filename, R_OK) == 0) { /* If the file exist */
      xibackground=jpeg_decode(filename, m_display, xdepth, 1, 1, &didxcreate, &sucess);
      if ( sucess ) { /* If we dont have an image (error) */
        XPutImage (m_display, window, image_gc, xibackground, 0, 0, 0, 0,xibackground->width, xibackground->height);
      }
      if (didxcreate) {
        XDestroyImage(xibackground);
      }
   }

   return sucess;
}
/*! \brief Xlogin Class Module Entry
 *  \par Function Description
 *  This function is the main entry point for this compilation unit
 *  and the only one referenced by a routine out-side of this module.
 *  The function is instantiated from within the main-line code in the
 *  Dilithium module. If the log-in is successfull then the user_name
 *  field will contain the user's name. If the user elects to select
 *  the Quit, Reboot, or Shutdown options then the user information is
 *  not updated.
 *
 * \param dilithium a Dilithium object
 * \retval int enumerated WhatDoNext: 0 = Login,
 *                                    1 = Quit,
 *                                    2 = Reboot,
 *                                    3 = Shutdown
 */
int Xlogin::login()
{
  login_answer = -1;

  try {
      display d (dilithium->display);
      m_display = d;

      color background( d, MAIN_WINDOW_BG_COLOR);

      event_dispatcher events ( d );
      get_dimensions();
      login_window w ( events, rectangle(point(THISWINX,THISWINY), width, height));

      w.set_background( background );

      window = w.m_window;

      xdepth = w.get_depth();

      setup_display();
      load_background();
      events.run();
  }
  catch ( exception_with_text& e ) {
      std::cout << "Exception: " << e.what() << "\n";
  }

  /* If the response was "login" then save the user's name */
  if (login_answer == Login) {
    dilithium->user_name = &user[0];
  }
  return login_answer;
}

/*!@par Xlogin helper to close the dialog from an external routine
 * @note This is not used by the Dilithium program */
void Xlogin::close()
{
   XDestroyWindow(m_display, window);  // Remove the window - but dont close the connection
   XUnmapWindow(m_display, window);
   XFlush(m_display);
}

/*!@par Xlogin Constructor */
Xlogin::Xlogin (Dilithium *lithium) {

  dilithium = lithium;

  colorcursor             = 0xffffff;
  font_name               = helvetica;
}