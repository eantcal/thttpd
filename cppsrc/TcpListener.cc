//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "TcpListener.h"

#include <memory>
#include <string.h>
#include <thread>


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

