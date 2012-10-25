#ifndef PROC_READER_H
#define PROC_READER_H PROC_READER_H

#include "ProcCache.h"

#include <set>
#include <string>
#include <vector>

typedef enum {
    Name,                   ///< executable file name
    State,                  ///< process state
    PID,                    ///< process ID
    PPID,                   ///< parent process ID
    PGRP,                   ///< process group ID
    AvgCPUPerc,             ///< average CPU Utilization, in percent
    CurCPUPerc,             ///< current utilization since last iteration, in percent
    MinFlt,                 ///< minor page faults
    MajFlt,                 ///< major page faults
    UserTimeJiffies,        ///< total time spent in user mode, in jiffies
    SystemTimeJiffies,      ///< total time spent in kernel mode, in jiffies
    UserTimePerc,           ///< total time spent in user mode, in percent
    SystemTimePerc,         ///< total time spent in kernel mode, in percent
    Priority,               ///< process priority
    Nice,                   ///< process nice value
    Threads,                ///< number of threads
    StartTimeJiffies,       ///< start time, in jiffies
    RunTimeSecs,            ///< total runtime, in seconds
    VmPeakkB,               ///< peak virtual memory size, in kB
    VmSizekB,               ///< virtual (allocated) memory size, in kB
    VmLckkB,                ///< locked (non-swapable) memory size, in kB
    VmHWMkB,                ///< peak resident set size ("high water mark"), in kB
    VmRSSkB,                ///< resident set (touched memory currently in RAM) size, in kB
    VmSwapkB,               ///< swap usage, in kB
    TotReadBytes,           ///< total bytes read (incl. cached)
    CurReadBytes,           ///< current bytes read (incl. cached), per second
    TotReadBytesStorage,    ///< total bytes read (excl. cached)
    CurReadBytesStorage,    ///< current bytes read (excl. cached), per second
    TotWrittenBytes,        ///< total bytes written (incl. cached)
    CurWrittenBytes,        ///< current bytes written (incl. cached), per second
    TotWrittenBytesStorage, ///< total bytes written (excl. cached)
    CurWrittenBytesStorage, ///< current bytes written (excl. cached), per second
    TotReadCalls,           ///< total calls to read()/pread()
    CurReadCalls,           ///< current calls to read()/pread()
    TotWriteCalls,          ///< total calls to write()/pwrite()
    CurWriteCalls,          ///< current calls to write()/pwrite()
    StatusColumnCount
} StatusColumns;

const std::string statusColumnHeader[] = {
    "Name", "State", "PID", "PPID", "PGRP", "AvgCPUPerc", "CurCPUPerc", "MinFlt", "MajFlt",
    "UserTimeJiffies", "SystemTimeJiffies", "UserTimePerc", "SystemTimePerc", 
    "Priority", "Nice", "Threads", "StartTimeJiffies", "RunTimeSecs",
    "VmPeakkB", "VmSizekB", "VmLckkB", "VmHWMkB", "VmRsskB", "VmSwapkB",
    "TotReadBytes", "CurReadBytesPerSec", "TotReadBytesStorage", "CurReadBytesStoragePerSec",
    "TotWrittenBytes", "CurWrittenBytesPerSec", "TotWrittenBytesStorage", "CurWrittenBytesStoragePerSec",
    "TotReadCalls", "CurReadCalls", "TotWriteCalls", "CurWriteCalls"
};

/// stores all relevant data from /proc/pid/
typedef std::vector<std::string> ProcessStatus;
/// stores all current PIDs from /proc/
typedef std::set<std::string> PIDSet;

/// reads and processes various data from /proc/pid/
class ProcReader {
  public:
    /// constructs a ProcReader object for the given PID
    ProcReader(const std::string& processID);
    
    /// reads all interesting information from /proc,
    /// combines @ref readProcessStat(), @ref readProcessStatus() and @ref readProcessIO()
    void readAll();
    
    /// parses various information from /proc/pid/status
    void readProcessStat();
    
    /// parses memory-related information from /proc/pid/status
    /// @note: we parse the memory-related information from /proc/pid/status
    ///        instead of/proc/pid/stat because status has more information
    void readProcessStatus();
    
    /// parses IO information from /proc/pid/io
    void readProcessIO();
    
    /// updates data cache, has to be called before any of the calc functions
    /// @note don't call multiple times
    void updateCache();
    
    /// processes all read information,
    /// combines @ref calcRuntime(), @ref calcUserSystemTimes(),
    /// @ref calcCPUUtilization() and @ref calcIOUtilization()
    void calcAll(const Cache& oldCache, const double elapsedSecs);
    
    /// calculates total process runtime in seconds
    /// and fills @ref runTimeSecs in @p cache
    void calcRuntime();
    
    /// calculates user and system times in percent
    void calcUserSystemTimes();
    
    /// calculates average and current CPU usage
    void calcCPUUtilization(const Cache& oldCache, const double elapsedSecs);
    
    /// calculates current IO load
    void calcIOUtilization(const Cache& oldCache, const double elapsedSecs);

    /// returns whether this process is a kernel thread
    bool isKernelThread() const { return status[PGRP] == "0"; }
    
    /// returns data we have read and processed
    const ProcessStatus& getProcessStatus() const { return status; }
    
    /// returns internal data cache
    const Cache& getCache() const { return cache; }

    /// returns a set of all current PIDs
    static PIDSet pids();
    
  private:
    std::string    pid;     ///< PID to read, stored as string for performance reasons (requires no conversions)
    bool           hasRead; ///< stores if we have read any data from /proc at all
    ProcessStatus  status;  ///< data we have read and processed
    Cache          cache;   ///< cache for read data
    
    bool           canReadStat;    ///< can we read /proc/pid/stat?
    bool           canReadStatus;  ///< can we read /proc/pid/status?
    bool           canReadIO;      ///< can we read /proc/pid/io?
};

#endif // PROC_READER_H
