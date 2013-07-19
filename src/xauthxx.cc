/* Xauth++
 *
 * Copyright (C) 2008  David Munger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110
 * -1301 USA
 * 
 * Author(s):
 * 	David Munger <mungerd@gmail.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/stat.h>

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

#include "global.h"
#include "xauthxx.h"

#ifdef HAVE_GCRYPT
# include <gcrypt.h>
#endif

#define MAGIC_COOKIE_NAME "MIT-MAGIC-COOKIE-1"

using namespace Xau;

/******************************************************************************/

/* class InternetAddress */

InternetAddress::InternetAddress(const InternetAddress& addr)
    : Address(addr.data(), 4)
{
    if (addr.length() != 4)
        throw Error(ERROR_INVALID_ADDRESS, "Internet addresses must be exactly 4 bytes long");
}

InternetAddress::InternetAddress(int a0, int a1, int a2, int a3)
    : Address("\0\0\0\0", 4)
{
    at(0) = (char) a0; at(1) = (char) a1; at(2) = (char) a2; at(3) = (char) a3;
}

InternetAddress::InternetAddress(const std::string& addr)
    : Address("\0\0\0\0", 4)
{
    unsigned short a0, a1, a2, a3;
    if (std::sscanf(addr.c_str(), "%hu.%hu.%hu.%hu",
        &a0, &a1, &a2, &a3) != 4)
        throw Error(ERROR_INVALID_ADDRESS, "invalid Internet address: " + addr);
    at(0) = (char) a0; at(1) = (char) a1; at(2) = (char) a2; at(3) = (char) a3;
}

const std::string InternetAddress::as_text() const
{
    char addr[16];
    std::snprintf(addr, sizeof(addr), "%d.%d.%d.%d", (unsigned char) at(0),
        (unsigned char) at(1), (unsigned char) at(2), (unsigned char) at(3));
    return std::string(addr);
}

const std::string LocalAddress::hostname()
{
    char hostname_buf[64];
    gethostname(hostname_buf, sizeof(hostname_buf));
    return std::string(hostname_buf);
}

/******************************************************************************/

/* class Display */

Display::Display(unsigned short n)
{
    char nstr[16];
    std::snprintf(nstr, sizeof(nstr), "%u", n);
    assign(nstr, std::strlen(nstr));
}

/******************************************************************************/

/* class Cookie */

const std::string Cookie::as_text() const
{
    std::string str;
    for (size_t i = 0; i < length(); i++) {
        char buf[16];
        unsigned char c = (unsigned char) at(i);
        size_t n = std::snprintf(buf, sizeof(buf), "%02hx", (unsigned short) c);
        str.append(buf, n);
    }
    return str;
}

/******************************************************************************/

/* class MagicCookie */

const Token MagicCookie::NAME(MAGIC_COOKIE_NAME, std::strlen(MAGIC_COOKIE_NAME));

MagicCookie::MagicCookie()
    : Cookie(0, 0)
{
#ifdef HAVE_GCRYPT
    char buf[16];

     /* Version check is required to intialize  libgcrypt */
       if (!gcry_check_version (NULL)) {
           fputs ("error initializing libgcrypt\n", stderr);
       }
       else {

         /* Disable secure memory.  */
         gcry_control (GCRYCTL_DISABLE_SECMEM, 0);

         /* Tell Libgcrypt that initialization has completed. */
         gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);

         gcry_md_hash_buffer(GCRY_MD_MD5, buf,
                             gcry_random_bytes(sizeof(buf),
                            GCRY_WEAK_RANDOM), sizeof(buf));
       }

       assign(buf, sizeof(buf));

#else
    throw Error(ERROR_INVALID_COOKIE,
        "recompile with libgcrypt to enable automatic magic cookie generation");
#endif
}

MagicCookie::MagicCookie(const std::string& str)
    : Cookie(0, 0)
{
    if (str.length() == 16) {
        // this is a binary cookie
        assign(str);
    }
    else if (str.length() == 32) {
        // this is an hexadecimal text cookie
        std::string binstr;
        for (size_t i = 0; i < str.length(); i += 2) {
            char buf[] = { str[i], str[i+1], '\0' };
            unsigned short c;
            std::sscanf(buf, "%02hx", &c);
            binstr.push_back((char) c);
        }
        assign(binstr);
    }
    else
        throw Error(ERROR_INVALID_COOKIE, "invalid cookie length: must be 16 (binary) or 32 (hex)");
}

