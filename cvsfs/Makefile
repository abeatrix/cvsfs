# Makefile for cvsfs vfs module
#
# Author: Petric Frank
# Date:   May 24th, 2001

OBJECT_FILES =	cvsfs_init.o \
		commands.o \
		dir.o \
		file.o \
		cache.o \
		inode.o \
		proc.o \
		socket.o
			
RESULT_FILE =	cvsfs.o


CC = gcc
CXX = gcc
LD = ld
RM = rm -f

KERNEL_VERSION = $(shell uname -r)

CFLAGS = -O3 -Wall -DMODULE -D__KERNEL__ -DLINUX -fomit-frame-pointer -I/lib/modules/$(KERNEL_VERSION)/build/include
CXXFLAGS =
LDFLAGS = -m elf_i386 -r


all:		$(RESULT_FILE)


$(RESULT_FILE):	$(OBJECT_FILES) cvsfs_config.h
	$(LD) $(LDFLAGS) -o $@ $(OBJECT_FILES)


.c.o:
	$(CC)	-c -o $@ $(CFLAGS) $<


clean:
	$(RM) *.o


distclean:
	$(RM) *.o
	$(RM) $(RESULT_FILE)


install:	$(RESULT_FILE)
	install -d /lib/modules/$(KERNEL_VERSION)/kernel/fs/cvsfs
	install $(RESULT_FILE) /lib/modules/$(KERNEL_VERSION)/kernel/fs/cvsfs/$(RESULT_FILE)
	depmod -a
