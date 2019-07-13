//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__


/* -------------------------------------------------------------------------- */

#include "TransportSocket.h"

#include <memory>
#include <string>


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
    const std::string& getLocalIpAddress() const {
        return _localIpAddress;
    }

    /**
     * Returns the local peer's tcp port number
     */
    const TranspPort& getLocalPort() const {
        return _localPort;
    }

    /**
     * Returns the remote peer's ipv4 address.
     */
    const std::string& getRemoteIpAddress() const {
        return _remoteIpAddress;
    }

    /**
     * Returns the remote peer's tcp port number
     */
    const TranspPort& getRemotePort() const {
        return _remotePort;
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
    std::string _localIpAddress;
    TranspPort _localPort = 0;
    std::string _remoteIpAddress;
    TranspPort _remotePort = 0;

    TcpSocket(const SocketFd& sd, const sockaddr* local_sa,
        const sockaddr* remote_sa);
};


/* -------------------------------------------------------------------------- */

#endif // __TCP_SOCKET_H__

