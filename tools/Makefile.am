# Process this file with automake to produce a Makefile.in

CXXFLAGS = -O3 -Wall

INCLUDES = -I../include

bin_PROGRAMS = cvsctl

cvsctl_SOURCES = main.cpp TFunction.cpp TFunctionCheckout.cpp \
	TFunctionVersion.cpp TFunctionServer.cpp

noinst_HEADERS = TFunction.h TFunctionCheckout.h TFunctionVersion.h \
	TFunctionServer.h
