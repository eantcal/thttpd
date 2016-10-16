/*
 *  tiny_http_server.cc
 *
 *  This file is part of TinyHttpServer
 *
 *  TinyHttpServer is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  TinyHttpServer is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TinyHttpServer; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
 *
 *  Author:	Antonino Calderone, <acaldmail@gmail.com>
 *
 */


// -----------------------------------------------------------------------------


#include "gen_utils.h"
#include "http_server.h"
#include "socket_utils.h"

#include <iostream>
#include <string>


// -----------------------------------------------------------------------------


class prog_args_t {
private:
    std::string _prog_name;
    std::string _command_line;
    std::string _web_root = HTTP_SERVER_WROOT;
    tcp_socket_t::port_t _http_server_port = HTTP_SERVER_PORT;
    bool _show_help = false;
    bool _show_ver = false;
    bool _error = false;
    bool _verbose_mode = false;
    std::string _err_msg;

    static const int _min_ver = HTTP_SERVER_MIN_V;
    static const int _maj_ver = HTTP_SERVER_MAJ_V;

public:
    prog_args_t() = delete;


    inline const std::string& get_prog_name() const { return _prog_name; }


    inline const std::string& get_command_line() const { return _command_line; }


    inline const std::string& get_web_root() const { return _web_root; }


    inline tcp_socket_t::port_t get_http_server_port() const
    {
        return _http_server_port;
    }


    inline bool is_good() const { return !_error; }


    inline bool verbose_mode() const { return _verbose_mode; }


    inline const std::string& error() const { return _err_msg; }


    bool show_info(std::ostream& os) const
    {
        if (_show_ver)
            os << HTTP_SERVER_NAME << " " << _maj_ver << "." << _min_ver
               << "\n";

        if (!_show_help)
            return _show_ver;

        os << "Usage:\n";
        os << "\t" << get_prog_name() << "\n";
        os << "\t\t-p | --port <port>\n";
        os << "\t\t\tBind server to a TCP port number (default is "
           << HTTP_SERVER_PORT << ") \n";
        os << "\t\t-w | --webroot <working_dir_path>\n";
        os << "\t\t\tSet a local working directory (default is "
           << HTTP_SERVER_WROOT << ") \n";
        os << "\t\t-vv | --verbose\n";
        os << "\t\t\tEnable logging on stderr\n";
        os << "\t\t-v | --version\n";
        os << "\t\t\tShow software version\n";
        os << "\t\t-h | --help\n";
        os << "\t\t\tShow this help \n";

        return true;
    }


    /**
     * Parse the command line
     */
    prog_args_t(int argc, char* argv[])
    {
        if (!argc)
            return;

        _prog_name = argv[0];
        _command_line = _prog_name;

        if (argc <= 1)
            return;

        enum class state_t { OPTION, PORT, WEBROOT } state = state_t::OPTION;

        for (int idx = 1; idx < argc; ++idx) {
            std::string sarg = argv[idx];

            _command_line += " ";
            _command_line += sarg;

            switch (state) {
            case state_t::OPTION:
                if (sarg == "--port" || sarg == "-p") {
                    state = state_t::PORT;
                } else if (sarg == "--webroot" || sarg == "-w") {
                    state = state_t::WEBROOT;
                } else if (sarg == "--help" || sarg == "-h") {
                    _show_help = true;
                    state = state_t::OPTION;
                } else if (sarg == "--version" || sarg == "-v") {
                    _show_ver = true;
                    state = state_t::OPTION;
                } else if (sarg == "--verbose" || sarg == "-vv") {
                    _verbose_mode = true;
                    state = state_t::OPTION;
                } else {
                    _err_msg = "Unknown option '" + sarg
                        + "', try with --help or -h";
                    _error = true;
                    return;
                }
                break;

            case state_t::WEBROOT:
                _web_root = sarg;
                state = state_t::OPTION;
                break;

            case state_t::PORT:
                _http_server_port = std::stoi(sarg);
                state = state_t::OPTION;
                break;
            }
        }
    }
};

/**
 * Program entry point
 */
int main(int argc, char* argv[])
{
    std::string msg;

    // Initialize O/S specific libraries
    if (!os_dep::init_lib(msg)) {
        if (!msg.empty())
            std::cerr << msg << std::endl;

        return 1;
    }

    // Parse the command line
    prog_args_t args(argc, argv);


    if (!args.is_good()) {
        std::cerr << args.error() << std::endl;
        return 1;
    }

    if (args.show_info(std::cout)) {
        return 0;
    }

    http_server_t& httpsrv = http_server_t::get_instance();

    httpsrv.set_web_root(args.get_web_root());

    bool res = httpsrv.bind(args.get_http_server_port());

    if (!res) {
        std::cerr << "Error binding server port " << httpsrv.get_local_port()
                  << "\n";
        return 1;
    }

    res = httpsrv.listen(HTTP_SERVER_BACKLOG);
    if (!res) {
        std::cerr << "Error setting listeing mode\n";
        return 1;
    }

    std::cout << gen_utils::get_local_time() << std::endl
              << "Command line :'" << args.get_command_line() << "'"
              << std::endl
              << HTTP_SERVER_NAME << " is listening on TCP port "
              << args.get_http_server_port() << std::endl
              << "Working directory is '" << args.get_web_root() << "'\n";

    httpsrv.set_logger(args.verbose_mode() ? &std::clog : nullptr);

    if (!httpsrv.run()) {
        std::cerr << "Error starting the server\n";
        return 1;
    }

    return 0;
}
