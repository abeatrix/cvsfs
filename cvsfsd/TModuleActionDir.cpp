/***************************************************************************
                          TModuleActionDir.h  -  description
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

#include "TModuleActionDir.h"

#include "TModuleServer.h"
#include "TCvsInterface.h"
#include "TSyslog.h"



TModuleActionDir::TModuleActionDir ()
: TModuleAction ()
{
}



TModuleActionDir::~TModuleActionDir ()
{
}



/* expecting : "dir <number> <path>" */
/* the item "dir" is already read    */
bool TModuleActionDir::doit (TCvsInterface & interface)
{
  TSyslog *log = TSyslog::instance ();
  bool dataWritten = false;
  char buf[512];
  char *dir;
  int size;

  log->debug << "in dir::doit" << std::endl;
  
  if ((size = readLine (buf, sizeof (buf))) != -1)
  {
    log->debug << "line: " << buf << std::endl;

    while ((size > 0) && (buf[size - 1] == ' '))
    {
      --size;
      buf[size] = '\0';
    }

    if ((size != 0) && ((dir = strchr (buf, ' ')) != NULL))
    {
      int index;

      *dir = '\0';
      ++dir;

      index = atoi (buf);

      // <dir> contains the directory in question
      // <index> is the sequence number of the entry
      //         only values above 1 should be treated
      //           0 is reserved for '.'
      //           1 is reserved for '..'
      //         these are already treated by the vfs kernel module

      if (index >= 2)
      {
        const TEntry * entry = interface.GetEntry (dir, index - 2);

        if (entry != 0)
        {
          writeData (entry->GetName ());

	  dataWritten = true;
        }
      }
    }
  }

  if (!dataWritten)
    writeDummy ();

  return false;		// continue with the next command
}
