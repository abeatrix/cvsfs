/***************************************************************************
                          TSyslogStreambuf.cpp  -  description
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

#include "TSyslogStreambuf.h"

#include "TSyslogFile.h"



TSyslogStreambuf::TSyslogStreambuf (TSyslogFile & file, int logLevel)
: std::streambuf (), fFile (file), fLevel (logLevel), fEnable (true)
{
}



TSyslogStreambuf::~TSyslogStreambuf ()
{
  if (fLineBuffer.length () > 0)
    overflow ('\n');
}



int TSyslogStreambuf::overflow (int ch)
{
  if ((fEnable) && (ch != EOF))
  {
    if (ch == '\n')
    {
      fFile.write (fLevel, fLineBuffer);
      fLineBuffer.erase ();
    }
    else
      fLineBuffer += ch;
  }

  return ch;
}
