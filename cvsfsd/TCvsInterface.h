/***************************************************************************
                          TCvsInterface.h  -  description
                             -------------------
    begin                : Fri Aug 9 2002
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

#ifndef __TCVSINTERFACE_H__
#define __TCVSINTERFACE_H__

#include "TDirectory.h"

// forward references
class TEntry;

// This is the base class for all communication with a cvs database.
// Each communication interface must derive from here.
// Following methods are provided:
//   GetEntry ........ Allow to scan a directory by an index (zero based).
//                     It returns 0 (zero) if no more data is available.
//                     Otherwise a TEntry with the filename set.
//   GetFullEntry .... Obtains all data available for a specific file or
//                     directory. A version can be supplied as parameter.
//                     If the version string is empty, the latest version
//                     of the file or directory is returned. Returns
//                     0 (zero) if the file does not exist or no data can
//                     be evaluated.
//   GetFile ......... Retrieve the contents of a file.
//
// The directory and file paths must be absolute - i.e. begin with a slash.

class TCvsInterface
{
  public:
    TCvsInterface ();
    virtual ~TCvsInterface ();

    virtual const TEntry * GetEntry (const std::string &, int) = 0;
    virtual const TEntry * GetFullEntry (const std::string &, const std::string &) = 0;
    virtual int GetFile (const std::string &, const std::string &, long long, int, char *) = 0;
    virtual int PutFile (const std::string &, const std::string &, long long, int, char *) = 0;
    virtual const TEntry * MakeDirectory (const std::string &, const std::string &, int) = 0;
    virtual int RemoveDirectory (const std::string &, const std::string &) = 0;
    virtual const TEntry * MakeFile (const std::string &, const std::string &, int) = 0;
    virtual int RemoveFile (const std::string &, const std::string &) = 0;

  protected:
    TDirectory	fRootDir;
    bool	fTreeLoaded;
};



#endif
