CXX=g++
CXXFLAGS+=-g -Wall -Wextra -std=c++0x -lpthread
NGX_ROOT=/usr/local/src/nginx-1.11.1

TARGETS=cacheManger
OBJ=main.o  similarlinkedhashmap.o entrance.o  parsexml.o parsehash.o writeDisk.o bitmap.o
CLEANUP=rm -f $(TARGETS) *.o
all:$(TARGETS)

clean:
	$(CLEANUP)
CORE_INCS= -I. \
-I$(NGX_ROOT)/src/core \
-I$(NGX_ROOT)/src/event \
-I$(NGX_ROOT)/src/event/modules \
-I$(NGX_ROOT)/src/os/unix \
-I$(NGX_ROOT)/objs \


$(TARGETS):$(OBJ)
	$(CXX) $(CXXFLAGS) $(CORE_INCS)  $^ -o $@
main.o:main.cpp entrance.h 
	$(CXX) $(CXXFLAGS) $(CORE_INCS)  -c $< 
entrance.o:entrance.cpp entrance.h  
	$(CXX) $(CXXFLAGS) $(CORE_INCS) -c $<
similarlinkedhashmap.o:similarlinkedhashmap.cpp similarlinkedhashmap.h
	$(CXX) $(CXXFLAGS) $(CORE_INCS) -c $<
parsexml.o:parsexml.cpp parsexml.h
	$(CXX) $(CXXFLAGS) $(CORE_INCS) -c $<
parsehash.o:parsehash.cpp parsehash.h
	$(CXX) $(CXXFLAGS) $(CORE_INCS) -c $<
writeDisk.o:writeDisk.cpp writeDisk.h
	$(CXX) $(CXXFLAGS) $(CORE_INCS) -c $<
bitmap.o:bitmap.cpp bitmap.h
	$(CXX) $(CXXFLAGS) $(CORE_INCS) -c $<
clean:
	rm  *o



