CC=gcc
CXX=g++
CFLAGS=-O2 -Wall -g -I/usr/local/include
CXXFLAGS=
LDFLAGS=-L/usr/local/lib -lev

#SOURCES=$(shell find src/ -type f -name \*.c -or -name \*.cc -printf %P\\n)
SOURCES=\
	src/opendns/countd/client.cc \
	src/opendns/countd/commitlog.cc \
	src/opendns/countd/commitlog/file.cc \
	src/opendns/countd/message.cc \
	src/countd-commitlog.cc \
	src/countd-write.cc
OBJECTS=$(SOURCES:.cc=.o)

all: $(SOURCES) $(OBJECTS)
	$(CXX) \
		src/opendns/countd/commitlog.o \
		src/opendns/countd/commitlog/file.o \
		src/opendns/countd/message.o \
		src/countd-commitlog.o \
		$(LDFLAGS) -o bin/countd-commitlog
	$(CXX) \
		src/opendns/countd/client.o \
		src/opendns/countd/commitlog.o \
		src/opendns/countd/commitlog/file.o \
		src/opendns/countd/message.o \
		src/countd-write.o \
		$(LDFLAGS) -o bin/countd-write
#	$(CXX) TODO $(LDFLAGS) -o bin/countd-work
#	$(CXX) TODO $(LDFLAGS) -o bin/countd-read

.c.o:
	echo $(OBJECTS)
	$(CC) -c $(CFLAGS) $< -o $@

.cc.o:
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $< -o $@

clean:
	find src/ -type f -name \*.o -delete
	rm -f bin/countd-read bin/countd-write bin/countd-work

libev:
	wget http://dist.schmorp.de/libev/libev-3.8.tar.gz
	tar xf libev-3.8.tar.gz
	cd libev-3.8 && ./configure && make && sudo make install
	rm -rf libev-3.8 libev-3.8.tar.gz

.PHONY: all clean libev
