# Process this file with automake to produce a Makefile.in

KERNEL_VERSION = $(shell uname -r)

AM_INSTALL_PROGRAM_FLAGS = -m 0500

CXXFLAGS = -O3 -Wall

INCLUDES = -I/lib/modules/$(KERNEL_VERSION)/build/include

sbin_PROGRAMS = cvsfsd
cvsfsd_SOURCES = main.cpp TCache.cpp TCacheVersionedFiles.cpp \
	TCacheWorkFiles.cpp TCachedFile.cpp TConnectedSocket.cpp \
	TCvsConnection.cpp TCvsConnectionPserver.cpp TCvsInterface.cpp \
	TCvsInterfacePserver.cpp TCvsSession.cpp TCvsSessionPserver.cpp \
	TDirectory.cpp TEntry.cpp TFile.cpp TFileData.cpp TFileVersions.cpp \
	TModuleAction.cpp TModuleActionAttr.cpp TModuleActionDir.cpp \
	TModuleActionGet.cpp TModuleActionQuit.cpp TModuleServer.cpp \
	TMountParameters.cpp TSocket.cpp TSyslog.cpp TSyslogFile.cpp \
	TSyslogStream.cpp TSyslogStreambuf.cpp TVersionedFile.cpp \
	XException.cpp XPserverTimeout.cpp

noinst_HEADERS = TCache.h TCacheVersionedFiles.h TCacheWorkFiles.h \
	TCachedFile.h TConnectedSocket.h TCvsConnection.h \
	TCvsConnectionPserver.h TCvsInterface.h TCvsInterfacePserver.h \
	TCvsSession.h TCvsSessionPserver.h TDirectory.h TEntry.h TFile.h \
	TFileData.h TFileVersions.h TModuleAction.h TModuleActionAttr.h \
	TModuleActionDir.h TModuleActionGet.h TModuleActionQuit.h \
	TModuleServer.h TMountParameters.h TSocket.h TSyslog.h TSyslogFile.h \
	TSyslogStream.h TSyslogStreambuf.h TVersionedFile.h XException.h \
	XPserverTimeout.h
