//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__


/* -------------------------------------------------------------------------- */

#include "HttpSocket.h"
#include "TcpListener.h"

#include <iostream>
#include <string>


/* -------------------------------------------------------------------------- */

/**
 * The top-level class of the HTTP server.
 */
class HttpServer {
public:
    using TranspPort = TcpListener::TranspPort;
    enum { DEFAULT_PORT = 80 };

private:
    std::ostream* _loggerOStreamPtr = &std::clog;
    static HttpServer* _instance;
    TranspPort _serverPort = DEFAULT_PORT;
    TcpListener::Handle _tcpServer;
    std::string _webRootPath = "/tmp";
    bool _verboseModeOn = true;

    HttpServer() = default;

public:
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;


    /**
     * Sets a loggerOStream enabling the verbose mode.
     * @param pointer to ostream used for logging
     */
    void setupLogger(std::ostream* loggerOStream = nullptr) {
        if (loggerOStream) {
            _loggerOStreamPtr = loggerOStream;
            _verboseModeOn = true;
        } else {
            _verboseModeOn = false;
        }
    }

    /**
     * Gets HttpServer object instance reference.
     * This class is a singleton. First time this function is called, 
     * the HttpServer object is initialized.
     * @return the HttpServer reference
     */
    static auto getInstance() -> HttpServer&;

    /**
     * Gets current server working directory
     */
    const std::string& getWebRootPath() const { 
       return _webRootPath; 
    }

    /**
     * Sets the server working directory
     */
    void setupWebRootPath(const std::string& webRootPath) {
        _webRootPath = webRootPath;
    }

    /**
     * Gets port where server is listening
     * @return the port number
     */
    const TranspPort getLocalPort() const { 
       return _serverPort; 
    }

    /**
     * Binds the HTTP server to a local TCP port
     * @param port The listening port
     * @return true if operation is successfully completed, false otherwise
     */
    bool bind(TranspPort port = DEFAULT_PORT);

    /**
     * Set the server in listening mode
     * @param maxConnections back log list length
     * @return true if operation is successfully completed, false otherwise
     */
    bool listen(int maxConnections);

    /**
     * Run the server. This function is blocking for the caller.
     * @return false if operation failed, otherwise the function
     * doesn't return ever
     */
    bool run();


protected:
    /**
     * Accepts a new connection from remote client.
     * This function blocks until connection is established or
     * an error occurs.
     * @return a handle to tcp socket
     */
    TcpSocket::Handle accept() { 
       return _tcpServer->accept(); 
    }
};


/* -------------------------------------------------------------------------- */

#endif // __HTTP_SERVER_H__
