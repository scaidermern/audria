#ifndef PROC_CACHE_H
#define PROC_CACHE_H PROC_CACHE_H

#include <string>
#include <vector>

typedef std::vector<std::string> ProcessStatus;

/// cached values from a @ref processStatus in order to
/// reduce expensive string-number-string conversions
class Cache {
  public:
    /// creates an empty cache
    Cache();
    
    /// creates a cache from a @ref processStatus
    /// @note @ref runTimeSecs has to be filled later
    Cache(const ProcessStatus &status);
    
    bool               isEmpty;
    unsigned long long userTimeJiffies;
    unsigned long long systemTimeJiffies;
    unsigned long long startTimeJiffies;
    double             runTimeSecs;
    unsigned long long totReadBytes;
    unsigned long long totReadBytesStorage;
    unsigned long long totWrittenBytes;
    unsigned long long totWrittenBytesStorage;
    unsigned long long totReadCalls;
    unsigned long long totWriteCalls;
};

#endif // PROC_CACHE_H
