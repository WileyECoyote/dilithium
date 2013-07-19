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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author(s):
 * 	David Munger <mungerd@gmail.com>
 */

#ifndef XAUTHXX_H
#define XAUTHXX_H

#include <list>
#include <string>
#include <exception>

#include <X11/X.h>
#include <X11/Xauth.h>

namespace Xau {

typedef enum {
    FAMILY_INTERNET            = FamilyInternet,
    FAMILY_DECNET              = FamilyDECnet,
    FAMILY_CHAOS               = FamilyChaos,
    FAMILY_INTERNET6           = FamilyInternet6,
    FAMILY_SERVER_INTERPRETED  = FamilyServerInterpreted,
    FAMILY_LOCAL               = FamilyLocal,
    FAMILY_WILD                = FamilyWild,
    FAMILY_NETNAME             = FamilyNetname,
    FAMILY_KRB5_PRINCIPAL      = FamilyKrb5Principal,
    FAMILY_LOCAL_HOST          = FamilyLocalHost
} Family;

class Token : public std::string
{
public:
    explicit Token()
        : std::string() {}

    Token(const std::string& str)
        : std::string(str) {}

    Token(const char* s, size_t n)
        : std::string(s, n) {}

    virtual ~Token() {}

    virtual const std::string as_text() const { return *this; }
};

/******************************************************************************/

class Address : public Token
{
public:
    virtual Family family() const = 0;
    virtual Address* clone() const = 0;

protected:
    Address(const Token& token) : Token(token) {}

    Address(const char *data, unsigned short length)
        : Token(data, length) {}
};

class InternetAddress : public Address
{
public:
    InternetAddress(const InternetAddress& addr);
    InternetAddress(int a0, int a1, int a2, int a3);
    InternetAddress(const std::string& addr);

    InternetAddress(const char* data, unsigned short length)
        : Address(data, length) {}

    virtual Address* clone() const { return new InternetAddress(*this); }

    const std::string as_text() const;

    Family family() const
        { return FAMILY_INTERNET; }
};

class LocalAddress : public Address
{
public:
    static const std::string hostname();

public:
    LocalAddress(const LocalAddress& addr)
        : Address(addr) {}

    LocalAddress(const std::string addr = hostname())
        : Address(addr) {}

    LocalAddress(const char* data, unsigned short length)
        : Address(data, length) {}

    virtual Address* clone() const { return new LocalAddress(*this); }

    const std::string as_text() const { return (*this) + "/unix"; }

    Family family() const
        { return FAMILY_LOCAL; }
};

// avoid using this class
class GenericAddress : public Address
{
public:
    GenericAddress(const GenericAddress& a)
        : Address(a.data(), a.length()), _family(a.family()) {}

    GenericAddress(Family family, const char* data, unsigned short length)
        : Address(data, length), _family(family) {}

    virtual Address* clone() const { return new GenericAddress(*this); }

    Family family() const
        { return _family; }

private:
    Family _family;
};

/******************************************************************************/

class Display : public Token
{
public:
    Display(const std::string& s)
        : Token(s) {}

    Display(const char* data, unsigned short length)
        : Token(data, length) {}

    Display(unsigned short n);

    virtual Display* clone() const { return new Display(*this); }
};

/******************************************************************************/

class Cookie : public Token
{
public:
    virtual Cookie* clone() const = 0;
    virtual const Token name() const = 0;
    virtual const std::string as_text() const;

protected:
    Cookie(const std::string& c)
        : Token(c) {}

    Cookie(const char* data, unsigned short length)
        : Token(data, length) {}
};

class MagicCookie : public Cookie
{
public:
    MagicCookie();

    MagicCookie(const MagicCookie& c)
        : Cookie(c) {}

    MagicCookie(const std::string& str);

    MagicCookie(const char* data, unsigned short data_length)
        : Cookie(data, data_length) {}

    virtual Cookie* clone() const { return new MagicCookie(*this); }

    const Token name() const
        { return NAME; }

    static const Token NAME;
};

// avoid using this class
class GenericCookie : public Cookie
{
public:
    GenericCookie(const GenericCookie& c)
        : Cookie(c), _name(c.name()) {}

    GenericCookie(const std::string& name, const std::string& data)
        : Cookie(data), _name(name) {}

    GenericCookie(const char* name, unsigned short name_length,
        const char* data, unsigned short data_length)
        : Cookie(data, data_length), _name(name, name_length) {}

    virtual Cookie* clone() const { return new GenericCookie(*this); }

    const Token name() const
        { return _name; }

protected:
    Token _name;
};

/******************************************************************************/

template <class T>
class Property
{
public:
    Property() { obj = new T(); }
    Property(const T& o) { obj = o.clone(); }
    ~Property() { delete obj; }

