/***************************************************************************
                          TDirectory.h  -  description
                             -------------------
    begin                : Thu Jun 13 22:16:19 CEST 2002
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

#ifndef __TDIRECTORY_H__
#define __TDIRECTORY_H__

#include <string>
#include <map>
#include "TEntry.h"



class TDirectory : public TEntry
{
  public:
    TDirectory (const std::string &);
    TDirectory (const TDirectory &);
    virtual ~TDirectory ();

    virtual TEntry * Clone () const;

    TEntry * FindEntry (const std::string &) const;
    TEntry * GetEntry (int) const;
    virtual const TFileData & GetData () const { return fFileData; };

    virtual TEntry::EntryType isA () const;

    virtual void SetData (const TFileData & data);

    void AddEntry (TEntry *);
    void RemoveEntry (TEntry *);

    bool ValidData () const { return fValidData; }

  private:
    virtual void streamData (std::ostream &) const;

    typedef std::map<std::string, TEntry *> MapType;
    typedef MapType::value_type ValuePair;
    
    TFileData		fFileData;
    bool		fValidData;
    MapType		fContents;
};



#endif
