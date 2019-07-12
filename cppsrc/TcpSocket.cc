//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "TcpSocket.h"


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

