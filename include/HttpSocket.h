//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

/// \file HttpSocket.h
/// \brief Declaration of HTTP classes


/* -------------------------------------------------------------------------- */

#ifndef __HTTP_SOCKET_H__
#define __HTTP_SOCKET_H__


/* -------------------------------------------------------------------------- */

#include "TcpSocket.h"

#include "HttpRequest.h"
#include "HttpResponse.h"

#include "config.h"

#include <string>


/* -------------------------------------------------------------------------- */
// HttpResponse


/* -------------------------------------------------------------------------- */

/**
 * This class represents an HTTP connection between a client and a server.
 */
class HttpSocket {
private:
    TcpSocket::Handle _socketHandle;
    bool _connUp = true;
    HttpRequest::Handle recv();
    int _connectionTimeOut = HTTP_CONNECTION_TIMEOUT; // secs

public:
    HttpSocket(int connectionTimeout = HTTP_CONNECTION_TIMEOUT) noexcept :
        _connectionTimeOut(connectionTimeout) 
    {}

    HttpSocket(const HttpSocket&) = default;

    /**
     * Construct the HTTP connection starting from TCP connected-socket handle.
     */
    HttpSocket(TcpSocket::Handle handle)
        : _socketHandle(handle)
    {
    }

    /**
     * Assigns a new TCP connected socket handle to this HTTP socket.
     */
    HttpSocket& operator=(TcpSocket::Handle handle);

    /**
     * Returns TCP socket handle
     */
    operator TcpSocket::Handle() const { 
       return _socketHandle; 
    }

    /**
     * Receives an HTTP request from remote peer.
     * @param the handle of http request object
     */
    HttpSocket& operator>>(HttpRequest::Handle& handle) {
        handle = recv();
        return *this;
    }

    /**
     * Returns false if last recv/send operation detected
     * that connection was down; true otherwise.
     */
    explicit operator bool() const { 
       return _connUp; 
    }

    /**
     * Send a response to remote peer.
     * @param response The HTTP response
     */
    HttpSocket& operator<<(const HttpResponse& response);


    /**
     * Send a response to remote peer.
     * @param response The HTTP response
     */
    int sendFile(const std::string& fileName) {
        return _socketHandle->sendFile(fileName);
    }

    /*
     * Return connection timeout interval in seconds
     */
    const int& getConnectionTimeout() const noexcept {
        return _connectionTimeOut;
    }
};


/* -------------------------------------------------------------------------- */

#endif // __HTTP_SOCKET_H__
