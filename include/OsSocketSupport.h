//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __OS_SOCKET_SUPPORT_H__
#define __OS_SOCKET_SUPPORT_H__


/* -------------------------------------------------------------------------- */

#include <string>
#include <sys/stat.h>
#include <sys/types.h>


/* -------------------------------------------------------------------------- */

namespace OsSocketSupport {


/* -------------------------------------------------------------------------- */

/**
 * Initializes O/S Libraries. In case of failure
 * the function returns false and msg will contain a
 * spesific error message
 *
 * @param msg specific error string
 * @return true if operation is sucessfully completed,
 * false otherwise
 */
bool initSocketLibrary(std::string& msg);


/* -------------------------------------------------------------------------- */

/**
 * Closes a socket descriptor
 * @return zero on success. On error, -1 is returned
 */
int closeSocketFd(int sd);


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#ifdef WIN32


/* -------------------------------------------------------------------------- */
// MS Visual C++

/* -------------------------------------------------------------------------- */

#include <WinSock2.h>
#define stat _stat
typedef int socklen_t;


/* -------------------------------------------------------------------------- */

#else


/* -------------------------------------------------------------------------- */
// GNU C++

/* -------------------------------------------------------------------------- */

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


/* -------------------------------------------------------------------------- */

typedef int errno_t;


/* -------------------------------------------------------------------------- */

#endif


/* -------------------------------------------------------------------------- */

#endif // __OS_SOCKET_SUPPORT_H__
