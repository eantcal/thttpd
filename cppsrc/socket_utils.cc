//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

/// \file socket_utils.cc
/// \brief Implementation of socket classes containing some
/// useful stuff cross-platform for manipulating socket


/* -------------------------------------------------------------------------- */
// Socket Utils

/* -------------------------------------------------------------------------- */

#include "osSocketSpecific.h"
#include "utils.h"
#include "socket_utils.h"

#include <memory>
#include <string.h>
#include <thread>


/* -------------------------------------------------------------------------- */
// TransportSocket

/* -------------------------------------------------------------------------- */

TransportSocket::~TransportSocket()
{
    if (isValid())
        osSocketSpecific::closeSocketFd(getSocketFd());
}


/* -------------------------------------------------------------------------- */

TransportSocket::WaitingEvent TransportSocket::waitForRecvEvent(
    const TransportSocket::TimeoutInterval& timeout)
{
    struct timeval tv_timeout = { 0 };
    utils::convertDurationInTimeval(timeout, tv_timeout);

    fd_set rd_mask;

    // Add socket to receive select mask
    FD_ZERO(&rd_mask);
    FD_SET(getSocketFd(), &rd_mask);

    long nd = select(FD_SETSIZE, &rd_mask, (fd_set*)0, (fd_set*)0, &tv_timeout);

    if (nd == 0)
        return WaitingEvent::TIMEOUT;
    else if (nd < 0)
        return WaitingEvent::RECV_ERROR;

    return WaitingEvent::RECV_DATA;
}


/* -------------------------------------------------------------------------- */

int TransportSocket::sendFile(const std::string& filepath)
{
    std::ifstream ifs(filepath.c_str(), std::ios::in | std::ios::binary);

    if (!ifs.is_open())
        return -1;

    std::unique_ptr<char[]> buffer(new char[TX_BUFFER_SIZE]);

    int sent_bytes = 0;

    while (ifs.good()) {
        ifs.read(buffer.get(), sizeof(buffer));

        int size = static_cast<int>(ifs.gcount());

        if (size > 0) {
            int bsent = 0;

            // sent the whole buffer content
            while (bsent < size) {
                int txc = send(buffer.get(), size);
                if (txc < 0)
                    return -1;

                if (txc == 0) { // tx queue is congested ?
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }

                bsent += txc;
            }

            sent_bytes += bsent;
        } 
        else {
            return -1;
        }
    }

    return sent_bytes;
}


/* -------------------------------------------------------------------------- */
// TcpSocket

/* -------------------------------------------------------------------------- */

TcpSocket& TcpSocket::operator<<(const std::string& text)
{
    send(text);
    return *this;
}


/* -------------------------------------------------------------------------- */

TcpSocket::TcpSocket(const SocketFd& sd, const sockaddr* local_sa,
    const sockaddr* remote_sa)
    : TransportSocket(sd)
{
    auto conv = [](TranspPort& port, std::string& ip, const sockaddr* sa) {
        port = htons(reinterpret_cast<const sockaddr_in*>(sa)->sin_port);
        ip = std::string(
            inet_ntoa(reinterpret_cast<const sockaddr_in*>(sa)->sin_addr));
    };

    conv(_local_port, _local_ip, local_sa);
    conv(_remote_port, _remote_ip, remote_sa);
}


/* -------------------------------------------------------------------------- */
// TcpListener

/* -------------------------------------------------------------------------- */

TcpListener::TcpListener()
    : TransportSocket(int(::socket(AF_INET, SOCK_STREAM, 0)))
    , _state(isValid() ? State::VALID : State::INVALID)
    , _bind_st(BindingState::UNBOUND)
{
    memset(&_local_ip_port_sa_in, 0, sizeof(_local_ip_port_sa_in));
}


/* -------------------------------------------------------------------------- */

bool TcpListener::bind(const std::string& ip, const TranspPort& port)
{
    if (getSocketFd() <= 0)
        return false;

    sockaddr_in& sin = _local_ip_port_sa_in;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip.empty() ? INADDR_ANY : inet_addr(ip.c_str());
    sin.sin_port = htons(port);

    if (0 == ::bind(getSocketFd(), reinterpret_cast<const sockaddr*>(&sin), sizeof(sin))) {
        _bind_st = BindingState::BOUND;
        return true;
    }

    return false;
}


/* -------------------------------------------------------------------------- */

TcpSocket::Handle TcpListener::accept()
{
    if (getState() != State::VALID)
        return TcpSocket::Handle();

    sockaddr remote_sockaddr = { 0 };

    struct sockaddr* local_sockaddr
        = reinterpret_cast<struct sockaddr*>(&_local_ip_port_sa_in);

    socklen_t sockaddrlen = sizeof(struct sockaddr);

    SocketFd sd = int(::accept(getSocketFd(), &remote_sockaddr, &sockaddrlen));

    TcpSocket::Handle handle = TcpSocket::Handle(sd > 0
            ? new TcpSocket(sd, local_sockaddr, &remote_sockaddr)
            : nullptr);

    return handle;
}

