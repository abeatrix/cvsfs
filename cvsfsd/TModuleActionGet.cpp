/***************************************************************************
                          TModuleActionGet.h  -  description
                             -------------------
    begin                : Sun Jun 9 18:32:49 CEST 2002
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

#include "TModuleActionGet.h"

#include "TModuleServer.h"
#include "TCvsInterface.h"
#include "TSyslog.h"



TModuleActionGet::TModuleActionGet ()
: TModuleAction ()
{
}



TModuleActionGet::~TModuleActionGet ()
{
}



/* expecting : "get <offset> <count> <path>[ <version>]" */
/* the item "dir" is already read    */
bool TModuleActionGet::doit (TCvsInterface & interface)
{
  TSyslog *log = TSyslog::instance ();
  bool dataWritten = false;
  char buf[512];
  char *count;
  int size;

  log->debug << "in get::doit" << std::endl;
  
  if ((size = readLine (buf, sizeof (buf))) != -1)
  {
    log->debug << "line: " << buf << std::endl;

    while ((size > 0) && (buf[size - 1] == ' '))
    {
      --size;
      buf[size] = '\0';
    }

    if ((size != 0) && ((count = strchr (buf, ' ')) != NULL))
    {
      char *ptr;
      unsigned long start;

      *count = '\0';
      ++count;
    
      start = strtoul (buf, 0, 0);

      if ((ptr = strchr (count, ' ')) != NULL)
      {
        std::string::size_type pos;
        int maxbytes;

        *ptr = '\0';
        ++ptr;

        maxbytes = atoi (count);

        std::string name = ptr;
        std::string version;

        if ((pos = name.find (' ')) != std::string::npos)
        {
          version = name;
          version.erase (0, pos + 1);
          name.erase (pos);
        }

        if ((pos = name.find ("@@")) != std::string::npos)
        {
          if (version.length () == 0)
          {
            version = name;
            version.erase (0, pos + 2);
          }

          name.erase (pos);
        }

        // values stored:
        //   start ...... start read position
        //   maxbytes ... max number of bytes to read
        //   name ....... file name (absolute) w/o version number
        //   version .... if not NULL - version of file requested

        char *buffer = new char[maxbytes];

        int received = interface.GetFile (name, version, start, maxbytes, buffer);
        if (received > 0)
        {
          writeData (buffer, received);

          dataWritten = true;
        }

        delete [] buffer;
      }
    }
  }

  if (!dataWritten)
   writeDummy ();

  return false;		// continue with the next command
}
