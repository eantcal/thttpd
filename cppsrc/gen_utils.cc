//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

/// \file gen_utils.cc
/// \brief Collection of general purpose utilities


/* -------------------------------------------------------------------------- */

#include "gen_utils.h"
#include "os_dep.h"


/* -------------------------------------------------------------------------- */

void gen_utils::convert_duration_in_timeval(const timeout_t& d, timeval& tv)
{
    std::chrono::microseconds usec
        = std::chrono::duration_cast<std::chrono::microseconds>(d);

    if (usec <= std::chrono::microseconds(0)) {
        tv.tv_sec = tv.tv_usec = 0;
    } else {
        tv.tv_sec = static_cast<long>(usec.count() / 1000000LL);
        tv.tv_usec = static_cast<long>(usec.count() % 1000000LL);
    }
}


/* -------------------------------------------------------------------------- */

void gen_utils::get_local_time(std::string& local_time)
{
    time_t ltime;
    ltime = ::time(NULL); // get current calendar time
    local_time = ::asctime(::localtime(&ltime));
    gen_utils::remove_last_ch_if(local_time, '\n');
}


/* -------------------------------------------------------------------------- */

void gen_utils::remove_last_ch_if(std::string& s, char c)
{
    while (!s.empty() && s.c_str()[s.size() - 1] == c)
        s = s.substr(0, s.size() - 1);
}


/* -------------------------------------------------------------------------- */

bool gen_utils::file_stat(const std::string& filename, std::string& date_time,
    std::string& ext, size_t& fsize)
{
    struct stat rstat = { 0 };
    int ret = stat(filename.c_str(), &rstat);

    if (ret >= 0) {
        date_time = ctime(&rstat.st_atime);
        fsize = rstat.st_size;

        std::string::size_type pos = filename.rfind('.');
        ext = pos != std::string::npos
            ? filename.substr(pos, filename.size() - pos)
            : ".";

        gen_utils::remove_last_ch_if(date_time, '\n');

        return true;
    }

    return false;
}


/* -------------------------------------------------------------------------- */

bool gen_utils::split_line_in_tokens(const std::string& line,
    std::vector<std::string>& tokens, const std::string& sep)
{
    if (line.empty() || line.size() < sep.size())
        return false;

    std::string subline = line;

    while (!subline.empty()) {
        size_t pos = subline.find(sep);

        if (pos == std::string::npos) {
            tokens.push_back(subline);
            return true;
        }

        tokens.push_back(subline.substr(0, pos));

        size_t off = pos + sep.size();

        subline = subline.substr(off, subline.size() - off);
    }

    return true;
}


