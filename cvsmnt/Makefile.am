# Process this file with automake to produce a Makefile.in

KERNEL_VERSION = $(shell uname -r)

AM_INSTALL_PROGRAM_FLAGS = -m 4755

INCLUDES = -I/lib/modules/$(KERNEL_VERSION)/build/include

bin_PROGRAMS = cvsmnt
cvsmnt_SOURCES = main.c
