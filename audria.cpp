/*      audria.cpp
 *
 *      Copyright 2012 Alexander Heinlein <alexander.heinlein@web.de>
 *
 *      audria - A Utility for Detailed Ressource Inspection of Applications
 *
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 *
 *
 *  references:
 *   - http://www.kernel.org/doc/Documentation/filesystems/proc.txt
 *   - http://www.kernel.org/doc/man-pages/online/pages/man5/proc.5.html
 *   - http://procps.sourceforge.net/
 * 
 *  @todo:
 *   - syscalls?
 *   - per process net I/O (only works by capturing packets via pcap ;/)
 *   - check kernel version? needed when checking each file separately?
 *   - option to show userland threads?
 *
 */

#include "audria.h"
#include "helper.h"
#include "definitions.h"
#include "ProcReader.h"
#include "ProcCache.h"
#include "TimeSpec.h"

#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <errno.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

/// checks if all values in the current cache seem reasonable, just for debugging
void checkCacheConsistency(const Cache& curCache, const Cache& oldCache) {
	if (oldCache.isEmpty) return;
    
    (void)curCache; // skip warning in release build
    assert(curCache.userTimeJiffies >= oldCache.userTimeJiffies || curCache.userTimeJiffies == 0);
    assert(curCache.systemTimeJiffies >= oldCache.systemTimeJiffies || curCache.systemTimeJiffies == 0);
    assert(curCache.startTimeJiffies >= oldCache.startTimeJiffies || curCache.startTimeJiffies == 0);
    assert(curCache.runTimeSecs >= oldCache.runTimeSecs || curCache.runTimeSecs == 0);
    assert(curCache.totReadBytes >= oldCache.totReadBytes || curCache.totReadBytes == 0);
    assert(curCache.totReadBytesStorage >= oldCache.totReadBytesStorage || curCache.totReadBytesStorage == 0);
    assert(curCache.totWrittenBytes >= oldCache.totWrittenBytes || curCache.totWrittenBytes == 0);
    assert(curCache.totWrittenBytesStorage >= oldCache.totWrittenBytesStorage || curCache.totWrittenBytesStorage == 0);
    assert(curCache.totReadCalls >= oldCache.totReadCalls || curCache.totReadCalls == 0);
    assert(curCache.totWriteCalls >= oldCache.totWriteCalls || curCache.totWriteCalls == 0);
}

/// parses status column fields from a string and returns the corresponding internal IDs as a set
/// @note: in case of errors, an empty set is returned
std::set<int> parseFieldsFromString(const std::string& str) {
    std::stringstream sstream(str);
    std::string field;
    std::set<int> fields;
    while (std::getline(sstream, field, ',')) {
        bool fieldValid = false;
        for (int statusColumnID = 0; statusColumnID < StatusColumnCount; ++statusColumnID) {
            if (statusColumnHeader[statusColumnID] == field) {
                fields.insert(statusColumnID);
                fieldValid = true;
                break;
            }
        }

        if (!fieldValid) {
            return std::set<int>();
        }
    }

    return fields;
}

void printUsage(const std::string& name) {
    std::cout << "Usage: " << name << " [OPTIONS] PID(s)" << std::endl
              << "  -a        monitor all processes" << std::endl
              << "  -d delay  delay in seconds between intervals (default: 0.5), specify '-1' to use" << std::endl
              << "            2 * kernel clock tick rate (2 * 1/100 on most systems), note: values equal or below"<< std::endl
              << "            the kernel clock tick rate will lead to bogus values for the 'CurCPUPerc' field" << std::endl
              << "  -e cmd    program to execute and watch, all remaining arguments will be forwarded" << std::endl
              << "  -f fields names of fields to show, separated by comma (default: all)" << std::endl
              << "  -k        show kernel threads (default: false)" << std::endl
              << "  -n num    number of iterations before quitting (default: unlimited)" << std::endl
              << "  -o file   file to write output to instead of stdout, will append to existing files," << std::endl
              << "            if file is '-' then output will be written to stdout (default)" << std::endl
              << "  -r        acquire real-time priority (lowest niceness, highest scheduling priority)," << std::endl
              << "            usually requires root privileges or the CAP_SYS_NICE capability" << std::endl
              << "  -s        include self in list of processes to monitor" << std::endl
              << "  -h        print this help and exit" << std::endl;
    return;
}

