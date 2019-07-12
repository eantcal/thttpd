/*
*  http_config.h
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


/* -------------------------------------------------------------------------- */

///\file http_config.h
///\brief HTTP Server configuration file


/* -------------------------------------------------------------------------- */

#ifndef __HTTP_CONFIG_H__
#define __HTTP_CONFIG_H__

#ifdef WIN32
#define HTTP_SERVER_WROOT "C:/tmp"
#else
#define HTTP_SERVER_WROOT "/tmp"
#endif
#define HTTP_SERVER_INDEX "index.html"
#define HTTP_SERVER_PORT 80
#define HTTP_SERVER_NAME "TinyHttpServer"
#define HTTP_SERVER_MAJ_V 1
#define HTTP_SERVER_MIN_V 0
#define HTTP_SERVER_TX_BUF_SIZE 0x100000
#define HTTP_SERVER_BACKLOG SOMAXCONN

#endif // __HTTP_CONFIG_H__


/* -------------------------------------------------------------------------- */

