#
# builds by default in release mode
# call with mode=debug to build in debug mode
#

CXX = g++
CXXFLAGS = -Wall -Wextra -Weffc++ -Wshadow #-Wconversion
LDFLAGS = -lrt

ifeq ($(mode),debug)
	CXXFLAGS += -O1 -g
else
	CXXFLAGS += -Os -DNDEBUG
endif

SRCS=audria.cpp ProcReader.cpp ProcCache.cpp TimeSpec.cpp helper.cpp
SRCSTEST=TimeSpecTest.cpp TimeSpec.cpp
OBJS=$(SRCS:.cpp=.o)
OBJSTEST=$(SRCSTEST:.cpp=.o)

.PHONY: all
all: info audria tests

# info message in which mode to build
info:
ifeq ($(mode),debug)
	@echo "building in DEBUG mode\n"
else
	@echo "building in RELEASE mode\n"
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
