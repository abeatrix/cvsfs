/***************************************************************************
                          TCvsPserverCommandCheckout.cpp  -  description
                             -------------------
    begin                : Fri Sep 27 2002
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

// this to get the function 'strptime' defined
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "TCvsPserverCommandCheckout.h"

#include <time.h>
#include "TCvsConnection.h"
#include "TCvsSessionPserver.h"



TCvsPserverCommandCheckout::TCvsPserverCommandCheckout (const std::string & path,
							const std::string & version,
							std::ostream * out)
: TCvsPserverCommand (), fPath (path), fVersion (version), fFileStream (out)
{
}



TCvsPserverCommandCheckout::~TCvsPserverCommandCheckout ()
{
}



bool TCvsPserverCommandCheckout::execute (TCvsSessionPserver & session)
{
  fDataReceived = false;

  if (!session.execute ("Argument -N"))
    return false;

  if (!session.execute ("Argument -P"))
    return false;

  if (!session.execute ("Argument -r"))
    return false;

  if (!session.execute ("Argument " + fVersion))
    return false;

  if (!session.execute ("Argument " + fPath))
    return false;

  if (!session.execute ("Directory ."))
    return false;

  if (!session.execute (session.GetConnection().GetRoot ()))
    return false;

  if (!session.execute ("co"))
    return false;

  if (!processData (session))
    if (fFileStream || !fDataReceived)
      return false;

  return true;
}



bool TCvsPserverCommandCheckout::processLine (TCvsSessionPserver & session,
					      const std::string & line)
{
  if (line.find ("Mod-time ") == 0)
  {
    struct tm tm;
    int filetime = 0;

    memset (&tm, 0, sizeof (tm));
    if (strptime (&(line.c_str ())[9], "%d %b %Y %T ", &tm) != 0)
      filetime = mktime (&tm);

    fFileData.SetAtime (filetime);
    fFileData.SetCtime (filetime);
    fFileData.SetMtime (filetime);

    return true;
  }

  if ((line.length () > 2) && (line[0] == 'u') && (line[1] == '='))
  {
    fFileData.SetAttribute (CvsAttr2SysAttr (line));

    return true;
  }

  if (isdigit (line[0]) != 0)
  {
    fFileData.SetSize (strtoul (line.c_str (), 0, 0));
    fDataReceived = true;

    if (!fFileStream)
      return false;		// skip the rest of the data

    // now load the file contents
    const int bufsize = 4096;	// read the file in 4k blocks
    char readbuf[bufsize];
    loff_t pos = 0;
    loff_t size = fFileData.GetSize ();

    while (pos < size)
    {
      int chunk = bufsize;
      if ((pos + chunk) > size)
        chunk = size - pos;

      int bytes_read;
      if ((bytes_read = session.ReadRaw (readbuf, chunk)) == -1) // timeout
        return false;

      fFileStream->write (readbuf, bytes_read);

      pos += bytes_read;
    }
  }

  return true;
}
