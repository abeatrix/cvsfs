/***************************************************************************
                          TVersionedFile.h  -  description
                             -------------------
    begin                : Tue Aug 13 2002
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

#ifndef __TVERSIONEDFILE_H__
#define __TVERSIONEDFILE_H__

#include <string>
#include <map>
#include "TEntry.h"
#include "TFileVersions.h"

// forward reference
class TFile;



class TVersionedFile : public TEntry
{
  public:
    TVersionedFile (const std::string &, const std::string &);
    virtual ~TVersionedFile ();

    virtual TEntry * Clone () const;

    const std::string & GetHeadVersion () const { return fHeadVersion; }

    const TFile * FindVersion (const std::string &) const;
    const TFile * GetVersion (int) const;

    virtual TEntry::EntryType isA () const;

    void AddVersion (const std::string &, const TFileData &);

    void SetHeadVersion (const std::string & version) { fHeadVersion = version; }

    virtual bool ValidData () const { return fDataValid; }

  private:
    virtual void streamData (std::ostream &) const;

    std::string		fHeadVersion;
    bool		fDataValid;
    TFileVersions	fFileVersions;
};



#endif
