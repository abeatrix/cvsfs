/***************************************************************************
                          TEntry.h  -  description
                             -------------------
    begin                : Thu Jun 13 22:36:25 CEST 2002
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

#ifndef __TENTRY_H__
#define __TENTRY_H__

#include <string>
#include <iostream>
#include "TFileData.h"



class TEntry
{
  public:
    typedef enum
    {
      DirEntry,
      FileEntry,
      VersionedFileEntry
    } EntryType;

//    typedef enum
//    {
//      Remote,
//      Local,
//    } EntrySource;

    TEntry (const std::string &);
    virtual ~TEntry ();

    std::ostream & operator << (std::ostream &) const;

    virtual TEntry * Clone () const = 0;

    const std::string & GetName () const { return fName; }
    int GetLayer () const { return fLayer; }

    void SetLayer (int layer) { fLayer = layer; }
    void SetReadOnly () { fReadOnly = true; }

    virtual EntryType isA () const = 0;
    virtual bool ValidData () const = 0;

  protected:
    virtual void streamData (std::ostream &) const = 0;

    std::string fName;
    int		fLayer;
    bool	fReadOnly;
};



#endif
