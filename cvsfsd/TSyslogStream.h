/***************************************************************************
                          TSyslogStream.h  -  description
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

#ifndef __TSYSLOGSTREAM_H__
#define __TSYSLOGSTREAM_H__

#include <iostream>

// forward reference
class TSyslogFile;


class TSyslogStream : public std::ostream
{
  public:
    typedef enum { Emergency = 0, Alert, Critical, Error, Warning, Notice, Info, Debug } LogLevel;

    TSyslogStream (TSyslogFile &, LogLevel);

    void Enable ();
    void Disable ();
};



#endif
