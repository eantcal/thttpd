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

#include "socket_utils.h"
#include "gen_utils.h"

#include <memory>
#include <string.h>
#include <thread>


/* -------------------------------------------------------------------------- */
// basic_socket_t

/* -------------------------------------------------------------------------- */

basic_socket_t::~basic_socket_t()
{
    if (is_valid())
        os_dep::close_socket(get_sd());
}


/* -------------------------------------------------------------------------- */

basic_socket_t::wait_ev_t basic_socket_t::wait_for_recv_event(
    const basic_socket_t::timeout_t& timeout)
{
    struct timeval tv_timeout = { 0 };
    gen_utils::convert_duration_in_timeval(timeout, tv_timeout);

    fd_set rd_mask;

    // Add socket to receive select mask
    FD_ZERO(&rd_mask);
    FD_SET(get_sd(), &rd_mask);

    long nd = select(FD_SETSIZE, &rd_mask, (fd_set*)0, (fd_set*)0, &tv_timeout);

    if (nd == 0)
        return wait_ev_t::TIMEOUT;
    else if (nd < 0)
        return wait_ev_t::RECV_ERROR;

    return wait_ev_t::RECV_DATA;
}


/* -------------------------------------------------------------------------- */

int basic_socket_t::send_file(const std::string& filepath)
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
// tcp_socket_t

/* -------------------------------------------------------------------------- */

tcp_socket_t& tcp_socket_t::operator<<(const std::string& text)
{
    send(text);
    return *this;
}


/* -------------------------------------------------------------------------- */

tcp_socket_t::tcp_socket_t(const socket_desc_t& sd, const sockaddr* local_sa,
    const sockaddr* remote_sa)
    : basic_socket_t(sd)
{
    auto conv = [](port_t& port, std::string& ip, const sockaddr* sa) {
        port = htons(reinterpret_cast<const sockaddr_in*>(sa)->sin_port);
        ip = std::string(
            inet_ntoa(reinterpret_cast<const sockaddr_in*>(sa)->sin_addr));
    };

    conv(_local_port, _local_ip, local_sa);
    conv(_remote_port, _remote_ip, remote_sa);
}


/* -------------------------------------------------------------------------- */
// tcp_listener_t

/* -------------------------------------------------------------------------- */

tcp_listener_t::tcp_listener_t()
    : basic_socket_t(int(::socket(AF_INET, SOCK_STREAM, 0)))
    , _state(is_valid() ? state_t::VALID : state_t::INVALID)
    , _bind_st(bind_st_t::UNBOUND)
{
    memset(&_local_ip_port_sa_in, 0, sizeof(_local_ip_port_sa_in));
}


/* -------------------------------------------------------------------------- */

bool tcp_listener_t::bind(const std::string& ip, const port_t& port)
{
    if (get_sd() <= 0)
        return false;

    sockaddr_in& sin = _local_ip_port_sa_in;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip.empty() ? INADDR_ANY : inet_addr(ip.c_str());
    sin.sin_port = htons(port);

    if (0 == ::bind(get_sd(), reinterpret_cast<const sockaddr*>(&sin), sizeof(sin))) {
        _bind_st = bind_st_t::BOUND;
        return true;
    }

    return false;
}


/* -------------------------------------------------------------------------- */

tcp_socket_t::handle_t tcp_listener_t::accept()
{
    if (get_state() != state_t::VALID)
        return tcp_socket_t::handle_t();

    sockaddr remote_sockaddr = { 0 };

    struct sockaddr* local_sockaddr
        = reinterpret_cast<struct sockaddr*>(&_local_ip_port_sa_in);

    socklen_t sockaddrlen = sizeof(struct sockaddr);

    socket_desc_t sd = int(::accept(get_sd(), &remote_sockaddr, &sockaddrlen));

    tcp_socket_t::handle_t handle = tcp_socket_t::handle_t(sd > 0
            ? new tcp_socket_t(sd, local_sockaddr, &remote_sockaddr)
            : nullptr);

    return handle;
}

