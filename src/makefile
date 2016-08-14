DEFINE=__CACHE_DEBUG
CXX=g++
CXXFLAGS+=-g -Wall -Wextra -std=c++0x -lpthread -D$(DEFINE) -O0 
NGX_ROOT=/usr/local/src/nginx-1.11.1
BUILD_DIR=./build/
BIN_DIR=./bin/
TARGETS=cacheManger
OBJ=main.o  similarlinkedhashmap.o entrance.o  parsexml.o parsehash.o writeDisk.o bitmap.o
CLEANUP=rm -f $(BIN_DIR)$(TARGETS) $(BUILD_DI)*.o

all:$(TARGETS)

clean:
	$(CLEANUP)

$(TARGETS):$(OBJ)
	$(CXX) $(CXXFLAGS)  $(BUILD_DIR)*.o -o $(BIN_DIR)$@
main.o:main.cpp entrance.h 
	$(CXX) $(CXXFLAGS)   -c $< -o $(BUILD_DIR)$@
entrance.o:entrance.cpp entrance.h  
	$(CXX) $(CXXFLAGS)  -c $<  -o $(BUILD_DIR)$@
similarlinkedhashmap.o:similarlinkedhashmap.cpp similarlinkedhashmap.h
	$(CXX) $(CXXFLAGS)  -c $<  -o $(BUILD_DIR)$@
parsexml.o:parsexml.cpp parsexml.h
	$(CXX) $(CXXFLAGS)  -c $<  -o $(BUILD_DIR)$@
parsehash.o:parsehash.cpp parsehash.h
	$(CXX) $(CXXFLAGS)  -c $<  -o $(BUILD_DIR)$@
writeDisk.o:writeDisk.cpp writeDisk.h
	$(CXX) $(CXXFLAGS)  -c $<  -o $(BUILD_DIR)$@
bitmap.o:bitmap.cpp bitmap.h
	$(CXX) $(CXXFLAGS)  -c $<  -o $(BUILD_DIR)$@



