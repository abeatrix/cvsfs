# Process this file with automake to produce a Makefile.in

KERNEL_VERSION = $(shell uname -r)

AM_INSTALL_PROGRAM_FLAGS = -m 644

DEFS = -DMODULE -D__KERNEL__ -DLINUX -DVERSION=\"$(VERSION)\"
CFLAGS = -O3 -Wall -fomit-frame-pointer
INCLUDES = -I/lib/modules/$(KERNEL_VERSION)/build/include

LINK = $(LD) -m elf_i386 -r -o $@

libdir = /lib/modules/$(KERNEL_VERSION)/kernel/fs/

pkglib_PROGRAMS = cvsfs.o
cvsfs_o_SOURCES = main.c devfs.c dir.c \
	file.c inode.c proc.c procfs.c superblock.c util.c

noinst_HEADERS = cvsfs_config.h devfs.h dir.h \
	file.h inode.h procfs.h proc.h superblock.h util.h

install-exec-hook:
	[ "$(DESTDIR)" ] || /sbin/depmod -a

install-exec-local:
	[ "$(DESTDIR)" ] || ../makedevs

uninstall-local:
	[ "$(DESTDIR)" ] || ../rmdevs
