/***************************************************************************
                          TSyslogStream.cpp  -  description
                             -------------------
    begin                : Fri Aug 16 2002
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

#include "TSyslogStream.h"

#include <syslog.h>
#include "TSyslogStreambuf.h"
#include "TSyslogFile.h"



// maps the levels to syslog aware values
const int LevelMapping[8]	= { LOG_EMERG, LOG_ALERT, LOG_CRIT, LOG_ERR,
				    LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG};



TSyslogStream::TSyslogStream (TSyslogFile & file, LogLevel logLevel)
: ostream (new TSyslogStreambuf (file, LevelMapping[logLevel]))
{
}



void TSyslogStream::Enable ()
{
  static_cast <TSyslogStreambuf *> (rdbuf ())->Enable ();
}



void TSyslogStream::Disable ()
{
  static_cast <TSyslogStreambuf *> (rdbuf ())->Disable ();
}
