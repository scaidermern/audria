#ifndef PROC_CACHE_H
#define PROC_CACHE_H PROC_CACHE_H

#include <cstdint>
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

    bool     isEmpty;
    uint64_t userTimeJiffies;
    uint64_t systemTimeJiffies;
    uint64_t startTimeJiffies;
    double   runTimeSecs;
    uint64_t totReadBytes;
    uint64_t totReadBytesStorage;
    uint64_t totWrittenBytes;
    uint64_t totWrittenBytesStorage;
    uint64_t totReadCalls;
    uint64_t totWriteCalls;
};

#endif // PROC_CACHE_H
