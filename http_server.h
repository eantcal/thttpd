/*
*  http_server.h
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
#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__


// -----------------------------------------------------------------------------


/// \file http_server.h 
/// \brief Declaration of HTTP classes


// -----------------------------------------------------------------------------


#include "http_config.h"
#include "socket_utils.h"

#include <list>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <memory>

// -----------------------------------------------------------------------------


/**
 * Encapsulates HTTP style request, consisting of a request line, 
 * some headers, and a content body
 */
class http_request_t
{
public:
    using handle_t = std::shared_ptr< http_request_t >;

    enum class method_t { GET, HEAD, POST, UNKNOWN };
    enum class version_t { HTTP_1_0, HTTP_1_1, UNKNOWN };

    http_request_t() = default;
    http_request_t(const http_request_t&) = default;


    /**
     * Returns the request headers
     */
    inline const std::list<std::string> & get_header() const
    {
        return _header;
    }


    /**
     * Returns the method of the command line (GET, HEAD, ...)
     */
    inline method_t get_method() const
    {
        return _method;
    }


    /**
     * Returns the HTTP version (HTTP/1.0, HTTP/1.1, ...)
     */
    inline version_t get_version() const
    {
        return _version;
    }


    /**
     * Returns the command line URI
     */
    inline const std::string& get_uri() const
    {
        return _uri;
    }


    /**
     * Set HTTP method field decoding the string method 
     *
     * @param method The input string to parse
     */
    void parse_method(const std::string& method);


    /**
     * Set HTTP URI field decoding the string uri
     *
     * @param uri The input string to parse
     */
    inline void parse_uri(const std::string& uri)
    {
        _uri = uri == "/" ? HTTP_SERVER_INDEX : uri;
    }


    /**
     * Set HTTP version field decoding the string ver
     *
     * @param ver The input string to parse
     */
    void parse_version(const std::string& ver);


    /**
     * Add a new header to request
     *
     * @param new_header The header content to add to headers fields
     */
    inline void add_header(const std::string& new_header)
    {
        _header.push_back(new_header);
    }


    /**
     * Prints the request out to the os stream
     *
     * @param os The output stream
     * @param id A string used to identify the request
     * @return the os output stream
     */
    std::ostream & dump(std::ostream & os, const std::string& id = "");


private:
    std::list<std::string> _header;
    method_t _method = method_t::UNKNOWN;
    version_t _version = version_t::UNKNOWN;
    std::string _uri;
};




// -----------------------------------------------------------------------------
// http_response_t
// -----------------------------------------------------------------------------

/**
 * Encapsulates HTTP style response, consisting of a status line,
 * some headers, and a content body
 */
class http_response_t
{
public:
    http_response_t() = delete;
    http_response_t(const http_response_t&) = default;
    http_response_t& operator=(const http_response_t&) = default;

    /**
     * Construct a response to a request
     * @param request an http request
     * @param web_root local working directory of the web server 
     */
    http_response_t(
        const http_request_t & request,
        const std::string& web_root);

    /**
     * Returns the content of response status line and response headers 
     */
    inline operator const std::string& () const
    {
        return _response;
    }


    /**
     * Returns the content of local resource related to the URI requested
     */
    inline const std::string& get_local_uri_path() const
    {
         return _local_uri_path;
    }

 
    /**
     * Prints the response out to os stream
     *
     * @param os The output stream
     * @param id A string used to identify the response
     * @return the os output stream
     */
    std::ostream & dump(std::ostream & os, const std::string& id = "");

private:
    static std::map < std::string, std::string > _mime_tbl;
    
    std::string _response;
    std::string _local_uri_path;

    // Format an error response 
    static void format_error(
        std::string & output, int code, const std::string & msg);
};




/**
 * This class represents an HTTP connection between a client and a server
 */
