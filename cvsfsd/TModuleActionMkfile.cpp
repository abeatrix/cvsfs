/***************************************************************************
                          TModuleActionMkfile.cpp  -  description
                             -------------------
    begin                : Tue Aug 20 2002
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

#include "TModuleActionMkfile.h"

#include <asm/errno.h>
#include "TModuleServer.h"
#include "TCvsInterface.h"
#include "TSyslog.h"



TModuleActionMkfile::TModuleActionMkfile ()
: TModuleAction ()
{
}



TModuleActionMkfile::~TModuleActionMkfile ()
{
}



/* expecting : "mkfile <filepath> <mode>"  */
/* Note: the item "mkfile" is already read */
bool TModuleActionMkfile::doit (TCvsInterface & interface)
{
  TSyslog *log = TSyslog::instance ();
  std::ostrstream buffer;
  char buf[512];
  int size;

  log->debug << "in mkfile::doit" << std::endl;

  if ((size = readLine (buf, sizeof (buf))) != -1)
  {
    log->debug << "line: " << buf << std::endl;

    if (size != 0)
    {
      std::string path = buf;
      std::string version;

      while ((path.length () > 0) && (*(path.rbegin ()) == ' '))
        path.erase (path.length () - 1, 1);

      std::string::size_type pos = path.find (' ');
      if (pos != std::string::npos)
      {
        std::string modestr = path;
        int mode;

        modestr.erase (0, pos + 1);
        path.erase (pos);
        mode = atoi (modestr.c_str ());

        pos = path.find ("@@");
        if (pos != std::string::npos)
        {
          version = path;
          version.erase (0, pos + 2);
          path.erase (pos);
        }

        log->debug << "make file '" << path << "' with mode " << mode << std::endl;

        const TEntry * entry = interface.GetFullEntry (path, version);

        if (entry == 0)
        {
          entry = interface.MakeFile (path, version, mode);
          if (entry != 0)
            entry->operator << (buffer);
        }
      }
    }
  }

  if (buffer.pcount () != 0)
    writeData (buffer.str (), buffer.pcount ());
  else
    writeDummy ();

  return false;		// continue with the next command
}
