//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

///\file config.h
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

