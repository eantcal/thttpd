//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "OsSocketSupport.h"


/* -------------------------------------------------------------------------- */

#ifdef _MSC_VER


/* -------------------------------------------------------------------------- */
// MS Visual C++

/* -------------------------------------------------------------------------- */

#pragma comment(lib, "Ws2_32.lib")


/* -------------------------------------------------------------------------- */

bool OsSocketSupport::initSocketLibrary(std::string& msg)
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

int OsSocketSupport::closeSocketFd(int sd) { 
    return ::closesocket(sd); 
}


/* -------------------------------------------------------------------------- */

#else


/* -------------------------------------------------------------------------- */
// Other C++ platform


/* -------------------------------------------------------------------------- */

bool OsSocketSupport::initSocketLibrary(std::string&) { 
    return true; 
}


/* -------------------------------------------------------------------------- */

int OsSocketSupport::closeSocketFd(int sd) { 
    return ::close(sd); 
}


/* -------------------------------------------------------------------------- */

#endif
