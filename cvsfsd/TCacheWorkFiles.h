/***************************************************************************
                          TCacheWorkFiles.h  -  description
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

#ifndef __TCACHEWORKFILES_H__
#define __TCACHEWORKFILES_H__

#include "TCache.h"

// forward reference
class TCachedFile;
class TDirectory;
class TFileData;


class TCacheWorkFiles : public TCache
{
  public:
    TCacheWorkFiles (const std::string &, const std::string &);

    TCachedFile * CachedFile (const std::string &) const;

    bool LoadTree (TDirectory &);
    bool MakeDirectory (const std::string &, int);
    int RemoveDirectory (const std::string &);
    bool MakeFile (const std::string &, int);
    bool FileData (const std::string &, TFileData &);

  private:
    bool LoadDir (TDirectory &, const std::string &);

    std::string fMountpoint;
};



#endif
