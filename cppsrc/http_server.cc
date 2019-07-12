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
#include "gen_utils.h"
#include "os_dep.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>


/* -------------------------------------------------------------------------- */
// http_request_t

/* -------------------------------------------------------------------------- */

void http_request_t::parse_method(const std::string& method)
{
    if (method == "GET")
        _method = method_t::GET;
    else if (method == "HEAD")
        _method = method_t::HEAD;
    else if (method == "POST")
        _method = method_t::POST;
    else
        _method = method_t::UNKNOWN;
}


/* -------------------------------------------------------------------------- */

void http_request_t::parse_version(const std::string& ver)
{
    const size_t vstrlen = sizeof("HTTP/x.x") - 1;
    std::string v = ver.size() > vstrlen ? ver.substr(0, vstrlen) : ver;

    if (v == "HTTP/1.0")
        _version = version_t::HTTP_1_0;
    else if (v == "HTTP/1.1")
        _version = version_t::HTTP_1_1;
    else
        _version = version_t::UNKNOWN;
}


/* -------------------------------------------------------------------------- */

std::ostream& http_request_t::dump(std::ostream& os, const std::string& id)
{

    std::string ss;
    ss = ">>> REQUEST " + id + "\n";
    for (auto e : get_header())
        ss += e;
    os << ss << "\n";

    return os;
}


/* -------------------------------------------------------------------------- */
// http_response_t


/* -------------------------------------------------------------------------- */

void http_response_t::format_error(
    std::string& output, int code, const std::string& msg)
{
    std::string scode = std::to_string(code);

    std::string error_html = "<html><head><title>" + scode + " " + msg
        + "</title></head>" + "<body>Forbidden</body></html>\r\n";

    output = "HTTP/1.1 " + scode + " " + msg + "\r\n";
    output += "Date: " + gen_utils::get_local_time() + "\r\n";
    output += "Server: " HTTP_SERVER_NAME "\r\n";
    output += "Content-Length: " + std::to_string(error_html.size()) + "\r\n";
    output += "Connection: Keep-Alive\r\n";
    output += "Content-Type: text/html\r\n\r\n";
    output += error_html;
}


/* -------------------------------------------------------------------------- */

http_response_t::http_response_t(
    const http_request_t& request, const std::string& web_root)
{
    if (request.get_method() == http_request_t::method_t::UNKNOWN) {
        format_error(_response, 403, "Forbidden");
        return;
    }

    auto rpath = [](std::string& s) {
        if (!s.empty() && s[0] != '/')
            s = "/" + s;
    };

    _local_uri_path = request.get_uri();
    rpath(_local_uri_path);
    _local_uri_path = web_root + _local_uri_path;

    std::string file_time, mimekey;
    size_t content_len = 0;

    if (gen_utils::file_stat(
            _local_uri_path, file_time, mimekey, content_len)) {
        _response = "HTTP/1.1 200 OK\r\n";
        _response += "Date: " + gen_utils::get_local_time() + "\r\n";
        _response += "Server: " HTTP_SERVER_NAME "\r\n";
        _response += "Content-Length: " + std::to_string(content_len) + "\r\n";
        _response += "Connection: Keep-Alive\r\n";
        _response += "Last Modified: " + file_time + "\r\n";
        _response += "Content-Type: ";

        // Resolve mime type using the uri/file extension
        auto it = _mime_tbl.find(mimekey);

        _response
            += it != _mime_tbl.end() ? it->second : "application/octet-stream";

        // Close the rensponse header by using the sequence CRFL twice
        _response += "\r\n\r\n";
    } else
        format_error(_response, 404, "Not Found");
}


/* -------------------------------------------------------------------------- */

std::ostream& http_response_t::dump(std::ostream& os, const std::string& id)
{

    std::string ss;
    ss = "<<< RESPONSE " + id + "\n";
    ss += _response;
    os << ss << "\n";

    return os;
}


/* -------------------------------------------------------------------------- */
// http_socket_t


/* -------------------------------------------------------------------------- */

http_socket_t& http_socket_t::operator=(tcp_socket_t::handle_t handle)
{
    _socket_handle = handle;
    return *this;
}


/* -------------------------------------------------------------------------- */

http_request_t::handle_t http_socket_t::recv()
{
    http_request_t::handle_t handle(new http_request_t);

    char c = 0;
    int ret = 1;

    enum class crlf_t { CR1, LF1, CR2, LF2, IDLE } s = crlf_t::IDLE;

    auto crlf = [&s](char c) -> bool {
        switch (s) {
        case crlf_t::IDLE:
            s = (c == '\r') ? crlf_t::CR1 : crlf_t::IDLE;
            break;
        case crlf_t::CR1:
            s = (c == '\n') ? crlf_t::LF1 : crlf_t::IDLE;
            break;
        case crlf_t::LF1:
            s = (c == '\r') ? crlf_t::CR2 : crlf_t::IDLE;
            break;
        case crlf_t::CR2:
            s = (c == '\n') ? crlf_t::LF2 : crlf_t::IDLE;
            break;
        default:
            break;
        }

        return s == crlf_t::LF2;
    };

    std::string line;

    while (ret > 0 && _conn_up && _socket_handle) {
        ret = _socket_handle->recv(&c, 1);

        if (ret > 0) {
            line += c;
        } else if (ret <= 0) {
            _conn_up = false;
            break;
        }

        if (crlf(c)) {
            break;
        }

        if (s == crlf_t::LF1) {
            if (!line.empty()) {
                handle->add_header(line);
                line.clear();
            }
        }
    }

    if (ret < 0 || !_socket_handle || handle->get_header().empty()) {
        return handle;
    }

    std::string request = *handle->get_header().cbegin();
    std::vector<std::string> tokens;

    if (!gen_utils::split_line_in_tokens(request, tokens, " ")) {
        return handle;
    }

    if (tokens.size() != 3) {
        return handle;
    }

    handle->parse_method(tokens[0]);
    handle->parse_uri(tokens[1]);
    handle->parse_version(tokens[2]);

    return handle;
}


