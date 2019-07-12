//
// This file is part of thttpd
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __TOOLS_H__
#define __TOOLS_H__


/* -------------------------------------------------------------------------- */

#include <chrono>
#include <regex>
#include <string>
#include <time.h>

#include "OsSocketSupport.h"


/* -------------------------------------------------------------------------- */

namespace Tools {


/* -------------------------------------------------------------------------- */

using TimeoutInterval = std::chrono::system_clock::duration;


/* -------------------------------------------------------------------------- */

/**
* Convert a timeval object into standard duration object
*
* @param d  Duration
* @param tv Timeval source object
*/
void convertDurationInTimeval(const TimeoutInterval& d, timeval& tv);


/* -------------------------------------------------------------------------- */

/**
 * Get the system time, corrected for the local time zone
 * Time format is "DoW Mon dd hh:mm:ss yyyy"
 * Example "Thu Sep 19 10:03:50 2013"
 *
 * @param localTime will contain the time
 */
void getLocalTime(std::string& localTime);


/* -------------------------------------------------------------------------- */

/**
 * Get the system time, corrected for the local time zone
 * Time format is "DoW Mon dd hh:mm:ss yyyy"
 * Example "Thu Sep 19 10:03:50 2013"
 *
 * @return the string will contain the time
 */
inline std::string getLocalTime()
{
    std::string lt;
    getLocalTime(lt);
    return lt;
}


/* -------------------------------------------------------------------------- */

/**
 *  Removes any instances of character c if present at the end of the string s
 *
 *  @param s The input / ouput string
 *  @param c Searching character to remove
 */
void removeLastCharIf(std::string& s, char c);


/* -------------------------------------------------------------------------- */

/**
 *  Returns file attributes of fileName.
 *
 *  @param fileName String containing the path of existing file
 *  @param dateTime Time of last modification of file
 *  @param ext File extension or "." if there is no any
 *  @return true if operation successfully completed, false otherwise
 */
bool fileStat(const std::string& fileName, std::string& dateTime,
    std::string& ext, size_t& fsize);


/* -------------------------------------------------------------------------- */

/**
 *  Returns file size
 *
 *  @param fileName String containing the path of existing file/directory
 *  @return file size if operation successfully completed, -1 otherwise
 */
inline size_t file_size(const std::string& fileName)
{
    std::string dateTime;
    std::string ext;
    size_t fsize = 0;

    const bool ok = fileStat(fileName, dateTime, ext, fsize);

    return ok ? fsize : -1;
}


/* -------------------------------------------------------------------------- */

/**
*  Returns true if file/directory fileName exists
*
*  @param fileName String containing the path of existing file/directory
*  @return true if operation successfully completed, false otherwise
*/
inline bool fileExists(const std::string& fileName)
{
    std::string dateTime;
    std::string ext;
    size_t fsize = 0;

    return fileStat(fileName, dateTime, ext, fsize);
}


/* -------------------------------------------------------------------------- */

/**
 * Split text line in a vector of tokens.
 *
 * @param line The string to split
 * @param tokens The vector of splitted tokens
 * @param sep The separator string used to recognize the tokens
 *
 * @return true if operation successfully completed, false otherwise
 */
bool splitLineInTokens(const std::string& line,
    std::vector<std::string>& tokens, const std::string& sep);


} // namespace Tools


/* -------------------------------------------------------------------------- */

#endif // __TOOLS_H__

