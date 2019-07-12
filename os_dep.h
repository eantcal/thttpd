/*
*  os_dep.h
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
