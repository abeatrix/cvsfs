# Makefile for cvsmnt mount replacement
#
# Author: Petric Frank
# Date:   June 14th, 2001

OBJECT_FILES =	main.o

RESULT_FILE =	cvsumount


CC = gcc
CXX = gcc
LD = ld
RM = rm -f

# CFLAGS = -O3 -Wall
CFLAGS = -Wall
CXXFLAGS =


all:		$(RESULT_FILE)


$(RESULT_FILE):	$(OBJECT_FILES)
	$(CC)	-o $(RESULT_FILE) $(CFLAGS) $(OBJECT_FILES)


.c.o:
	$(CC)	-c -o $@ $(CFLAGS) $<


clean:
	$(RM) *.o


distclean:
	$(RM) *.o
	$(RM) $(RESULT_FILE)


install:	$(RESULT_FILE)
	install -m 4755 $(RESULT_FILE) /usr/bin/$(RESULT_FILE)
