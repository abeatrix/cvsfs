# Process this file with automake to produce a Makefile.in

AM_INSTALL_PROGRAM_FLAGS = -m 0500

CXXFLAGS = -O3 -Wall

INCLUDES = -I../include

sbin_PROGRAMS = cvsfsd
cvsfsd_SOURCES = main.cpp TCacheManager.cpp TCacheSystem.cpp \
	TCacheSystemCheckedout.cpp TCacheSystemSimple.cpp \
	TCacheSystemVersioned.cpp TConnectedSocket.cpp TCvsConnection.cpp \
	TCvsConnectionPserver.cpp TCvsInterface.cpp TCvsInterfacePserver.cpp \
	TCvsPserverCommand.cpp TCvsPserverCommandCheckout.cpp \
	TCvsPserverCommandTree.cpp TCvsSession.cpp TCvsSessionPserver.cpp \
	TDirectory.cpp TEntry.cpp TFile.cpp TFileData.cpp TFileVersions.cpp \
	TModuleAction.cpp TModuleActionAttr.cpp TModuleActionDir.cpp \
	TModuleActionGet.cpp TModuleActionIoctl.cpp TModuleActionMkdir.cpp \
	TModuleActionMkfile.cpp TModuleActionMove.cpp TModuleActionPut.cpp \
	TModuleActionQuit.cpp TModuleActionRmdir.cpp TModuleActionRmfile.cpp \
	TModuleActionSetattr.cpp TModuleActionTruncfile.cpp TModuleServer.cpp \
	TMountParameters.cpp TSocket.cpp TSyslog.cpp TSyslogFile.cpp \
	TSyslogStream.cpp TSyslogStreambuf.cpp TVersionedFile.cpp \
	XException.cpp XPserverTimeout.cpp

noinst_HEADERS = TCacheManager.h TCacheSystem.h TCacheSystemCheckedout.h \
	TCacheSystemSimple.h TCacheSystemVersioned.h TConnectedSocket.h \
	TCvsConnection.h TCvsConnectionPserver.h TCvsInterface.h \
	TCvsInterfacePserver.h TCvsPserverCommand.h \
	TCvsPserverCommandCheckout.h TCvsPserverCommandTree.h TCvsSession.h \
	TCvsSessionPserver.h TDirectory.h TEntry.h TFile.h TFileData.h \
	TFileVersions.h TModuleAction.h TModuleActionAttr.h TModuleActionDir.h \
	TModuleActionGet.h TModuleActionIoctl.h TModuleActionMkdir.h \
	TModuleActionMkfile.h TModuleActionMove.h TModuleActionPut.h \
	TModuleActionQuit.h TModuleActionRmdir.h TModuleActionRmfile.h \
	TModuleActionSetattr.h TModuleActionTruncfile.h TModuleServer.h \
	TMountParameters.h TSocket.h TSyslog.h TSyslogFile.h TSyslogStream.h \
	TSyslogStreambuf.h TVersionedFile.h XException.h XPserverTimeout.h
