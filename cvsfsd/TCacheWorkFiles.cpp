/***************************************************************************
                          TCacheWorkFiles.cpp  -  description
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

#include "TCacheWorkFiles.h"

#include "TCachedFile.h"



TCacheWorkFiles::TCacheWorkFiles (const std::string & root, const std::string & mount)
: TCache (root, false), fMountpoint (mount)
{
}



TCachedFile * TCacheWorkFiles::CachedFile (const std::string & path) const
{
  std::string dir = fRoot + fMountpoint + "/" + path;
  std::string::size_type pos;

  pos = dir.rfind ('/');
  if (pos == std::string::npos)
    return 0;			// file to be at root '/' - nonsense

  std::string name = dir;

  dir.erase (pos);
  name.erase (0, pos + 1);

  return new TCachedFile (dir, name);
}
