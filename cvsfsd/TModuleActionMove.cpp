/***************************************************************************
                          TModuleActionMove.cpp  -  description
                             -------------------
    begin                : Wed Sep 11 2002
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

#include <strstream>

#include "TModuleActionMove.h"

#include <asm/errno.h>
#include "TModuleServer.h"
#include "TCvsInterface.h"
#include "TSyslog.h"



TModuleActionMove::TModuleActionMove ()
: TModuleAction ()
{
}



TModuleActionMove::~TModuleActionMove ()
{
}



/* expecting : "move <old filepath> <new filepath>" */
/* Note: the item "move" is already read            */
bool TModuleActionMove::doit (TCvsInterface & interface)
{
  TSyslog *log = TSyslog::instance ();
  std::ostrstream buffer;
  char buf[512];
  int size;

  log->debug << "in move::doit" << std::endl;

  if ((size = readLine (buf, sizeof (buf))) != -1)
  {
    log->debug << "line: " << buf << std::endl;

    if (size != 0)
    {
      std::string oldpath = buf;
      std::string::size_type pos;;

      while ((oldpath.length () > 0) && (*(oldpath.rbegin ()) == ' '))
        oldpath.erase (oldpath.length () - 1, 1);

      pos = oldpath.find (' ');
      if (pos != std::string::npos)
      {
        std::string version;
        std::string newpath = oldpath;

        oldpath.erase (pos);
        newpath.erase (0, pos + 1);

        pos = oldpath.find ("@@");
        if (pos != std::string::npos)
        {
          version = oldpath;
          version.erase (0, pos + 2);
          oldpath.erase (pos);
        }

        log->debug << "move file from '" << oldpath << "' to '" << newpath << "'" << std::endl;

        const TEntry * entry = interface.GetFullEntry (oldpath, version);

        if (entry == 0)
          buffer << ENOENT;
        else
          buffer << interface.Move (oldpath, version, newpath);
      }
    }
  }

  if (buffer.pcount () != 0)
    writeData (buffer.str (), buffer.pcount ());
  else
    writeDummy ();

  return false;		// continue with the next command
}
