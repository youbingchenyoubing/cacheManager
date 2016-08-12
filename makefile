DEFINE=__CACHE_DEBUG
CXX=g++
CXXFLAGS+=-g -Wall -Wextra -std=c++0x -lpthread -D$(DEFINE) -O0 
NGX_ROOT=/usr/local/src/nginx-1.11.1

TARGETS=cacheManger
OBJ=main.o  similarlinkedhashmap.o entrance.o  parsexml.o parsehash.o writeDisk.o bitmap.o
CLEANUP=rm -f $(TARGETS) *.o
all:$(TARGETS)

clean:
	$(CLEANUP)

$(TARGETS):$(OBJ)
	$(CXX) $(CXXFLAGS)   $^ -o $@
main.o:main.cpp entrance.h 
	$(CXX) $(CXXFLAGS)   -c $< 
entrance.o:entrance.cpp entrance.h  
	$(CXX) $(CXXFLAGS)  -c $<
similarlinkedhashmap.o:similarlinkedhashmap.cpp similarlinkedhashmap.h
	$(CXX) $(CXXFLAGS)  -c $<
parsexml.o:parsexml.cpp parsexml.h
	$(CXX) $(CXXFLAGS)  -c $<
parsehash.o:parsehash.cpp parsehash.h
	$(CXX) $(CXXFLAGS)  -c $<
writeDisk.o:writeDisk.cpp writeDisk.h
	$(CXX) $(CXXFLAGS)  -c $<
bitmap.o:bitmap.cpp bitmap.h
	$(CXX) $(CXXFLAGS)  -c $<
clean:
	rm  *o



