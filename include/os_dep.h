//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

///\file os_dep.h
///\brief Platform dependent code


/* -------------------------------------------------------------------------- */

#ifndef __OS_DEP_H__
#define __OS_DEP_H__


/* -------------------------------------------------------------------------- */

#include <string>
#include <sys/stat.h>
#include <sys/types.h>


/* -------------------------------------------------------------------------- */

namespace os_dep {


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
bool init_lib(std::string& msg);


/* -------------------------------------------------------------------------- */

/**
 * Closes a socket descriptor
 * @return zero on success. On error, -1 is returned
 */
int close_socket(int sd);


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

#endif // __OS_DEP_H__
