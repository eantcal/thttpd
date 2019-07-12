//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

/// \file http_server.cc
/// \brief Implementation of HTTP classes


/* -------------------------------------------------------------------------- */
// HTTP Server

/* -------------------------------------------------------------------------- */

#include "http_server.h"
#include "utils.h"
#include "osSocketSpecific.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>


/* -------------------------------------------------------------------------- */
// HttpRequest

/* -------------------------------------------------------------------------- */

void HttpRequest::parseMethod(const std::string& method)
{
    if (method == "GET")
        _method = Method::GET;
    else if (method == "HEAD")
        _method = Method::HEAD;
    else if (method == "POST")
        _method = Method::POST;
    else
        _method = Method::UNKNOWN;
}


/* -------------------------------------------------------------------------- */

void HttpRequest::parseVersion(const std::string& ver)
{
    const size_t vstrlen = sizeof("HTTP/x.x") - 1;
    std::string v = ver.size() > vstrlen ? ver.substr(0, vstrlen) : ver;

    if (v == "HTTP/1.0")
        _version = Version::HTTP_1_0;
    else if (v == "HTTP/1.1")
        _version = Version::HTTP_1_1;
    else
        _version = Version::UNKNOWN;
}


/* -------------------------------------------------------------------------- */

std::ostream& HttpRequest::dump(std::ostream& os, const std::string& id)
{

    std::string ss;
    ss = ">>> REQUEST " + id + "\n";
    for (auto e : get_header())
        ss += e;
    os << ss << "\n";

    return os;
}


/* -------------------------------------------------------------------------- */
// HttpResponse


/* -------------------------------------------------------------------------- */

void HttpResponse::formatError(
    std::string& output, int code, const std::string& msg)
{
    std::string scode = std::to_string(code);

    std::string error_html = "<html><head><title>" + scode + " " + msg
        + "</title></head>" + "<body>Forbidden</body></html>\r\n";

    output = "HTTP/1.1 " + scode + " " + msg + "\r\n";
    output += "Date: " + utils::getLocalTime() + "\r\n";
    output += "Server: " HTTP_SERVER_NAME "\r\n";
    output += "Content-Length: " + std::to_string(error_html.size()) + "\r\n";
    output += "Connection: Keep-Alive\r\n";
    output += "Content-Type: text/html\r\n\r\n";
    output += error_html;
}


/* -------------------------------------------------------------------------- */

void HttpResponse::formatPositiveResponse(
    std::string& response, std::string& fileTime,
    std::string& fileExt,
    size_t& contentLen)
{

    response = "HTTP/1.1 200 OK\r\n";
    response += "Date: " + utils::getLocalTime() + "\r\n";
    response += "Server: " HTTP_SERVER_NAME "\r\n";
    response += "Content-Length: " + std::to_string(contentLen) + "\r\n";
    response += "Connection: Keep-Alive\r\n";
    response += "Last Modified: " + fileTime + "\r\n";
    response += "Content-Type: ";

    // Resolve mime type using the uri/file extension
    auto it = _mimeTbl.find(fileExt);

    response
        += it != _mimeTbl.end() ? it->second : "application/octet-stream";

    // Close the rensponse header by using the sequence CRFL twice
    response += "\r\n\r\n";
}

/* -------------------------------------------------------------------------- */

HttpResponse::HttpResponse(
    const HttpRequest& request, const std::string& webRootPath)
{
    if (request.getMethod() == HttpRequest::Method::UNKNOWN) {
        formatError(_response, 403, "Forbidden");
        return;
    }

    auto rpath = [](std::string& s) {
        if (!s.empty() && s[0] != '/')
            s = "/" + s;
    };

    _localUriPath = request.getUri();
    rpath(_localUriPath);
    _localUriPath = webRootPath + _localUriPath;

    std::string fileTime, fileExt;
    size_t contentLen = 0;

    if (utils::fileStat(_localUriPath, fileTime, fileExt, contentLen)) {
        formatPositiveResponse(_response, fileTime, fileExt, contentLen);
    } 
    else {
        formatError(_response, 404, "Not Found");
    }
}


