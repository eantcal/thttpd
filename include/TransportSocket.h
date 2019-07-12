//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __TRANSPORT_SOCKET_H__
#define __TRANSPORT_SOCKET_H__


/* -------------------------------------------------------------------------- */

#include "config.h"
#include "OsSocketSupport.h"


#include <chrono>
#include <cstdint>
#include <fstream>


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

#endif // __TRANSPORT_SOCKET_H__

