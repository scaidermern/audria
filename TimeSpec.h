#ifndef TIME_SPEC_H
#define TIME_SPEC_H TIME_SPEC_H

#include <iostream>

/// wrapper around timespec struct providing some helpful functions
/// @note will not handle negative times correctly
class TimeSpec {
  public:
    timespec ts; ///< actual timespec struct
    
    static const long secInNsec; ///< helper variable, seconds in nanoseconds
  
    /// zero initialization
    TimeSpec() : ts() {}
    /// initialization from seconds and nanoseconds
    TimeSpec(const long secs, const long nsecs);
    /// initialization from fractional seconds
    TimeSpec(const double secs);
    /// initialization from usual timespec struct
    TimeSpec(const timespec& otherTS) : ts(otherTS) {}
    
    /// returns fractional seconds
    double seconds() const;
    
    /// returns the seconds part of the timespec struct, shorthand for ts.tv_sec
    inline long sec() { return ts.tv_sec; }
    
    /// returns the nanoseconds part of the timespec struct, shorthand for ts.tv_nsec
    inline long nsec() { return ts.tv_nsec; }
    
    bool operator==(const TimeSpec& other) const;
    
    bool operator<(const TimeSpec& other) const;
    
    bool operator>(const TimeSpec& other) const;
    
    TimeSpec operator+(const TimeSpec& other) const;
      
    TimeSpec& operator+=(const TimeSpec& other);
    
    /// @todo cannot handle negative results
    TimeSpec operator-(const TimeSpec& other) const;
    
    /// @todo cannot handle negative results
    TimeSpec& operator-=(const TimeSpec& other);
};

/// allows us nice printing
std::ostream& operator<<(std::ostream& os, const TimeSpec& ts);

#endif // TIME_SPEC_H
