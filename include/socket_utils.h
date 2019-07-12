//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

/// \file socket_utils.h
/// \brief Declaration of socket classes containing some
/// useful stuff cross-platform for manipulating socket


/* -------------------------------------------------------------------------- */

#ifndef __SOCKET_UTILS_H__
#define __SOCKET_UTILS_H__


/* -------------------------------------------------------------------------- */

#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <memory>

#include "http_config.h"
#include "os_dep.h"


/* -------------------------------------------------------------------------- */

/**
 * Provides socket functionality
 */
class basic_socket_t {

public:
    using port_t = uint16_t;
    using socket_desc_t = int;
    enum class wait_ev_t { RECV_ERROR, TIMEOUT, RECV_DATA };
    using timeout_t = std::chrono::system_clock::duration;

protected:
    /**
     * Construct a basic_socket on an existing native socket
     * descriptor (@see ::socket())
     *
     * @param sd native socket descriptor
     */
    basic_socket_t(const socket_desc_t& sd)
        : _socket(sd)
    {
    }

public:
    basic_socket_t(const basic_socket_t&) = delete;
    basic_socket_t& operator=(const basic_socket_t&) = delete;
    virtual ~basic_socket_t();


    /**
     * Returns true if socket is valid, false otherwise.
     */
    bool is_valid() const {
        return _socket > 0;
    }



    /**
     * Returns true if socket is valid, false otherwise.
     */
    operator bool() const {
        return is_valid();
    }


    /**
     * Determines the readability status of this socket
     *
     * If you are in a listening state, readability means
     * that a call to accept() will succeed without blocking.
     * If you have already accepted the connection,
     * readability means that data is available for reading.
     * In these cases, all receive operations will succeed
     * without blocking.
     * Readability can also indicate whether the remote Socket
     * has shut down the connection; in that case a call
     * to recv() will return immediately,
     * with zero bytes returned.
     *
     * @param timeout The time-out value.
     * @return wait_ev_t::RECV_DATA if data is available for reading,
     *         wait_ev_t::TIMEOUT if the time limit expired or
     *         wait_ev_t::RECV_ERROR if an error occurred
     */
    wait_ev_t wait_for_recv_event(const timeout_t& timeout);


    /**
     * Returns the socket descriptor for this socket
     */
    socket_desc_t get_sd() const {
        return _socket;
    }


    /**
     * Sends data on a connected socket
     *
     * @param buf   A pointer to a buffer containing the data to be
     *              transmitted.
     * @param len   The length, in bytes, of the data in buffer pointed
     *              to by the buf parameter.
     * @param flags A set of flags that specify the way in which the
     *              call is made.
     * @return      If no error occurs, send() returns the total number
     *              of bytes sent, which can be less than the number
     *              requested to be sent in the len parameter.
     *              Otherwise, -1 is returned, and a specific error code
     *              can be retrieved by calling errno
     *
     */
    int send(const char* buf, int len, int flags = 0) {
        return ::send(get_sd(), buf, len, flags);
    }


    /**
     * Receives data from a connected socket
     *
     * @param buf   A pointer to the buffer to receive the incoming data.
     * @param len   The length, in bytes, of the data in buffer pointed
     *              to by the buf parameter.
     * @param flags A set of flags that specify the way in which the
     *              call is made.
     * @return      If no error occurs, recv returns the number of bytes
     *              received and the buffer pointed to by the buf parameter
     *              will contain this data received.
     *              If the connection has been gracefully closed,
     *              the return value is zero.
     *              Otherwise, -1 is returned, and a specific error code
     *             can be retrieved by calling errno
     */
    int recv(char* buf, int len, int flags = 0) {
        return ::recv(get_sd(), buf, len, flags);
    }


    /**
     * Sends text on a connected socket
     *
     * @param text  String containing the data to be transmitted.
     * @return      If no error occurs, send() returns the total number
     *              of bytes sent, which can be less than the number
     *              requested to be sent in the len parameter.
     *              Otherwise, -1 is returned, and a specific error code
     *              can be retrieved by errno
     */
    int send(const std::string& text) {
        return send(text.c_str(), int(text.size()));
    }


    /**
     * Sends a file on a connected socket
     *
     * @param filepath String containing the path of existing file
     * @return      If no error occurs, send() returns the total number
     *              of bytes sent, which can be less than the number
     *              requested to be sent in the len parameter.
     *              Otherwise, -1 is returned, and a specific error code
     *              can be retrieved by errno
     */
    int send_file(const std::string& filepath);

private:
    socket_desc_t _socket = 0;
    enum { TX_BUFFER_SIZE = HTTP_SERVER_TX_BUF_SIZE };
};


