#The MIT License (MIT)
#

CC=g++

CPPFLAGS += -I.

CFLAGS += -Wall -Wextra
CFLAGS_FAST = $(CFLAGS) -O0 -g $(CFLAGS_DEBUG_EXTRA)
CFLAGS_LIB = $(CFLAGS_FAST) -fPIC
LDFLAGS= -lpthread

SOURCES=DtmfDetector.cpp \
	DtmfGenerator.cpp \
	main.cpp
        
OBJECTS=$(SOURCES:.c=.o)

all: dtmf

dtmf: $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@ 

.c.o: $(SOURCES) Makefile
	$(CC) $(CPPFLAGS_FAST) $(CFLAGS_LIB) -c $< -o $@

clean:
	rm -f *.o *.a dtmf

.PHONY: clean all dtmf
