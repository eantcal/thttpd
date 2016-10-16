/*
*  os_dep.cc
*
*  This file is part of TinyHttpServer
*
*  TinyHttpServer is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  TinyHttpServer is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with TinyHttpServer; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
*
*  Author:	Antonino Calderone, <acaldmail@gmail.com>
*
*/


// -----------------------------------------------------------------------------


///\file os_dep.cc
///\brief Platform dependent code


// -----------------------------------------------------------------------------


#include "os_dep.h"


// ----------------------------------------------------------------------------


#ifdef _MSC_VER


// ----------------------------------------------------------------------------
// MS Visual C++
// ----------------------------------------------------------------------------


#pragma comment(lib, "Ws2_32.lib")


bool os_dep::init_lib(std::string& msg)
{
    // Socket library initialization
    WORD wVersionRequested = WINSOCK_VERSION;
    WSADATA wsaData = { 0 };

    bool ret = 0 == WSAStartup(wVersionRequested, &wsaData);

    if (!ret)
        msg = "WSAStartup failed";

    return ret;
}


// ----------------------------------------------------------------------------


int os_dep::close_socket(int sd) { return ::closesocket(sd); }


// ----------------------------------------------------------------------------


#else


// ----------------------------------------------------------------------------
// GNU C++
// ----------------------------------------------------------------------------


bool os_dep::init_lib(std::string& msg) { return true; }


// ----------------------------------------------------------------------------


int os_dep::close_socket(int sd) { return ::close(sd); }


// ----------------------------------------------------------------------------


#endif


// ----------------------------------------------------------------------------