class http_socket_t
{
private:
    tcp_socket_t::handle_t _socket_handle;
    bool _conn_up = true;
    http_request_t::handle_t recv();

public:
    http_socket_t() = default;
    http_socket_t(const http_socket_t&) = default;

    
    /**
     * Construct the HTTP connection starting from TCP connected-socket handle
     */
    inline http_socket_t(tcp_socket_t::handle_t handle) : 
        _socket_handle(handle)
    {}

    
    /**
     * Assigns a new TCP connected socket handle to this HTTP socket
     */
    http_socket_t & operator=(tcp_socket_t::handle_t handle);


    /**
     * Returns TCP socket handle
     */
    inline operator tcp_socket_t::handle_t() const 
    {
        return _socket_handle;
    }


    /**
     * Receives an HTTP request from remote peer
     * @param the handle of http request object
     */
    inline http_socket_t & operator>>(http_request_t::handle_t & handle)
    {
        handle = recv();
        return *this;
    }


    /**
     * Returns false if last recv/send operation detected 
     * that connection was down; true otherwise
     */
    inline explicit operator bool() const
    {
        return _conn_up;
    }


    /**
     * Send a response to remote peer
     *
     * @param response The HTTP response
     */
    http_socket_t & operator <<(const http_response_t & response);


    /**
     * Send a response to remote peer
     *
     * @param response The HTTP response
     */
    inline int send_file (const std::string & filename)
    {
        return  _socket_handle->send_file(filename);
    }
};


// -----------------------------------------------------------------------------


/**
 * The top-level class of the HTTP server
 */
class http_server_t
{
public:
    using port_t = tcp_listener_t::port_t;
    enum { DEFAULT_PORT = HTTP_SERVER_PORT };

private:
    std::ostream * _logger_ptr = &std::clog;
    static http_server_t * _instance;
    port_t _server_port = DEFAULT_PORT;
    tcp_listener_t::handle_t _tcp_server;
    std::string _web_root = "/tmp";
    bool _verbose_mode = true;

    http_server_t() = default;

public:
    http_server_t(const http_server_t&) = delete;
    http_server_t& operator = (const http_server_t&) = delete;


    /**
     * Sets a logger enabling the verbose mode
     *
     * @param pointer to ostream used for logging
     */
    inline void set_logger(std::ostream * logger_ptr = nullptr)
    {
        if (logger_ptr)
        {
            _logger_ptr = logger_ptr;
            _verbose_mode = true;
        }
        else
        {
            _verbose_mode = false;
        }
    }


    /**
     * Gets http_server_t object instance reference
     * This class is a singleton. First time this
     * function is called, the http_server_t object
     * is initialized
     *
     * @return the http_server_t reference
     */
    static auto get_instance()->http_server_t&;


    /**
     * Gets current server working directory
     */
    inline const std::string& get_web_root() const
    {
        return _web_root;
    }


    /**
     * Sets the server working directory
     */
    inline void set_web_root(const std::string& web_root)
    {
        _web_root = web_root;
    }


    /**
     * Gets the server listening port
     * @return the port number
     */
    inline const port_t get_local_port() const
    {
        return _server_port;
    }


    /**
     * Binds the HTTP server to a local TCP port
     *
     * @param port The listening port
     * @return true if operation is successfully completed, false otherwise
     */
    bool bind(port_t port = DEFAULT_PORT);


    /**
     * Set the server in listening mode
     * @param max_connections back log list length
     * @return true if operation is successfully completed, false otherwise
     */
    bool listen(int max_connections);


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
    bool wait_for_conn(const basic_socket_t::timeout_t & timeout);

    /**
     * Accepts a new connection from remote client. 
     * This function blocks until connection is established or
     * an error occurs.
     * @return a handle to tcp socket
     */
    inline tcp_socket_t::handle_t accept()
    {
        return _tcp_server->accept();
    }

    /**
     * Returns the last error code
     * @return the error code
     */
    inline errno_t get_last_errno() const
    {
        return _tcp_server->get_last_errno();
    }
};


// -----------------------------------------------------------------------------


#endif // __HTTP_SERVER_H__