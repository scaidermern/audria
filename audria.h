#ifndef AUDRIA_H
#define AUDRIA_H AUDRIA_H

#include "helper.h"
#include "ProcCache.h"
#include "TimeSpec.h"

#include <map>
#include <string>

// clock source for clock_gettime()
#if defined(__linux) || defined(__linux__) || defined(linux)
// note: Normally we should use CLOCK_MONOTONIC_RAW on Linux because CLOCK_MONOTONIC is monotonic but
//       _not_ steady as it can be influenced by NTP. However, clock_nanosleep() doesn't support
//       CLOCK_MONOTONIC_RAW. Specifying an absolute time obtained by CLOCK_MONOTONIC_RAW will lead
//       to hickups if both clocks differ, so we have to use the non-steady CLOCK_MONOTONIC instead ;/
static const int clockSource = CLOCK_MONOTONIC; // Linux-specific
#else
static const int clockSource = CLOCK_MONOTONIC;
#endif

class Process;
typedef std::map<std::string, Process> ProcessMap;

class Process {
  public:
    Process(const std::string& processID) : pid(processID), status(), oldStatusCache(), oldStatusTS() {}
    /// returns whether the process still exists
    bool exists() const { return dirExists("/proc/" + pid); }

    std::string    pid;
    ProcessStatus  status;
    Cache          oldStatusCache;
    TimeSpec       oldStatusTS;
};

#endif // AUDRIA_H
