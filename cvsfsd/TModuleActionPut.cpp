/***************************************************************************
                          TModuleActionPut.cpp  -  description
                             -------------------
    begin                : Thu Aug 22 2002
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

#include "TModuleActionPut.h"

#include <strstream>
#include "TModuleServer.h"
#include "TCvsInterface.h"
#include "TSyslog.h"



TModuleActionPut::TModuleActionPut ()
: TModuleAction ()
{
}



TModuleActionPut::~TModuleActionPut ()
{
}



/* expecting : "put <offset> <count> <path> <data>" */
/* the item "put" is already read                   */
bool TModuleActionPut::doit (TCvsInterface & interface)
{
  TSyslog *log = TSyslog::instance ();
  char buf[512];
  int size;

  log->debug << "in put::doit" << std::endl;

  if ((size = readItem (buf, sizeof (buf))) == -1)
    return true;

  long long start = strtoll (buf, 0, 0);

  if ((size = readItem (buf, sizeof (buf))) == -1)
    return true;

  int count = atoi (buf);

  if ((size = readItem (buf, sizeof (buf))) == -1)
    return true;

  std::string name = buf;

  log->debug << "line: " << start << " " << count << " " << name << std::endl;

  char *data = new char[count];

  if (!data)
    return true;

  if (readData (data, count) == count)
  {
    std::string version;
    std::string::size_type pos;

    if ((pos = name.find ("@@")) != std::string::npos)
    {
      version = name;
      version.erase (0, pos + 2);
      name.erase (pos);
    }

    // values stored:
    //   start ...... start write position
    //   count ...... number of bytes to write
    //   name ....... file name (absolute) w/o version number
    //   version .... if not NULL - version of file requested

    std::ostrstream result;

    result << interface.PutFile (name, version, start, count, data);

    writeData (result.str (), result.pcount ());
  }

  delete [] data;

  return false;		// continue with the next command
}
