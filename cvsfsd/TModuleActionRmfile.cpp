/***************************************************************************
                          TModuleActionRmfile.cpp  -  description
                             -------------------
    begin                : Wed Aug 21 2002
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

#include "TModuleActionRmfile.h"

#include <sstream>
#include <asm/errno.h>
#include "TModuleServer.h"
#include "TCvsInterface.h"
#include "TSyslog.h"



TModuleActionRmfile::TModuleActionRmfile ()
: TModuleAction ()
{
}



TModuleActionRmfile::~TModuleActionRmfile ()
{
}



/* expecting : "rmfile <filepath>"         */
/* Note: the item "rmfile" is already read */
bool TModuleActionRmfile::doit (TCvsInterface & interface)
{
  TSyslog *log = TSyslog::instance ();
  std::ostringstream buffer;
  char buf[512];
  int size;

  log->debug << "in rmfile::doit" << std::endl;

  if ((size = readLine (buf, sizeof (buf))) != -1)
  {
    log->debug << "line: " << buf << std::endl;

    if (size != 0)
    {
      std::string path = buf;
      std::string version;

      while ((path.length () > 0) && (*(path.rbegin ()) == ' '))
        path.erase (path.length () - 1, 1);

      std::string::size_type pos = path.find ("@@");
      if (pos != std::string::npos)
      {
        version = path;
        version.erase (0, pos + 2);
        path.erase (pos);
      }

      log->debug << "remove file '" << path << "'" << std::endl;

      const TEntry * entry = interface.GetFullEntry (path, version);

      if (entry == 0)
        buffer << ENOENT;
      else
        buffer << interface.RemoveFile (path, version);
    }
  }

  if (buffer.str ().size () != 0)
    writeData (buffer.str ().c_str (), buffer.str ().size ());
  else
    writeDummy ();

  return false;		// continue with the next command
}
