/***************************************************************************
                          TSyslog.h  -  description
                             -------------------
    begin                : Sun Jun 9 18:37:42 CEST 2002
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

#ifndef __TSYSLOG_H__
#define __TSYSLOG_H__

#include <string>
#include "TSyslogStream.h"

// forward reference
class TSyslogFile;



class TSyslog
{
  public:
    typedef enum
    {
      Kernel = 0,
      User,
      Mail,
      Daemon,
      Auth,
      Syslog,
      Lpr,
      News,
      Uucp,
      Cron,
      AuthPriv,
      Ftp
    } LogFacility;

    TSyslog (const std::string &, LogFacility);
    ~TSyslog ();
  
    static TSyslog *instance ();
    static TSyslog *instance (const std::string &, LogFacility);

  private:
    TSyslog ();

    static TSyslogFile	*fFile;

  public:
    TSyslogStream	emergency;
    TSyslogStream	alert;
    TSyslogStream	critical;
    TSyslogStream	error;
    TSyslogStream	warning;
    TSyslogStream	notice;
    TSyslogStream	info;
    TSyslogStream	debug;

  private:
    static TSyslog	*fInstance;
};



#endif