/* -------------------------------------------------------------------------- */

http_socket_t& http_socket_t::operator<<(const http_response_t& response)
{
    const std::string& response_txt = response;
    size_t to_send = response_txt.size();

    while (to_send > 0) {
        int sent = _socket_handle->send(response);
        if (sent < 0) {
            _conn_up = false;
            break;
        }
        to_send -= sent;
    }

    return *this;
}


/* -------------------------------------------------------------------------- */
// http_server_task_t

/* -------------------------------------------------------------------------- */

class http_server_task_t {
private:
    std::ostream& _logger;
    bool _verbose_mode = true;
    tcp_socket_t::handle_t _tcp_socket_handle;
    std::string _web_root;

    inline std::ostream& log() { return _logger; }

    inline bool verbose_mode() const { return _verbose_mode; }

    inline tcp_socket_t::handle_t& get_tcp_socket_handle()
    {
        return _tcp_socket_handle;
    }

    inline const std::string& web_root_dir() const { return _web_root; }

    inline http_server_task_t(bool verbose_mode, std::ostream& logger,
        tcp_socket_t::handle_t socket_handle, const std::string& web_root)
        : _verbose_mode(verbose_mode)
        , _logger(logger)
        , _tcp_socket_handle(socket_handle)
        , _web_root(web_root)
    {
    }

public:
    using handle_t = std::shared_ptr<http_server_task_t>;

    inline static handle_t create(bool verbose_mode, std::ostream& logger,
        tcp_socket_t::handle_t socket_handle, const std::string& web_root)
    {
        return handle_t(new http_server_task_t(
            verbose_mode, logger, socket_handle, web_root));
    }

    http_server_task_t() = delete;
    void operator()(handle_t task_handle);
};


/* -------------------------------------------------------------------------- */

// Handles the HTTP server request
// This method executes in a specific thread context for
// each accepted HTTP request
void http_server_task_t::operator()(handle_t task_handle)
{
    (void)task_handle;

    const int sd = get_tcp_socket_handle()->get_sd();

    // Generates an identifier for recognizing the transaction
    auto transaction_id = [sd]() {
        return "[" + std::to_string(sd) + "] " + "["
            + gen_utils::get_local_time() + "]";
    };

    if (verbose_mode())
        log() << transaction_id() << "---- http_server_task +\n\n";

    while (get_tcp_socket_handle()) {
        // Create an http socket around a connected tcp socket
        http_socket_t http_socket(get_tcp_socket_handle());

        // Wait for a request from remote peer
        http_request_t::handle_t http_request;
        http_socket >> http_request;

        // If an error occoured terminate the task
        if (!http_socket)
            break;

        // Log the request
        if (verbose_mode())
            http_request->dump(log(), transaction_id());

        // Build a response to previous HTTP request
        http_response_t response(*http_request, web_root_dir());

        // Send the response to remote peer
        http_socket << response;

        // If HTTP command line method isn't HEAD then send requested URI
        if (http_request->get_method() != http_request_t::method_t::HEAD) {
            if (0 > http_socket.send_file(response.get_local_uri_path())) {
                if (verbose_mode())
                    log() << transaction_id() << "Error sending '"
                          << response.get_local_uri_path() << "'\n\n";
                break;
            }
        }

        if (verbose_mode())
            response.dump(log(), transaction_id());
    }

    get_tcp_socket_handle()->shutdown();

    if (verbose_mode()) {
        log() << transaction_id() << "---- http_server_task -\n\n";
        log().flush();
    }
}


/* -------------------------------------------------------------------------- */
// http_server_t


/* -------------------------------------------------------------------------- */

auto http_server_t::get_instance() -> http_server_t&
{
    if (_instance == nullptr) {
        _instance = new http_server_t();
    }

    return *_instance;
}


/* -------------------------------------------------------------------------- */

bool http_server_t::bind(port_t port)
{
    _tcp_server = tcp_listener_t::create();

    if (!_tcp_server) {
        return false;
    }

    if (!_tcp_server->bind(port)) {
        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool http_server_t::listen(int max_connections)
{
    if (!_tcp_server) {
        return false;
    }

    if (!_tcp_server->listen(max_connections)) {
        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool http_server_t::run()
{
    // Create a thread for each TCP accepted connection and
    // delegate it to handle HTTP request / response
    while (true) {
        const tcp_socket_t::handle_t handle = accept();

        // Fatal error: we stop the server
        if (!handle) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        assert(_logger_ptr);

        http_server_task_t::handle_t task_handle = http_server_task_t::create(
            _verbose_mode, *_logger_ptr, handle, get_web_root());

        // Coping the http_server_task handle (shared_ptr) the reference
        // count is automatically increased by one
        std::thread worker_thread(*task_handle, task_handle);

        worker_thread.detach();
    }

    // Ok, following instruction won't be ever executed
    return true;
}


/* -------------------------------------------------------------------------- */

bool http_server_t::wait_for_conn(const basic_socket_t::timeout_t& timeout)
{
    if (!_tcp_server->is_valid()
        || _tcp_server->get_bind_state()
            == tcp_listener_t::bind_st_t::UNBOUND) {
        return false;
    }

    const tcp_listener_t::wait_ev_t st
        = _tcp_server->wait_for_recv_event(timeout);

    return st == tcp_listener_t::wait_ev_t::RECV_DATA;
}


/* -------------------------------------------------------------------------- */

http_server_t* http_server_t::_instance = nullptr;

