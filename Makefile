CXX=g++
CXXFLAGS=-O2 -g0 -Wall -std=c++11
LDFLAGS=-s -lboost_serialization

PREFIX=/usr
BIN_DIR=$(PREFIX)/bin

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

install : hfs2tar
	install -d $(DESTDIR)$(BIN_DIR)
	install -m 0755 hfs2tar $(DESTDIR)$(BIN_DIR)

clean :
	rm -f *.o
	rm -f *.d
	rm -f hfs2tar

