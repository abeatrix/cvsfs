/***************************************************************************
                          TSyslog.cpp  -  description
                             -------------------
    begin                : Sun Jun 9 18:38:12 CEST 2002
    copyright            : (C) 2002 by Petric Frank
    email                : pfrank@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "TSyslog.h"

#include <syslog.h>
#include "TSyslogFile.h"



// mapping of the enum LogFacility to the values required by syslogd
const int FacilityMapping[] = { LOG_KERN,	// Kernel messages
				LOG_USER,	// user-level messages
				LOG_MAIL,	// mail system
				LOG_DAEMON,	// system daemons
				LOG_AUTH,	// authorization messages
				LOG_SYSLOG,	// syslogd internal messages
				LOG_LPR,	// line printer subsystem
				LOG_NEWS,	// network news subsystem
				LOG_UUCP,	// UUCP subsystem
				LOG_CRON,	// clock daemon
				LOG_AUTHPRIV,	// auth messages (private)
				LOG_FTP};	// ftp daemon



TSyslog *TSyslog::fInstance	= 0;
TSyslogFile *TSyslog::fFile	= 0;



TSyslog::TSyslog ()
: emergency (*fFile, TSyslogStream::Emergency),
  alert (*fFile, TSyslogStream::Alert),
  critical (*fFile, TSyslogStream::Critical),
  error (*fFile, TSyslogStream::Error),
  warning (*fFile, TSyslogStream::Warning),
  notice (*fFile, TSyslogStream::Notice),
  info (*fFile, TSyslogStream::Info),
  debug (*fFile, TSyslogStream::Debug)
{
}



TSyslog::~TSyslog ()
{
  delete fInstance;
  delete fFile;
}



TSyslog *TSyslog::instance ()
{
  if (fInstance == 0)
  {
    fFile = new TSyslogFile ("dummy", FacilityMapping[User]);
    fInstance = new TSyslog ();
  }

  return fInstance;
}



TSyslog *TSyslog::instance (const std::string & id, LogFacility logFacility)
{
  if (fInstance == 0)
  {
    fFile = new TSyslogFile (id, FacilityMapping[logFacility]);
    fInstance = new TSyslog ();
  }
  else
    fFile->reopen (id, FacilityMapping[logFacility]);

  return fInstance;
}
