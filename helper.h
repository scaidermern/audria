#ifndef HELPER_H
#define HELPER_H HELPER_H

#include <iomanip>
#include <sstream>
#include <string>
#include <cassert>

/// returns wether the given directory exists
bool dirExists(const std::string& dir);

/// returns wether the given file is readable
bool fileReadable(const std::string& path);

/// returns wether the given string is a number
/// @note doesn't detect all errors, but is fast
bool isNumber(const std::string& str);

/// converts a std::string to a number
/// @note expensive, try to avoid it
/// @note no real error handling
template <class T>
T stringToNumber(const std::string& str) {
    assert(!str.empty());
    
    T number = 0;
    std::stringstream sstr(str);
    sstr >> number;
    assert(sstr); // check if conversion succeeded
    
    return number;
}

/// converts a number to a std::string
/// @note expensive, try to avoid it
/// @note no real error handling
template <class T>
std::string numberToString(const T number) {
    std::stringstream sstr;
    sstr << std::fixed << std::setprecision(2) << number;
    
    assert(!sstr.str().empty());
    return sstr.str();
}

/// returns the system uptime in seconds from /proc/uptime
/// or std::numeric_limits<double>::quiet_NaN() on error
double uptime();

/// returns the kernel's hz value, might be wrong, see 'proc/sysinfo.c'
/// from the 'procps' package for other crappy ways of obtaining this value
long getHertz();

#endif // HELPER_H
