/***************************************************************************
                          TFile.h  -  description
                             -------------------
    begin                : Thu Jun 14 00:15:41 CEST 2002
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

#ifndef __TFILE_H__
#define __TFILE_H__

#include <string>
#include <map>
#include "TEntry.h"

// forward references
class TFileData;



class TFile : public TEntry
{
  public:
    TFile (const std::string &, const std::string &);
    virtual ~TFile ();

    const std::string & GetVersion () const { return fVersion; }
    const TFileData & GetData () const { return fData; }

    virtual TEntry::EntryType isA () const;

    virtual bool ValidData () const { return fDataValid; }

    void SetData (const TFileData & data);

  private:
    virtual void streamData (std::ostream &) const;

    std::string		fVersion;
    bool		fDataValid;
    TFileData		fData;
};



#endif
