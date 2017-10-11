# to compile to do program coverage tests, add these to CXXFLAGS:
# -fprofile-arcs -ftest-coverage

CXX = g++ 
#CXXFLAGS = -g -Wall -O3
CXXFLAGS = -g -Wall
#LIBS = -L/usr/local/lib -L/home/jgsherw/Libraries/boost_1_60_0 -lboost_filesystem -lboost_program_options -lboost_system
LIBS = -L/usr/local/lib -lboost_filesystem -lboost_program_options -lboost_system
INCLUDES = globals.h utils.h options.h resource.h properties.h device.h marketplace.h agent.h statstracker.h devmarketplace.h logging.h
SOURCES = main.cpp utils.cpp options.cpp resource.cpp properties.cpp device.cpp marketplace.cpp globals.cpp agent.cpp statstracker.cpp devmarketplace.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = societies

MKDEP = $(CXX) -MM

.PHONY: all depend clean docs 

all:
	+$(MAKE) depend
	+$(MAKE) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

depend: .dep
.dep: Makefile $(SOURCES) $(INCLUDES)
	$(MKDEP) $(SOURCES) $(INCLUDES) > '$(@)'

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) .dep docs/*

docs: Doxyfile
	doxygen

-include .dep
