/***************************************************************************
                          TCacheVersionedFiles.h  -  description
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

#ifndef __TCACHEVERSIONEDWORKFILES_H__
#define __TCACHEVERSIONEDFILES_H__

#include "TCache.h"

// forward reference
class TCachedFile;



class TCacheVersionedFiles : public TCache
{
  public:
    TCacheVersionedFiles (const std::string &, const std::string &,
			  const std::string &);

    TCachedFile * CachedFile (const std::string &, const std::string &) const;

  private:
    std::string fServer;
    std::string fCvsroot;
    std::string fSubdir;
};



#endif
