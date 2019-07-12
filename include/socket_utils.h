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
#include "osSocketSpecific.h"


/* -------------------------------------------------------------------------- */

/**
 * Provides socket functionality
 */
class TransportSocket {

public:
    using TranspPort = uint16_t;
    using SocketFd = int;
    enum class WaitingEvent { RECV_ERROR, TIMEOUT, RECV_DATA };
    using TimeoutInterval = std::chrono::system_clock::duration;

protected:
    /**
     * Construct a basic_socket on an existing native socket
     * descriptor (@see ::socket())
     *
     * @param sd native socket descriptor
     */
    TransportSocket(const SocketFd& sd)
        : _socket(sd)
    {
    }

public:
    TransportSocket(const TransportSocket&) = delete;
    TransportSocket& operator=(const TransportSocket&) = delete;
    virtual ~TransportSocket();


    /**
     * Returns true if socket is valid, false otherwise.
     */
    bool isValid() const {
        return _socket > 0;
    }


    /**
     * Returns true if socket is valid, false otherwise.
     */
    operator bool() const {
        return isValid();
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
     * @return WaitingEvent::RECV_DATA if data is available for reading,
     *         WaitingEvent::TIMEOUT if the time limit expired or
     *         WaitingEvent::RECV_ERROR if an error occurred
     */
    WaitingEvent waitForRecvEvent(const TimeoutInterval& timeout);


    /**
     * Returns the socket descriptor for this socket
     */
    SocketFd getSocketFd() const {
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
        return ::send(getSocketFd(), buf, len, flags);
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
        return ::recv(getSocketFd(), buf, len, flags);
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
    int sendFile(const std::string& filepath);

private:
    SocketFd _socket = 0;
    enum { TX_BUFFER_SIZE = HTTP_SERVER_TX_BUF_SIZE };
};


/* -------------------------------------------------------------------------- */

/**
 * This class represents a TCP connection between a client and a server
 */
class TcpSocket : public TransportSocket {
    friend class TcpListener;

public:
    enum class shutdown_mode_t : int {
        DISABLE_RECV,
        DISABLE_SEND,
        DISABLE_SEND_RECV
    };

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;
    ~TcpSocket() = default;

    using Handle = std::shared_ptr<TcpSocket>;

    /**
     * Returns the local peer's ipv4 address.
     */
    std::string get_local_ip() const {
        return _local_ip;
    }

    /**
     * Returns the local peer's tcp port number
     */
    TranspPort get_local_port() const {
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
    TranspPort get_remote_port() const {
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
        return ::shutdown(getSocketFd(), static_cast<int>(how));
    }

    /**
     * Sends text on this socket
     */
    TcpSocket& operator<<(const std::string& text);

    TcpSocket() = delete;

private:
    std::string _local_ip;
    TranspPort _local_port = 0;
    std::string _remote_ip;
    TranspPort _remote_port = 0;

    TcpSocket(const SocketFd& sd, const sockaddr* local_sa,
        const sockaddr* remote_sa);
};


/* -------------------------------------------------------------------------- */

/**
 * Listens for connections from TCP network clients.
 */
class TcpListener : public TransportSocket {
public:
    using TranspPort = TcpSocket::TranspPort;
    using Handle = std::unique_ptr<TcpListener>;

    enum class State { INVALID, VALID };
    enum class BindingState { UNBOUND, BOUND };


    /**
     * Returns the current state of this connection
     * @return State::VALID if connection is valid,
     *         State::INVALID otherwise
     */
    State getState() const {
        return _state;
    }

    /**
     * Returns the current state of this connection
     * @return true if connection is valid,
     *         false otherwise
     */
    operator bool() const {
        return getState() != State::INVALID;
    }

    /**
     * Returns the current binding state of this connection
     * @return BindingState::BOUND if connection is bound to
     *         local address and port,
     *         BindingState::UNBOUND otherwise
     */
    BindingState getBindingState() const {
        return _bind_st;
    }

    /**
     * Returns a handle to a new listener object
     * @return the handle to a new listenr object instance
     */
    static Handle create() {
        return Handle(new TcpListener());
    }

    /**
     * Associates a local IPv4 address and TCP port with this
     * connection.
     *
     * @param ip The IPv4 address of local interface to bind to
     * @param port The port to bind to
     * @return false if operation fails, true otherwise
     */
    bool bind(const std::string& ip, const TranspPort& port);

    /**
     * Associates a local TCP port with this connection.
     *
     * @param port The port to bind to
     * @return false if operation fails, true otherwise
     */
    bool bind(const TranspPort& port) {
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
        return ::listen(getSocketFd(), backlog) == 0;
    }

    /**
     * Extracts the first connection on the queue of pending connections,
     * and creates a new tcp connection handle
     *
     * @return an handle to a new tcp connection
     */
    TcpSocket::Handle accept();

private:
    std::atomic<State> _state;
    std::atomic<BindingState> _bind_st;

    TranspPort _port = 0;
    sockaddr_in _local_ip_port_sa_in;

    TcpListener();
};


/* -------------------------------------------------------------------------- */

#endif // __SOCKET_UTILS_H__