/* -------------------------------------------------------------------------- */

std::ostream& HttpResponse::dump(std::ostream& os, const std::string& id)
{
    std::string ss;
    ss = "<<< RESPONSE " + id + "\n";
    ss += _response;
    os << ss << "\n";

    return os;
}


/* -------------------------------------------------------------------------- */
// HttpSocket


/* -------------------------------------------------------------------------- */

HttpSocket& HttpSocket::operator=(TcpSocket::Handle handle)
{
    _socketHandle = handle;
    return *this;
}


/* -------------------------------------------------------------------------- */

HttpRequest::Handle HttpSocket::recv()
{
    HttpRequest::Handle handle(new HttpRequest);

    char c = 0;
    int ret = 1;

    enum class CrLfSeq { CR1, LF1, CR2, LF2, IDLE } s = CrLfSeq::IDLE;

    auto crlf = [&s](char c) -> bool {
        switch (s) {
        case CrLfSeq::IDLE:
            s = (c == '\r') ? CrLfSeq::CR1 : CrLfSeq::IDLE;
            break;
        case CrLfSeq::CR1:
            s = (c == '\n') ? CrLfSeq::LF1 : CrLfSeq::IDLE;
            break;
        case CrLfSeq::LF1:
            s = (c == '\r') ? CrLfSeq::CR2 : CrLfSeq::IDLE;
            break;
        case CrLfSeq::CR2:
            s = (c == '\n') ? CrLfSeq::LF2 : CrLfSeq::IDLE;
            break;
        default:
            break;
        }

        return s == CrLfSeq::LF2;
    };

    std::string line;

    while (ret > 0 && _connUp && _socketHandle) {
        ret = _socketHandle->recv(&c, 1);

        if (ret > 0) {
            line += c;
        } else if (ret <= 0) {
            _connUp = false;
            break;
        }

        if (crlf(c)) {
            break;
        }

        if (s == CrLfSeq::LF1) {
            if (!line.empty()) {
                handle->addHeader(line);
                line.clear();
            }
        }
    }

    if (ret < 0 || !_socketHandle || handle->get_header().empty()) {
        return handle;
    }

    std::string request = *handle->get_header().cbegin();
    std::vector<std::string> tokens;

    if (!utils::splitLineInTokens(request, tokens, " ")) {
        return handle;
    }

    if (tokens.size() != 3) {
        return handle;
    }

    handle->parseMethod(tokens[0]);
    handle->parseUri(tokens[1]);
    handle->parseVersion(tokens[2]);

    return handle;
}


/* -------------------------------------------------------------------------- */

HttpSocket& HttpSocket::operator<<(const HttpResponse& response)
{
    const std::string& response_txt = response;
    size_t to_send = response_txt.size();

    while (to_send > 0) {
        int sent = _socketHandle->send(response);
        if (sent < 0) {
            _connUp = false;
            break;
        }
        to_send -= sent;
    }

    return *this;
}


/* -------------------------------------------------------------------------- */
// HttpServerTask

/* -------------------------------------------------------------------------- */

class HttpServerTask {
private:
    std::ostream& _logger;
    bool _verboseModeOn = true;
    TcpSocket::Handle _tcpSocketHandle;
    std::string _webRootPath;

    std::ostream& log() { 
        return _logger; 
    }

    bool verboseModeOn() const { 
        return _verboseModeOn; 
    }

    TcpSocket::Handle& getTcpSocketHandle() {
        return _tcpSocketHandle;
    }

    const std::string& getWebRootPath() const { 
        return _webRootPath; 
    }

    HttpServerTask(bool verboseModeOn, std::ostream& loggerOStream,
        TcpSocket::Handle socketHandle, const std::string& webRootPath)
        : _verboseModeOn(verboseModeOn)
        , _logger(loggerOStream)
        , _tcpSocketHandle(socketHandle)
        , _webRootPath(webRootPath)
    {
    }

public:
    using Handle = std::shared_ptr<HttpServerTask>;

