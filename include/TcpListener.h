//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __TCP_LISTENER_H__
#define __TCP_LISTENER_H__


/* -------------------------------------------------------------------------- */

#include "TcpSocket.h"
#include "OsSocketSupport.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <memory>


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

#endif // __TCP_LISTENER_H__

