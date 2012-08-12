#include "ProcReader.h"
#include "helper.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>

#include <dirent.h>
#include <errno.h>

ProcReader::ProcReader(const std::string& processID) :
  pid(processID), hasRead(false), status(StatusColumnCount, "0.0"),
  cache(), canReadStat(false), canReadStatus(false), canReadIO(false) {
    // perform some checks
    if (!dirExists("/proc/" + pid)) {
        return;
    }
    
    canReadStat   = fileReadable("/proc/" + pid + "/stat");
    canReadStatus = fileReadable("/proc/" + pid + "/status");
    canReadIO     = fileReadable("/proc/" + pid + "/io");
    
    assert(canReadStat);   // can this fail? where/when?
    assert(canReadStatus); // can this fail? where/when?
}

void ProcReader::readAll() {
    readProcessStat();
    readProcessStatus();
    readProcessIO();
}

void ProcReader::readProcessStat() {
	assert(status.size() == StatusColumnCount);
    
    if (!canReadStat)
        return;
	
    const std::string fileName = "/proc/" + pid + "/stat";
    std::ifstream file(fileName.c_str(), std::ifstream::in);
    if (!file.good()) {
        canReadStat = false;  // process may already have been terminated
		return;
	}
	
    std::string line;
    std::getline(file, line);
    assert(!line.empty());
    
    std::stringstream sstr(line);
    
    // read first field
    sstr >> status[PID];
    
    // second field is the executable name in brackets, may contain spaces and other bad characters
    // example name from readproc.c ":-) 1 2 3 4 5 6" -> reverse search for closing bracket ')'
    const std::string& tmpStr = sstr.str();
    const size_t cmdStart = tmpStr.find("(") + 1;
    const size_t cmdEnd   = tmpStr.rfind(")");
    status[Name] = tmpStr.substr(cmdStart, cmdEnd - cmdStart);
    assert(!status[Name].empty());
    
    // skip first two fields
    sstr.seekg(cmdEnd + 1);
    
    // remaining fields can be parsed easily as there are no unexpected spaces
    std::string skip;
    sstr >> status[State] >> status[PPID]
         >> skip >> skip >> skip >> skip >> skip
         >> status[MinFlt] >> skip >> status[MajFlt] >> skip
         >> status[UserTimeJiffies] >> status[SystemTimeJiffies] >> skip >> skip
         >> status[Priority] >> status[Nice] >> status[Threads]
         >> skip >> status[StartTimeJiffies];
    
    hasRead = true;
}

void ProcReader::readProcessStatus() {
	assert(status.size() == StatusColumnCount);
    
    if (!canReadStatus)
        return;
	
    const std::string fileName = "/proc/" + pid + "/status";
    std::ifstream file(fileName.c_str(), std::ifstream::in);
    if (!file.good()) {
        canReadStatus = false;  // process may already have been terminated
		return;
	}
    
    std::string line;
    while (std::getline(file, line)) {
		assert(!line.empty());
        std::stringstream sstr(line);
        std::string name, value;
        sstr >> name >> value;
        assert(!name.empty());
        assert(!value.empty() || name == "Groups:");
        
        if        (name == "VmPeak:") {
            status[VmPeakkB] = value;
        } else if (name == "VmSize:") {
            status[VmSizekB] = value;
        } else if (name == "VmLck:") {
            status[VmLckkB] = value;
        } else if (name == "VmHWM:") {
            status[VmHWMkB] = value;
        } else if (name == "VmRSS:") {
            status[VmRSSkB] = value;
        } else if (name == "VmSwap:") {
            status[VmSwapkB] = value;
        }
    }
    
    hasRead = true;
}

void ProcReader::readProcessIO() {
	assert(status.size() == StatusColumnCount);
    
    if (!canReadIO)
        return;
	
    const std::string fileName = "/proc/" + pid + "/io";
    std::ifstream file(fileName.c_str(), std::ifstream::in);
    if (!file.good()) {
        canReadIO = false;  // process may already have been terminated
        return;
	}
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty())
            continue; // happens sometimes when self-profiling for some mysterious reason
        
        std::stringstream sstr(line);
        assert(sstr);
        std::string name, value;
        sstr >> name >> value;
        assert(sstr);
        assert(!name.empty());
        assert(!value.empty());
        
        if        (name == "rchar:") {
            status[TotReadBytes] = value;
        } else if (name == "wchar:") {
            status[TotWrittenBytes] = value;
        } else if (name == "read_bytes:") {
            status[TotReadBytesStorage] = value;
        } else if (name == "write_bytes:") {
            status[TotWrittenBytesStorage] = value;
        } else if (name == "syscr:") {
            status[TotReadCalls] = value;
        } else if (name == "syscw:") {
            status[TotWriteCalls] = value;
        }
    }
    
    hasRead = true;
}

