//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */
// HTTP Server

/* -------------------------------------------------------------------------- */

#include "HttpRequest.h"


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

