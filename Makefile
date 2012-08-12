#
# builds by default in debug mode
# call with mode=release to build in release mode
#

CXX = g++
CXXFLAGS = -Wall -Wextra -Weffc++ -Wshadow #-Wconversion
LDFLAGS = -lrt

ifeq ($(mode),release)
	CXXFLAGS += -Os -DNDEBUG
else
	CXXFLAGS += -O1 -g
endif

SRCS=audria.cpp ProcReader.cpp ProcCache.cpp TimeSpec.cpp helper.cpp
SRCSTEST=TimeSpecTest.cpp TimeSpec.cpp
OBJS=$(SRCS:.cpp=.o)
OBJSTEST=$(SRCSTEST:.cpp=.o)

.PHONY: all
all: info audria tests

# info message in which mode to build
info:
ifeq ($(mode),release)
	@echo "building in RELEASE mode\n"
else
	@echo "building in DEBUG mode\n"
endif

# our project
audria: $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) $(LDFLAGS) -o $@
ifeq ($(mode),release)
	strip $@
endif

# tests, don't build in release mode
tests: $(OBJSTEST)
ifneq ($(mode),release)
	$(CXX) $(OBJSTEST) $(CXXFLAGS) -g $(LDFLAGS) -o $@
endif

audria.o: audria.h
ProcReader.o: ProcReader.h
ProcCache.o: ProcCache.h
TimeSpec.o: TimeSpec.h
helper.o: helper.h

.PHONY: clean
clean:
	rm -f *.o audria tests