void ProcReader::updateCache() {
    assert(cache.isEmpty);
    cache = Cache(status);
}

void ProcReader::calcAll(const Cache& oldCache, const double elapsedSecs) {
    if (cache.isEmpty) {
        assert(false);
        return;
    }

    if (!hasRead) {
        return; // process may already have been terminated
    }
    
    calcRuntime();
    calcUserSystemTimes();
    calcCPUUtilization(oldCache, elapsedSecs);
    calcIOUtilization(oldCache, elapsedSecs);
}

void ProcReader::calcRuntime() {
    if (cache.isEmpty) {
        assert(false);
        return;
    }
    
    const double systemRuntimeSecs = uptime();
    const double processStarttimeSecs = cache.startTimeJiffies / (double)getHertz();
    
    if (systemRuntimeSecs - processStarttimeSecs <= 0) {
        cache.runTimeSecs = 0; // may happen with really short intervals shortly after process startup
    } else {
        cache.runTimeSecs = systemRuntimeSecs - processStarttimeSecs;
    }
    status[RunTimeSecs] = numberToString(cache.runTimeSecs); // required for output
}

void ProcReader::calcUserSystemTimes() {
    if (cache.isEmpty) {
        assert(false);
        return;
    }
    
    const int totProcessCPUTimeJiffies = cache.userTimeJiffies + cache.systemTimeJiffies;
    
    status[UserTimePerc]   = numberToString(cache.userTimeJiffies   * 100.0 / (double)totProcessCPUTimeJiffies);
    status[SystemTimePerc] = numberToString(cache.systemTimeJiffies * 100.0 / (double)totProcessCPUTimeJiffies);
}

void ProcReader::calcCPUUtilization(const Cache& oldCache, const double elapsedSecs) {
    if (cache.isEmpty) {
        assert(false);
        return;
    }
    
    if (cache.runTimeSecs == 0) { // really short interval and shortly after process startup
        return;
    }

    const double totProcessCPUTimeSecs = (cache.userTimeJiffies + cache.systemTimeJiffies) / (double)getHertz();
    status[AvgCPUPerc] = numberToString((totProcessCPUTimeSecs * 100.0) / cache.runTimeSecs);
    
    if (oldCache.isEmpty) { // first iteration, cannot calculate current CPU
        return;
    }
    
    const double oldTotProcessCPUTimeSecs = (oldCache.userTimeJiffies + oldCache.systemTimeJiffies) / (double)getHertz();
    const double elapsedCPUTimeSecs = totProcessCPUTimeSecs - oldTotProcessCPUTimeSecs;
    assert(elapsedCPUTimeSecs >= 0);
    status[CurCPUPerc] = numberToString((elapsedCPUTimeSecs * 100.0) / elapsedSecs);
}

void ProcReader::calcIOUtilization(const Cache& oldCache, const double elapsedSecs) {
    if (cache.isEmpty) {
        assert(false);
        return;
    }
    
    if (cache.runTimeSecs == 0) { // really short interval and shortly after process startup
        return;
    }
    
    if (oldCache.isEmpty) // first iteration, cannot calculate current IO
        return;
    
    status[CurReadBytes]           = numberToString((cache.totReadBytes - oldCache.totReadBytes) / elapsedSecs);
    status[CurWrittenBytes]        = numberToString((cache.totWrittenBytes - oldCache.totWrittenBytes) / elapsedSecs);
    status[CurReadBytesStorage]    = numberToString((cache.totReadBytesStorage - oldCache.totReadBytesStorage) / elapsedSecs);
    status[CurWrittenBytesStorage] = numberToString((cache.totWrittenBytesStorage - oldCache.totWrittenBytesStorage) / elapsedSecs);
}

PIDSet ProcReader::pids() {
    PIDSet pidSet;

    DIR* dir = opendir("/proc");
    if (!dir) {
        std::cerr << "could not open /proc:" << strerror(errno) << std::endl;
        assert(false);
        return pidSet;
    }

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        assert(entry->d_name);
        // first character is a number? -> name is a PID
        if (std::isdigit(entry->d_name[0])) {
            pidSet.insert(entry->d_name);
        }
    }

    closedir(dir);
    return pidSet;
}
