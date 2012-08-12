#ifndef AUDRIA_H
#define AUDRIA_H AUDRIA_H

#include "helper.h"
#include "ProcCache.h"
#include "TimeSpec.h"

#include <map>
#include <string>

// clock source for clock_gettime()
#if defined(__linux) || defined(__linux__) || defined(linux)
static const int clockSource = CLOCK_MONOTONIC_RAW; // Linux-specific
#else
static const int clockSource = CLOCK_MONOTONIC;
#endif

class Process;
typedef std::map<std::string, Process> ProcessMap;

class Process {
  public:
    Process(const std::string& processID) : pid(processID), status(), oldStatusCache(), oldStatusTS() {}
    /// returns wether the process still exists
    bool exists() const { return dirExists("/proc/" + pid); }

    std::string    pid;
    ProcessStatus  status;
    Cache          oldStatusCache;
    TimeSpec       oldStatusTS;
};

#endif // AUDRIA_H