/* -------------------------------------------------------------------------- */

/**
 * This class represents a TCP connection between a client and a server
 */
class tcp_socket_t : public basic_socket_t {
    friend class tcp_listener_t;

public:
    enum class shutdown_mode_t : int {
        DISABLE_RECV,
        DISABLE_SEND,
        DISABLE_SEND_RECV
    };

    tcp_socket_t(const tcp_socket_t&) = delete;
    tcp_socket_t& operator=(const tcp_socket_t&) = delete;
    ~tcp_socket_t() = default;

    using handle_t = std::shared_ptr<tcp_socket_t>;


    /**
     * Returns the local peer's ipv4 address.
     */
    std::string get_local_ip() const {
        return _local_ip;
    }


    /**
     * Returns the local peer's tcp port number
     */
    port_t get_local_port() const {
        return _local_port;
    }


    /**
     * Returns the remote peer's ipv4 address.
     */
    std::string get_remote_ip() const {
        return _remote_ip;
    }


    /**
     * Returns the remote peer's tcp port number
     */
    port_t get_remote_port() const {
        return _remote_port;
    }


    /**
     * Disables sends or receives on this socket
     * @param how can be DISABLE_RECV to disable receive operation,
     * DISABLE_SEND to disable send operations, DISABLE_SEND_RECV
     * for both send and receive operation
     * @return If no error occurs, shutdown returns zero.
     * Otherwise, -1 is returned
     */
    int shutdown(shutdown_mode_t how = shutdown_mode_t::DISABLE_SEND_RECV) {
        return ::shutdown(get_sd(), static_cast<int>(how));
    }


    /**
     * Sends text on this socket
     */
    tcp_socket_t& operator<<(const std::string& text);

    tcp_socket_t() = delete;

private:
    std::string _local_ip;
    port_t _local_port = 0;
    std::string _remote_ip;
    port_t _remote_port = 0;

    tcp_socket_t(const socket_desc_t& sd, const sockaddr* local_sa,
        const sockaddr* remote_sa);
};


/* -------------------------------------------------------------------------- */

/**
 * Listens for connections from TCP network clients.
 */
class tcp_listener_t : public basic_socket_t {
public:
    using port_t = tcp_socket_t::port_t;
    using handle_t = std::unique_ptr<tcp_listener_t>;

    enum class state_t { INVALID, VALID };
    enum class bind_st_t { UNBOUND, BOUND };


    /**
     * Returns the current state of this connection
     * @return state_t::VALID if connection is valid,
     *         state_t::INVALID otherwise
     */
    state_t get_state() const {
        return _state;
    }


    /**
     * Returns the current state of this connection
     * @return true if connection is valid,
     *         false otherwise
     */
    operator bool() const {
        return get_state() != state_t::INVALID;
    }


    /**
     * Returns the current binding state of this connection
     * @return bind_st_t::BOUND if connection is bound to
     *         local address and port,
     *         bind_st_t::UNBOUND otherwise
     */
    bind_st_t get_bind_state() const {
        return _bind_st;
    }


    /**
     * Returns a handle to a new listener object
     * @return the handle to a new listenr object instance
     */
    static handle_t create() {
        return handle_t(new tcp_listener_t());
    }


    /**
     * Associates a local IPv4 address and TCP port with this
     * connection.
     *
     * @param ip The IPv4 address of local interface to bind to
     * @param port The port to bind to
     * @return false if operation fails, true otherwise
     */
    bool bind(const std::string& ip, const port_t& port);


    /**
     * Associates a local TCP port with this connection.
     *
     * @param port The port to bind to
     * @return false if operation fails, true otherwise
     */
    bool bind(const port_t& port) {
        return bind("", port);
    }


    /**
     * Enables the listening mode, to listen for incoming
     * connection attempts.
     *
     * @param backlog The maximum length of the pending connections queue.
     * @return true if operation successfully completed, false otherwise
     */
    bool listen(int backlog = SOMAXCONN) {
        return ::listen(get_sd(), backlog) == 0;
    }


    /**
     * Extracts the first connection on the queue of pending connections,
     * and creates a new tcp connection handle
     *
     * @return an handle to a new tcp connection
     */
    tcp_socket_t::handle_t accept();


private:
    std::atomic<state_t> _state;
    std::atomic<bind_st_t> _bind_st;

    port_t _port = 0;
    sockaddr_in _local_ip_port_sa_in;

    tcp_listener_t();
};


/* -------------------------------------------------------------------------- */

#endif // __SOCKET_UTILS_H__

