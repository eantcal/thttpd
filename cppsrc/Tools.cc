//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "Tools.h"


/* -------------------------------------------------------------------------- */

void Tools::convertDurationInTimeval(const TimeoutInterval& d, timeval& tv)
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

void Tools::getLocalTime(std::string& localTime)
{
    time_t ltime;
    ltime = ::time(NULL); // get current calendar time
    localTime = ::asctime(::localtime(&ltime));
    Tools::removeLastCharIf(localTime, '\n');
}


/* -------------------------------------------------------------------------- */

void Tools::removeLastCharIf(std::string& s, char c)
{
    while (!s.empty() && s.c_str()[s.size() - 1] == c)
        s = s.substr(0, s.size() - 1);
}


/* -------------------------------------------------------------------------- */

bool Tools::fileStat(
    const std::string& fileName, 
    std::string& dateTime,
    std::string& ext, 
    size_t& fsize)
{
    struct stat rstat = { 0 };
    int ret = stat(fileName.c_str(), &rstat);

    if (ret >= 0) {
        dateTime = ctime(&rstat.st_atime);
        fsize = rstat.st_size;

        std::string::size_type pos = fileName.rfind('.');
        ext = pos != std::string::npos
            ? fileName.substr(pos, fileName.size() - pos)
            : ".";

        Tools::removeLastCharIf(dateTime, '\n');

        return true;
    }

    return false;
}


/* -------------------------------------------------------------------------- */

bool Tools::splitLineInTokens(const std::string& line,
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


