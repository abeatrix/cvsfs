/***************************************************************************
                          TCacheVersionedFiles.cpp  -  description
                             -------------------
    begin                : Mon Aug 12 2002
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

#include "TCacheVersionedFiles.h"

#include "TCachedFile.h"



TCacheVersionedFiles::TCacheVersionedFiles (const std::string & root,
					    const std::string & server,
					    const std::string & cvsroot)
: TCache (root, false), fServer (server), fCvsroot (cvsroot)
{
  if (fServer.length () == 0)
    fServer = "localhost";

  if (fCvsroot.length () == 0)
    fCvsroot = "/cvsroot";

  fSubdir = fServer + fCvsroot;
}



TCachedFile * TCacheVersionedFiles::CachedFile (const std::string & path,
						const std::string & version) const
{
  return new TCachedFile (fRoot + "/" + fSubdir + "/" + path, version);
}