/******************************************************************************/

/* class Xauth */

Xau::Xauth::Xauth()
    : address(LocalAddress(0, 0)), display(Display(0)), cookie(GenericCookie(0,0,0,0))
{
}

Xau::Xauth::Xauth(const Xau::Xauth& auth)
    : address(*auth.address), display(*auth.display), cookie(*auth.cookie)
{
}

Xau::Xauth::Xauth(const Address& address_, const Display& display_, const Cookie& cookie_)
    : address(address_), display(display_), cookie(cookie_)
{
}

Xau::Xauth::Xauth(const ::Xauth* auth)
    : address(LocalAddress(0, 0)), display(Display(0)), cookie(GenericCookie(0,0,0,0))
{
    switch (auth->family)
    {
    case FAMILY_INTERNET:
        address = InternetAddress(auth->address, auth->address_length);
        break;
    case FAMILY_LOCAL:
        address = LocalAddress(auth->address, auth->address_length);
        break;
    default:
        address = GenericAddress((Family) auth->family, auth->address, auth->address_length);
        break;
    }

    display = Display(auth->number, auth->number_length);

    if (std::string(auth->name, auth->name_length) == MagicCookie::NAME)
        cookie = MagicCookie(auth->data, auth->data_length);
    else
        cookie = GenericCookie(auth->name, auth->name_length, auth->data, auth->data_length);
}

const std::string Xau::Xauth::as_text() const
{
    return address->as_text() + ":" + display->as_text() + "\t"
        + cookie->name().as_text() + "\t" + cookie->as_text();
}

::Xauth* Xau::Xauth::as_xauth()
{
    std::memset(&_auth, 0, sizeof(_auth));

    _auth.family            = address->family();
    _auth.address_length    = address->length();
    _auth.address           = const_cast<char*>(address->data());
    _auth.number_length     = display->length();
    _auth.number            = const_cast<char*>(display->data());
    _auth.name_length       = cookie->name().length();
    _auth.name              = const_cast<char*>(cookie->name().data());
    _auth.data_length       = cookie->length();
    _auth.data              = const_cast<char*>(cookie->data());
    return &_auth;
}

/******************************************************************************/

/* class XauthList */

void XauthList::lock_file(const std::string& filename)
{
    if (XauLockAuth(filename.c_str(), 3, 3, 0) != LOCK_SUCCESS)
        throw Error(ERROR_LOCK_ERROR, "cannot lock Xauthority file: " + filename);
}

void XauthList::unlock_file(const std::string& filename)
{
    XauUnlockAuth(filename.c_str());
}

void XauthList::load_from_file(const std::string& filename)
{
    lock_file(filename);

    FILE* file;
    if ((file = fopen(filename.c_str(), "rb")) == NULL)
        throw Error(ERROR_FILE_ERROR, "cannot open " + filename + " for reading");

    ::Xauth* auth;
    while ((auth = XauReadAuth(file)) != NULL) {
        push_back(auth);
        XauDisposeAuth(auth);
    }

    fclose(file);

    unlock_file(filename);
}

void XauthList::write_to_file(const std::string& filename)
{
    lock_file(filename);

    FILE* file;
    if ((file = fopen(filename.c_str(), "wb")) == NULL)
        throw Error(ERROR_FILE_ERROR, "cannot open " + filename + " for writing");

    // set file permissions
    chmod(filename.c_str(), S_IRUSR | S_IWUSR);

    for (iterator it = begin(); it != end(); it++)
        XauWriteAuth(file, *it);

    fclose(file);

    unlock_file(filename);
}

/******************************************************************************/

/* class XauthCond and helpers */

XauthCond::XauthCond(const XauthCond& cond)
{
    for (std::list<XauthCondAtom*>::const_iterator it = cond.conds.begin();
        it != cond.conds.end(); it++)
        conds.push_back((*it)->clone());
}

XauthCond::~XauthCond()
{
    for (std::list<XauthCondAtom*>::iterator it = conds.begin(); it != conds.end(); it++)
        delete *it;
}

bool XauthCond::operator()(const Xau::Xauth& auth)
{
    for (std::list<XauthCondAtom*>::iterator it = conds.begin(); it != conds.end(); it++)
        if (!(*it)->check(auth)) return false;
    return true;
}