    T* operator->() { return obj; }
    const T* operator->() const { return obj; }

    T& operator*() { return *obj; }
    const T& operator*() const { return *obj; }

    operator T&() { return *obj; }
    operator const T&() const { return *obj; }

    Property<T>& operator=(const T& o)
        { delete obj; obj = o.clone(); return *this; }

private:
    T* obj;
};

/******************************************************************************/

class Xauth
{
public:

    Xauth();
    Xauth(const Xauth& auth);
    Xauth(const Address& address, const Display& display, const Cookie& cookie);
    Xauth(const ::Xauth* auth);    // wrapper constructor (copies data)

    const std::string as_text() const;

    ::Xauth* as_xauth();
    operator ::Xauth*() { return as_xauth(); }

    Property<Address>   address;
    Property<Display>   display;
    Property<Cookie>    cookie;

    Xauth& operator<<(const Address& a) { address = a; return *this; }
    Xauth& operator<<(const Display& d) { display = d; return *this; }
    Xauth& operator<<(const Cookie&  c) { cookie  = c; return *this; }

protected:
    ::Xauth _auth;   // used only when exporting
};

/******************************************************************************/

class XauthCondAtom {
public:
    XauthCondAtom() {}
    virtual bool check(const Xau::Xauth& auth) const = 0;
    virtual XauthCondAtom* clone() const = 0;
};

class XauthCondAddress : public XauthCondAtom
{
private:
    const Address& _address;
public:
    XauthCondAddress(const Address& address)
        : _address(address) {}

    virtual XauthCondAtom* clone() const
        { return new XauthCondAddress(_address); }

    bool check(const Xau::Xauth& auth) const
        { return auth.address->family() == _address.family() && (*auth.address) == _address; }
};

class XauthCondDisplay : public XauthCondAtom
{
private:
    const Display& _display;
public:
    XauthCondDisplay(const Display& display)
        : _display(display) {}

    virtual XauthCondAtom* clone() const
        { return new XauthCondDisplay(_display); }

    bool check(const Xau::Xauth& auth) const
        { return (*auth.display) == _display; }
};

class XauthCondCookie : public XauthCondAtom
{
private:
    const Cookie& _cookie;
public:
    XauthCondCookie(const Cookie& cookie)
        : _cookie(cookie) {}

    virtual XauthCondAtom* clone() const
        { return new XauthCondCookie(_cookie); }

    bool check(const Xau::Xauth& auth) const
        { return auth.cookie->name() == _cookie.name() && (*auth.cookie) == _cookie; }
};

class XauthCond
{
private:
    std::list<XauthCondAtom*> conds;
    XauthCond();
public:
    ~XauthCond();
    XauthCond(const XauthCond& cond);

    XauthCond(const Address& address)   { add(address); }
    XauthCond(const Display& display)   { add(display); }
    XauthCond(const Cookie& cookie)     { add(cookie);  }

    void add(const Address& address)
        { conds.push_back(new XauthCondAddress(address)); }
    void add(const Display& display)
        { conds.push_back(new XauthCondDisplay(display)); }
    void add(const Cookie& cookie)
        { conds.push_back(new XauthCondCookie(cookie)); }

    XauthCond operator&&(const Address& address)   { add(address); return *this; }
    XauthCond operator&&(const Display& display)   { add(display); return *this; }
    XauthCond operator&&(const Cookie& cookie)     { add(cookie);  return *this; }

    bool operator()(const Xau::Xauth& auth);
};

#define DECLARE_XAUTHCOND_ANDOP(T1,T2) \
    inline Xau::XauthCond operator&&(const Xau::T1& c1, const Xau::T2& c2) \
        { return Xau::XauthCond(c1) && c2; } \
    inline Xau::XauthCond operator&&(const Xau::T2& c1, const Xau::T1& c2) \
        { return Xau::XauthCond(c1) && c2; }

DECLARE_XAUTHCOND_ANDOP(Address,Display)
DECLARE_XAUTHCOND_ANDOP(Display,Cookie)
DECLARE_XAUTHCOND_ANDOP(Cookie,Address)

/******************************************************************************/

class XauthList : public std::list<Xauth>
{
public:
    static const std::string default_filename() { return XauFileName(); }
    static void lock_file(const std::string& filename = default_filename());
    static void unlock_file(const std::string& filename = default_filename());

    XauthList() {}

    void load_from_file(const std::string& filename = default_filename());
    void write_to_file(const std::string& filename = default_filename());

    void remove(const XauthCond& cond) { remove_if(cond); }
};

}

#endif /* XAUTHXX_H */
