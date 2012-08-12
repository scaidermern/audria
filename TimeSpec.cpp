#include "TimeSpec.h"

#include <iomanip>
#include <iostream>
#include <cassert>

// note: although C++11 finally provides nice initializer lists for
//       structs we avoid them for reasons of backwards compatibility

const long TimeSpec::secInNsec = 1000 * 1000 * 1000;

TimeSpec::TimeSpec(const long secs, const long nsecs) : ts() {
    assert(secs >= 0);  // cannot handle negative times
    assert(nsecs >= 0); // cannot handle negative times
    ts.tv_sec  = secs;
    ts.tv_nsec = nsecs;
}

TimeSpec::TimeSpec(const double secs) : ts() {
    assert(secs >= 0); // cannot handle negative times
    ts.tv_sec  = (long)(secs);
    ts.tv_nsec = (long)(secs * secInNsec) % secInNsec;
}
   
double TimeSpec::seconds() const {
    return (double)ts.tv_sec + (double)ts.tv_nsec / (double)secInNsec;
}
    
bool TimeSpec::operator==(const TimeSpec& other) const {
    return (ts.tv_sec  ==  other.ts.tv_sec) &&
           (ts.tv_nsec == other.ts.tv_nsec);
}

bool TimeSpec::operator<(const TimeSpec& other) const {
    if ( ts.tv_sec <  other.ts.tv_sec ||
        (ts.tv_sec == other.ts.tv_sec && ts.tv_nsec < other.ts.tv_nsec)) {
        return true;
    } else {
        return false;
    }
}

bool TimeSpec::operator>(const TimeSpec& other) const {
    if ( ts.tv_sec >  other.ts.tv_sec ||
        (ts.tv_sec == other.ts.tv_sec && ts.tv_nsec > other.ts.tv_nsec)) {
        return true;
    } else {
        return false;
    }
}

TimeSpec TimeSpec::operator+(const TimeSpec& other) const {
    const timespec ret = {ts.tv_sec + other.ts.tv_sec + (ts.tv_nsec + other.ts.tv_nsec) / secInNsec,
                          (ts.tv_nsec + other.ts.tv_nsec) % secInNsec};
    return TimeSpec(ret);
}
  
TimeSpec& TimeSpec::operator+=(const TimeSpec& other) {
    ts.tv_sec += other.ts.tv_sec + (ts.tv_nsec + other.ts.tv_nsec) / secInNsec;
    ts.tv_nsec = (ts.tv_nsec + other.ts.tv_nsec) % secInNsec;
    return *this;
}

TimeSpec TimeSpec::operator-(const TimeSpec& other) const {
    if (ts.tv_nsec < other.ts.tv_nsec) {
        const timespec ret = {ts.tv_sec - (other.ts.tv_sec + 1),
                              secInNsec + ts.tv_nsec - other.ts.tv_nsec};
        assert(ret.tv_sec >= 0);
        return TimeSpec(ret);
    } else {
        const timespec ret = {ts.tv_sec  - other.ts.tv_sec,
                              ts.tv_nsec - other.ts.tv_nsec};
        assert(ret.tv_sec >= 0);
        return TimeSpec(ret);
    }
}

TimeSpec& TimeSpec::operator-=(const TimeSpec& other) {
    if (ts.tv_nsec < other.ts.tv_nsec) {
        ts.tv_sec  -= other.ts.tv_sec + 1;
        ts.tv_nsec += secInNsec - other.ts.tv_nsec;
    } else {
        ts.tv_sec  -= other.ts.tv_sec;
        ts.tv_nsec -= other.ts.tv_nsec;
    }
    assert(ts.tv_sec >= 0);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const TimeSpec& ts) {
    os << ts.ts.tv_sec << "." << std::setfill('0') << std::setw(9) << ts.ts.tv_nsec;
    return os;
}