int main(int argc, char* argv[]) {
    // check if we have all column header
    assert(StatusColumnCount == sizeof(statusColumnHeader) / sizeof(statusColumnHeader[0]));
    
    if (argc < 2) {
        std::cerr << argv[0] << ": no arguments specified" << std::endl;
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // default argument values
    bool monitorAll  = false;
    bool monitorOwn  = false;
    bool monitorKThreads = false;
    bool rtPriority  = false;
    double delaySecs = 0.5;
    int iterations   = 0;
    std::set<int> fields;
    std::vector<char*> executeCmd;
    std::ofstream logFile;
    
    // parse command line arguments
    // note: on errors we try to mimic getopt()'s error message as they have a funny style
    int c;
    while ((c = getopt(argc, argv, "ad:e:f:kn:o:rsh")) != -1) {
        switch (c) {
            case 'a':
                monitorAll = true;
                break;
            case 'd':
                if (std::string(optarg) == "-1") {
                    delaySecs = 2 / (double)getHertz();
                } else if (isNumber(optarg)) {
                    delaySecs = stringToNumber<double>(optarg);
                    if (delaySecs < 0.0) {
                        std::cerr << argv[0] << ": option requires a positive number or -1 as argument -- '" << (char)c << "'" << std::endl;
                        printUsage(argv[0]);
                        exit(EXIT_FAILURE);
                    }
                } else {
                    std::cerr << argv[0] << ": option requires a number as argument -- '" << (char)c << "'" << std::endl;
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'e':
                executeCmd.push_back(optarg);
                // gather all remaining arguments
                for (; optind < argc; ++optind) {
                    executeCmd.push_back(argv[optind]);
                }
                break;
            case 'f':
                fields = parseFieldsFromString(optarg);
                if (fields.empty()) {
                    std::cerr << argv[0] << ": could not parse all given fields" << std::endl;
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'k':
                monitorKThreads = true;
                break;
            case 'n':
                if (isNumber(optarg)) {
                    iterations = stringToNumber<int>(optarg);
                    if (delaySecs < 0) {
                        std::cerr << argv[0] << ": option requires a positive number as argument -- '" << (char)c << "'" << std::endl;
                        printUsage(argv[0]);
                        exit(EXIT_FAILURE);
                    }
                } else {
                    std::cerr << argv[0] << ": option requires a number as argument -- '" << (char)c << "'" << std::endl;
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'o':
                if (std::string(optarg) != "-") {
                    logFile.open(optarg, std::ios::app);
                    if (!logFile) {
                        std::cerr << argv[0] << ": could not open file '" << optarg << "'' for appending: " << strerror(errno) << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }
                break;
            case 'r':
                rtPriority = true;
                break;
            case 's':
                monitorOwn = true;
                break;
            case 'h':
                printUsage(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case ':':
            case '?':
            default:
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    // check if specified delay is valid
    if (delaySecs <= 1 / (double)getHertz() &&
        (fields.empty() || fields.count(CurCPUPerc) == 1)) {
        std::cerr << "warning: interval " << delaySecs << " equal or below "
                  << "kernel tick rate (" << (1.0 / (double)getHertz()) << "), "
                  << "expect bogus values for the 'CurCPUPerc' field" << std::endl;
    } else if (fmod(getHertz(), (1.0 / (double) delaySecs)) != 0 &&
               (fields.empty() || fields.count(CurCPUPerc) == 1)) {
        std::cerr << "warning: iterations per second (" << (1.0 / delaySecs) << ") not a multiple of "
                  << "kernel ticks per second (" << (double)getHertz() << "), "
                  << "expect bogus values for the 'CurCPUPerc' field" << std::endl;
    }

    // output device
    std::ostream& log = logFile.is_open() ? logFile : std::cout;
    
    // add self if requested
    ProcessMap processes;
    if (monitorOwn) {
        const std::string& ownPID = numberToString(getpid());
        processes.insert(std::make_pair(ownPID, Process(ownPID)));
    }

    // all remaining arguments have to be PIDs
    for (; optind < argc; ++optind) {
        const std::string pid(argv[optind]);
        if (isNumber(pid)) {
            std::string fileName = "/proc/" + pid + "/";

            // check if PID exists
            if (dirExists(fileName)) {
                processes.insert(std::make_pair(pid, Process(pid)));
            } else {
                std::cerr << "cannot watch PID, could not open " << fileName << ": " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            std::cerr << "argument is not a PID -- '" << pid << "'" << std::endl;
            printUsage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // execute command if specified
    pid_t childPid = -1;
    if (!executeCmd.empty()) {
        childPid = fork();
        if (childPid == -1) {
            std::cerr << "fork failed: " << strerror(errno) << std::endl;
        } else if (childPid == 0) {
            // we are the child, execute specified command
            executeCmd.push_back(NULL); // last argument must be NULL
            if (execvp(executeCmd[0], &executeCmd[0]) == -1) {
                std::cerr << "could not execute specified command: " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            // we are the parent, add executed command to watch list
            std::cerr << "successfully spawned child " << childPid << std::endl;
            const std::string& childPidStr = numberToString(childPid);
            processes.insert(std::make_pair(childPidStr, Process(childPidStr)));
        }
    }
    
    if (processes.empty() && !monitorAll) {
        std::cerr << "no PID(s) specified" << std::endl;
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    // acquire rt priority if requested
    if (rtPriority) {
        if (setpriority(0, PRIO_PROCESS, PRIO_MIN) == -1) {
            std::cerr << "could not set minimum niceness (" << PRIO_MIN << "): " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }

        struct sched_param schedParam;
        schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO);
        if (schedParam.sched_priority == -1) {
            std::cerr << "could not determine highest scheduling priority: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }

        if (sched_setscheduler(0, SCHED_FIFO, &schedParam) == -1) {
            std::cerr << "could not acquire highest scheduling priority (" << schedParam.sched_priority << "): " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // print column headers
    log << "Time";
    for (int statusColumn = 0; statusColumn < StatusColumnCount; ++statusColumn) {
        // skip unwanted fields
        if (!fields.empty() && fields.count(statusColumn) == 0) continue;
        log << "," << statusColumnHeader[statusColumn];
    }
    log << std::endl;
    
    const TimeSpec intervalTS(delaySecs);
    TimeSpec wakeupTS;
    clock_gettime(clockSource, &wakeupTS.ts);
    
    int i = 0;
    while (iterations == 0 || ++i <= iterations) {
        // check if process to execute is still running
        if (childPid != -1 && !waitpid(childPid, 0, WNOHANG) == 0) {
            std::cerr << "child " << childPid << " terminated, exiting" << std::endl;
            exit(EXIT_SUCCESS);
        }

        // check if all processes still exist, remove terminated ones
        for (ProcessMap::iterator processIt = processes.begin(); processIt != processes.end(); ) {
            if (!processIt->second.exists()) {
                processes.erase(processIt++);
            } else {
                ++processIt;
            }
        }

        // check if there are new processes
        if (monitorAll) {
            const PIDSet& pidSet = ProcReader::pids();

            for (PIDSet::const_iterator it = pidSet.begin(); it != pidSet.end(); ++it) {
                if (processes.count(*it) == 0) {
                    processes.insert(std::make_pair(*it, Process(*it)));
                }
            }
        }

        if (unlikely(processes.empty())) {
            std::cerr << "no more processes to watch, exiting" << std::endl;
            exit(EXIT_SUCCESS);
        }

        for (ProcessMap::iterator processIt = processes.begin(); processIt != processes.end(); ++processIt) {
			TimeSpec curTS;
			clock_gettime(clockSource, &curTS.ts);
            Process& process = processIt->second;
            const TimeSpec& elapsedTS = curTS - process.oldStatusTS;
			
            ProcReader pr(process.pid);
			pr.readAll();

            if (!monitorKThreads && pr.isKernelThread()) {
                continue;
            }
			
			pr.updateCache();
			
            pr.calcAll(process.oldStatusCache, elapsedTS.seconds());
			
			const Cache& curCache = pr.getCache();
            checkCacheConsistency(curCache, process.oldStatusCache);
			
            const ProcessStatus& curStatus = pr.getProcessStatus();
				  
            log << curTS;
            for (unsigned int statusColumn = 0; statusColumn < curStatus.size(); ++statusColumn) {
                // skip unwanted fields
                if (!fields.empty() && fields.count(statusColumn) == 0) continue;

                // if printing a program name containing a comma, enclose it in double-quotes (rfc4180 section 2.6)
                if (unlikely(statusColumn == Name) &&
                    unlikely(curStatus[statusColumn].find(",") != std::string::npos)) {
                    log << ",\"" << curStatus[statusColumn] << "\"";
                } else {
                    log << "," << curStatus[statusColumn];
                }
            }
            log << std::endl;
			
            process.oldStatusCache = curCache;
            process.oldStatusTS    = curTS;
		}
		
        if (delaySecs != 0.0) {
            TimeSpec curTS;
            clock_gettime(clockSource, &curTS.ts);
            
            // calculate next wakeup time and make sure it is in the future
            wakeupTS += intervalTS;
            if (curTS > wakeupTS) {
                const TimeSpec diffTS = curTS - wakeupTS;
                unsigned int toSkip = 0;
                do {
                    wakeupTS += intervalTS;
                    ++toSkip;
                } while (curTS > wakeupTS);
                std::cerr << "warning: interval too high, cannot keep up!"
                          << " (" << diffTS << " seconds behind,"
                          << " skipping " << toSkip << " iterations)" << std::endl;
            }
            
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTS.ts, NULL);
        }
	}
    
    return 0;
}
