/***************************************************************************
                          TModuleActionSetattr.cpp  -  description
                             -------------------
    begin                : Fri Sep 13 2002
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

#include "TModuleActionSetattr.h"

#include <sstream>
#include <asm/errno.h>
#include "TModuleServer.h"
#include "TCvsInterface.h"
#include "TSyslog.h"



TModuleActionSetattr::TModuleActionSetattr ()
: TModuleAction ()
{
}



TModuleActionSetattr::~TModuleActionSetattr ()
{
}



/* expecting : "setattr <file> <new attribute>"     */
/* Note: the item "setattr" is already read         */
bool TModuleActionSetattr::doit (TCvsInterface & interface)
{
  TSyslog *log = TSyslog::instance ();
  std::ostringstream buffer;
  char buf[512];
  int size;

  log->debug << "in setattr::doit" << std::endl;

  if ((size = readLine (buf, sizeof (buf))) != -1)
  {
    log->debug << "line: " << buf << std::endl;

    if (size != 0)
    {
      std::string path = buf;
      std::string::size_type pos;;

      while ((path.length () > 0) && (*(path.rbegin ()) == ' '))
        path.erase (path.length () - 1, 1);

      pos = path.find (' ');
      if (pos != std::string::npos)
      {
        std::string version;
        std::string mode = path;

        path.erase (pos);
        mode.erase (0, pos + 1);

        pos = path.find ("@@");
        if (pos != std::string::npos)
        {
          version = path;
          version.erase (0, pos + 2);
          path.erase (pos);
        }

        int attr = atoi (mode.c_str ());

        log->debug << "set attribute for file '" << path << "' to 0x" << std::hex << attr << std::endl;

        buffer << interface.SetAttr (path, version, attr);
      }
    }
  }

  if (buffer.str ().size () != 0)
    writeData (buffer.str ().c_str (), buffer.str ().size ());
  else
    writeDummy ();

  return false;		// continue with the next command
}
