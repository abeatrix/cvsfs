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
    TEntry (const TEntry &);
    virtual ~TEntry ();

    std::ostream & operator << (std::ostream &) const;

    virtual TEntry * Clone () const = 0;

    const std::string & GetName () const { return fName; }
    unsigned int GetLayer () const { return fLayer; }
    virtual const TFileData & GetData () const = 0;

    void SetName (const std::string & name) { fName = name; }
    void SetLayer (unsigned int layer) { fLayer = layer; }
    virtual void SetData (const TFileData & data) = 0;
    void SetReadOnly () { fReadOnly = true; }
    void ResetReadOnly () { fReadOnly = false; }

    virtual EntryType isA () const = 0;
    virtual bool ValidData () const = 0;

  protected:
    virtual void streamData (std::ostream &) const = 0;

    std::string 	fName;
    unsigned int	fLayer;
    bool		fReadOnly;
};



#endif
