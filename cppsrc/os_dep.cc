//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

///\file os_dep.cc
///\brief Platform dependent code


/* -------------------------------------------------------------------------- */

#include "os_dep.h"


/* -------------------------------------------------------------------------- */

#ifdef _MSC_VER


/* -------------------------------------------------------------------------- */
// MS Visual C++

/* -------------------------------------------------------------------------- */

#pragma comment(lib, "Ws2_32.lib")


/* -------------------------------------------------------------------------- */

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


/* -------------------------------------------------------------------------- */

int os_dep::close_socket(int sd) { 
    return ::closesocket(sd); 
}


/* -------------------------------------------------------------------------- */

#else


/* -------------------------------------------------------------------------- */
// Other C++ compiler


/* -------------------------------------------------------------------------- */

bool os_dep::init_lib(std::string&) { 
    return true; 
}


/* -------------------------------------------------------------------------- */

int os_dep::close_socket(int sd) { 
    return ::close(sd); 
}


/* -------------------------------------------------------------------------- */

#endif
