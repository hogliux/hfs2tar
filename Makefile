CXX=g++
CXXFLAGS=-O0 -g3 -Wall -std=c++11
LDFLAGS=-lboost_serialization

SRC=locale.cpp main.cpp fastunicodecompare.cpp
OBJ=$(SRC:.cpp=.o)
DEPS=$(SRC:.cpp=.d)

hfs2tar : $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

-include $(DEPS)

%.o : %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

%.d : %.cpp
	set -e; rm -f $@; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean :
	rm -f *.o
	rm -f *.d
	rm -f hfs2tar

