/***************************************************************************
                          TModuleActionIoctl.cpp  -  description
                             -------------------
    begin                : Mon Aug 26 2002
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

#include "TModuleActionIoctl.h"

#include <asm/errno.h>
#include "TModuleServer.h"
#include "TCvsInterface.h"
#include "TSyslog.h"
#include "../../cvsfs.daemon/include/cvsfs_ioctl.h"



TModuleActionIoctl::TModuleActionIoctl ()
: TModuleAction ()
{
}



TModuleActionIoctl::~TModuleActionIoctl ()
{
}



/* expecting : "ioctl <id> [<parm>]"  */
/* Note: the item "ioctl" is already read */
bool TModuleActionIoctl::doit (TCvsInterface & interface)
{
  TSyslog *log = TSyslog::instance ();
  std::ostrstream buffer;
  char buf[4096];
  int size;

  log->debug << "in ioctl::doit" << std::endl;

  if ((size = readLine (buf, sizeof (buf))) != -1)
  {
    log->debug << "line: " << buf << std::endl;

    if (size != 0)
    {
      std::string idstr = buf;
      std::string parm;

      while ((idstr.length () > 0) && (*(idstr.rbegin ()) == ' '))
        idstr.erase (idstr.length () - 1, 1);

      std::string::size_type pos = idstr.find (' ');
      if (pos != std::string::npos)
      {
        parm = idstr;

        parm.erase (0, pos + 1);
        idstr.erase (pos);
      }

      int id = atoi (idstr.c_str ());

      std::string version;

      pos = parm.find ("@@");
      if (pos != std::string::npos)
      {
        version = parm;
        version.erase (0, pos + 2);
        parm.erase (pos);
      }

      std::string data;

      switch (id)
      {
        case CVSFS_RESCAN:
          buffer << interface.Invalidate (parm, version);
          break;

        case CVSFS_GET_VERSION:
	  buffer << interface.GetLocation (parm, version, data);
          if (data.length () > 0)
            buffer << " " << data;
          break;

        case CVSFS_CHECKOUT:
          buffer << interface.Checkout (parm, version);
          break;
      }
    }
  }

  if (buffer.pcount () != 0)
    writeData (buffer.str (), buffer.pcount ());
  else
    writeDummy ();

  return false;		// continue with the next command
}
