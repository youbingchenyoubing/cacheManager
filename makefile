DEFINE=__CACHE_DEBUG
CXX=g++
CXXFLAGS+=-g -Wall -Wextra -std=c++0x -lpthread -D$(DEFINE) -O0 
NGX_ROOT=/usr/local/src/nginx-1.11.1

TARGETS=cacheManger

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


$(TARGETS):main.o  similarlinkedhashmap.o entrance.o  parsexml.o 
	$(CXX) $(CXXFLAGS) $(CORE_INCS)  $^ -o $@
main.o:main.cpp entrance.h 
	$(CXX) $(CXXFLAGS) $(CORE_INCS)  -c $< 
entrance.o:entrance.cpp entrance.h  
	$(CXX) $(CXXFLAGS) $(CORE_INCS) -c $<
similarlinkedhashmap.o:similarlinkedhashmap.cpp similarlinkedhashmap.h
	$(CXX) $(CXXFLAGS) $(CORE_INCS) -c $<
parsexml.o:parsexml.cpp parsexml.h
	$(CXX) $(CXXFLAGS) $(CORE_INCS) -c $<
clean:
	rm  *o