    inline static Handle create(
        bool verboseModeOn, 
        std::ostream& loggerOStream,
        TcpSocket::Handle socketHandle, 
        const std::string& webRootPath)
    {
        return Handle(new HttpServerTask(
            verboseModeOn, 
            loggerOStream, 
            socketHandle, 
            webRootPath));
    }

    HttpServerTask() = delete;
    void operator()(Handle task_handle);
};


/* -------------------------------------------------------------------------- */

// Handles the HTTP server request
// This method executes in a specific thread context for
// each accepted HTTP request
void HttpServerTask::operator()(Handle task_handle)
{
    (void)task_handle;

    const int sd = getTcpSocketHandle()->getSocketFd();

    // Generates an identifier for recognizing the transaction
    auto transactionId = [sd]() {
        return "[" + std::to_string(sd) + "] " + "["
            + utils::getLocalTime() + "]";
    };

    if (verboseModeOn())
        log() << transactionId() << "---- http_server_task +\n\n";

    while (getTcpSocketHandle()) {
        // Create an http socket around a connected tcp socket
        HttpSocket httpSocket(getTcpSocketHandle());

        // Wait for a request from remote peer
        HttpRequest::Handle httpRequest;
        httpSocket >> httpRequest;

        // If an error occoured terminate the task
        if (!httpSocket)
            break;

        // Log the request
        if (verboseModeOn())
            httpRequest->dump(log(), transactionId());

        // Build a response to previous HTTP request
        HttpResponse response(*httpRequest, getWebRootPath());

        // Send the response to remote peer
        httpSocket << response;

        // If HTTP command line method isn't HEAD then send requested URI
        if (httpRequest->getMethod() != HttpRequest::Method::HEAD) {
            if (0 > httpSocket.sendFile(response.getLocalUriPath())) {
                if (verboseModeOn())
                    log() << transactionId() << "Error sending '"
                          << response.getLocalUriPath() << "'\n\n";
                break;
            }
        }

        if (verboseModeOn())
            response.dump(log(), transactionId());
    }

    getTcpSocketHandle()->shutdown();

    if (verboseModeOn()) {
        log() << transactionId() << "---- http_server_task -\n\n";
        log().flush();
    }
}


/* -------------------------------------------------------------------------- */
// HttpServer


/* -------------------------------------------------------------------------- */

auto HttpServer::getInstance() -> HttpServer&
{
    if (_instance == nullptr) {
        _instance = new HttpServer();
    }

    return *_instance;
}


/* -------------------------------------------------------------------------- */

bool HttpServer::bind(TranspPort port)
{
    _tcpServer = TcpListener::create();

    if (!_tcpServer) {
        return false;
    }

    if (!_tcpServer->bind(port)) {
        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool HttpServer::listen(int maxConnections)
{
    if (!_tcpServer) {
        return false;
    }

    if (!_tcpServer->listen(maxConnections)) {
        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool HttpServer::run()
{
    // Create a thread for each TCP accepted connection and
    // delegate it to handle HTTP request / response
    while (true) {
        const TcpSocket::Handle handle = accept();

        // Fatal error: we stop the server
        if (!handle) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        assert(_loggerOStreamPtr);

        HttpServerTask::Handle task_handle = HttpServerTask::create(
            _verboseModeOn, *_loggerOStreamPtr, handle, getWebRootPath());

        // Coping the http_server_task handle (shared_ptr) the reference
        // count is automatically increased by one
        std::thread worker_thread(*task_handle, task_handle);

        worker_thread.detach();
    }

    // Ok, following instruction won't be ever executed
    return true;
}


/* -------------------------------------------------------------------------- */

bool HttpServer::waitForData(const TransportSocket::TimeoutInterval& timeout)
{
    if (!_tcpServer->isValid()
        || _tcpServer->getBindingState()
            == TcpListener::BindingState::UNBOUND) {
        return false;
    }

    const TcpListener::WaitingEvent st
        = _tcpServer->waitForRecvEvent(timeout);

    return st == TcpListener::WaitingEvent::RECV_DATA;
}


/* -------------------------------------------------------------------------- */

HttpServer* HttpServer::_instance = nullptr;

