#include "helper.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <cassert>
#include <cstdlib>
#include <cstring>

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

bool dirExists(const std::string& dir) {
    assert(!dir.empty());
    
	struct stat st;
	stat(dir.c_str(), &st);
	return S_ISDIR(st.st_mode);
}

bool fileReadable(const std::string& path) {
    assert(!path.empty());
    
	return access(path.c_str(), R_OK) == 0;
}

bool isNumber(const std::string& str) {
    return !str.empty() && str.find_first_not_of("0123456789.-") == std::string::npos;
}

double uptime() {
    static const std::string fileName = "/proc/uptime";
    std::ifstream file(fileName.c_str(), std::ifstream::in);
    if (!file.good()) {
		std::cerr << "could not open " << fileName << ": " << strerror(errno) << std::endl;
        assert(false);
        return std::numeric_limits<double>::quiet_NaN();
	}
    
    std::string time;
    file >> time;
    assert(!time.empty());
    
    const double ret = stringToNumber<double>(time);
    assert(ret > 0.0);
    
    return ret;
}

long getHertz() {
    static long hertz = sysconf(_SC_CLK_TCK); // should not change during runtime
    assert(hertz > 0);
    return hertz;
}
