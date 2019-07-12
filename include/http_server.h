//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

/// \file http_server.h
/// \brief Declaration of HTTP classes


/* -------------------------------------------------------------------------- */

#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__


/* -------------------------------------------------------------------------- */

#include "http_config.h"
#include "socket_utils.h"

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>


/* -------------------------------------------------------------------------- */

/**
 * Encapsulates HTTP style request, consisting of a request line,
 * some headers, and a content body
 */
class HttpRequest {
public:
    using Handle = std::shared_ptr<HttpRequest>;

    enum class Method { GET, HEAD, POST, UNKNOWN };
    enum class Version { HTTP_1_0, HTTP_1_1, UNKNOWN };

    HttpRequest() = default;
    HttpRequest(const HttpRequest&) = default;


    /**
     * Returns the request headers
     */
    const std::list<std::string>& get_header() const { 
       return _header; 
    }

    /**
     * Returns the method of the command line (GET, HEAD, ...)
     */
    Method getMethod() const { 
       return _method; 
    }

    /**
     * Returns the HTTP version (HTTP/1.0, HTTP/1.1, ...)
     */
    Version getVersion() const { 
       return _version; 
    }

    /**
     * Returns the command line URI
     */
    const std::string& getUri() const { 
       return _uri; 
    }

    /**
     * Set HTTP method field decoding the string method
     *
     * @param method The input string to parse
     */
    void parseMethod(const std::string& method);

    /**
     * Set HTTP URI field decoding the string uri
     *
     * @param uri The input string to parse
     */
    void parseUri(const std::string& uri) {
        _uri = uri == "/" ? HTTP_SERVER_INDEX : uri;
    }

    /**
     * Set HTTP version field decoding the string ver
     *
     * @param ver The input string to parse
     */
    void parseVersion(const std::string& ver);

    /**
     * Add a new header to request
     *
     * @param new_header The header content to add to headers fields
     */
    void addHeader(const std::string& new_header) {
        _header.push_back(new_header);
    }

    /**
     * Prints the request out to the os stream
     *
     * @param os The output stream
     * @param id A string used to identify the request
     * @return the os output stream
     */
    std::ostream& dump(std::ostream& os, const std::string& id = "");

private:
    std::list<std::string> _header;
    Method _method = Method::UNKNOWN;
    Version _version = Version::UNKNOWN;
    std::string _uri;
};


/* -------------------------------------------------------------------------- */
// HttpResponse


/* -------------------------------------------------------------------------- */

/**
 * Encapsulates HTTP style response, consisting of a status line,
 * some headers, and a content body
 */
class HttpResponse {
public:
    HttpResponse() = delete;
    HttpResponse(const HttpResponse&) = default;
    HttpResponse& operator=(const HttpResponse&) = default;

    /**
     * Construct a response to a request
     * @param request an http request
     * @param webRootPath local working directory of the web server
     */
    HttpResponse(const HttpRequest& request, const std::string& webRootPath);

    /**
     * Returns the content of response status line and response headers
     */
    operator const std::string&() const { 
       return _response; 
    }

    /**
     * Returns the content of local resource related to the URI requested
     */
    const std::string& getLocalUriPath() const {
        return _localUriPath;
    }

    /**
     * Prints the response out to os stream
     *
     * @param os The output stream
     * @param id A string used to identify the response
     * @return the os output stream
     */
    std::ostream& dump(std::ostream& os, const std::string& id = "");

private:
    static std::map<std::string, std::string> _mimeTbl;

    std::string _response;
    std::string _localUriPath;

    // Format an error response
    static void formatError(
        std::string& output, 
        int code, 
        const std::string& msg);

    // Format an positive response
    static void formatPositiveResponse(
        std::string& response, 
        std::string& fileTime,
        std::string& fileExt,
        size_t& contentLen);
};


/* -------------------------------------------------------------------------- */

/**
 * This class represents an HTTP connection between a client and a server
 */
class HttpSocket {
private:
    TcpSocket::Handle _socketHandle;
    bool _connUp = true;
    HttpRequest::Handle recv();

public:
    HttpSocket() = default;
    HttpSocket(const HttpSocket&) = default;

    /**
     * Construct the HTTP connection starting from TCP connected-socket handle
     */
    HttpSocket(TcpSocket::Handle handle)
        : _socketHandle(handle)
    {
    }

    /**
     * Assigns a new TCP connected socket handle to this HTTP socket
     */
    HttpSocket& operator=(TcpSocket::Handle handle);

    /**
     * Returns TCP socket handle
     */
    operator TcpSocket::Handle() const { 
       return _socketHandle; 
    }

    /**
     * Receives an HTTP request from remote peer
     * @param the handle of http request object
     */
    HttpSocket& operator>>(HttpRequest::Handle& handle) {
        handle = recv();
        return *this;
    }

    /**
     * Returns false if last recv/send operation detected
     * that connection was down; true otherwise
     */
    explicit operator bool() const { 
       return _connUp; 
    }

    /**
     * Send a response to remote peer
     *
     * @param response The HTTP response
     */
    HttpSocket& operator<<(const HttpResponse& response);


    /**
     * Send a response to remote peer
     *
     * @param response The HTTP response
     */
    int sendFile(const std::string& fileName) {
        return _socketHandle->sendFile(fileName);
    }
};


/* -------------------------------------------------------------------------- */

/**
 * The top-level class of the HTTP server
 */
class HttpServer {
public:
    using TranspPort = TcpListener::TranspPort;
    enum { DEFAULT_PORT = HTTP_SERVER_PORT };

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
     * Sets a loggerOStream enabling the verbose mode
     *
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
     * Gets HttpServer object instance reference
     * This class is a singleton. First time this
     * function is called, the HttpServer object
     * is initialized
     *
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
    const TranspPort get_local_port() const { 
       return _serverPort; 
    }

    /**
     * Binds the HTTP server to a local TCP port
     *
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
     *
     * @return false if operation failed, otherwise the function
     * doesn't return ever
     */
    bool run();


protected:
    /**
     * This function wait for receive activity or until time-out expires
     *
     * @param timeout the duration of time-out
     * @return true if receive activity is detected, false otherwise
     * Using @see get_last_errno() is possible to distinguish between a timeout
     * and error
     */
    bool waitForData(const TransportSocket::TimeoutInterval& timeout);

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
