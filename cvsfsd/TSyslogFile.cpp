/***************************************************************************
                          TSyslogFile.cpp  -  description
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

#include "TSyslogFile.h"

#include <syslog.h>



TSyslogFile::TSyslogFile (const std::string & id, int logFacility)
: fId (id), fFacility (logFacility)
{
  openlog (id.c_str (), LOG_PID, logFacility);
}



TSyslogFile::~TSyslogFile ()
{
  closelog ();
}



void TSyslogFile::reopen (const std::string & id, int logFacility)
{
  closelog ();

  openlog (id.c_str (), LOG_PID, logFacility);
}



void TSyslogFile::write (int level, const std::string & data) const
{
  syslog (level, "%s\n", data.c_str ());
}
