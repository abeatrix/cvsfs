/***************************************************************************
                          TCacheSystem.h  -  description
                             -------------------
    begin                : Tue Aug 27 2002
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

#ifndef __TCACHESYSTEM_H__
#define __TCACHESYSTEM_H__

#include <string>
#include <fstream>

// forward reference
class TEntry;
class TDirectory;
class TFileData;



class TCacheSystem
{
  public:
    TCacheSystem (const std::string &, bool);
    virtual ~TCacheSystem ();

    void SetRoot (const std::string & root);
    void SetLayer (unsigned int layer) { fLayer = layer; }
    unsigned int GetLayer () { return fLayer; }

    virtual bool LoadTree (TDirectory &) const = 0;

    virtual bool HaveFile (const std::string &) const = 0;
    virtual bool HaveDirectory (const std::string &) const = 0;
    virtual bool CreateDirectory (const std::string &, int) const = 0;
    virtual bool CreateFile (const std::string &, int) const = 0;
    virtual bool DeleteDirectory (const std::string &) const = 0;
    virtual bool DeleteFile (const std::string &) const = 0;
    virtual bool Move (const std::string &, const TCacheSystem &, const std::string &) const = 0;

    virtual bool FileAttribute (const std::string &, TFileData &) const = 0;

    virtual std::ifstream * In (const std::string &, std::ios::openmode = std::ios::openmode(0)) const = 0;
    virtual std::ofstream * Out (const std::string &, std::ios::openmode = std::ios::openmode(0)) const = 0;

  protected:
    bool TreeLoad (TDirectory &, const std::string &) const;

    bool DirExists (const std::string &) const;
    bool FileExists (const std::string &) const;
    bool DirCreate (const std::string &, int) const;
    bool DirDelete (const std::string &) const;
    bool FileCreate (const std::string &, int) const;
    bool FileDelete (const std::string &) const;
    bool MoveItem (const std::string &, const TCacheSystem &, const std::string &) const;
    bool FileAttribs (const std::string &, TFileData &, bool &) const;

    virtual void FullPath (const std::string &, std::string &) const;
    virtual TEntry * AddDir (TDirectory &, const std::string &, TFileData &) const;
    virtual TEntry * AddFile (TDirectory &, const std::string &, TFileData &) const;

    std::string		fBase;
    std::string 	fAbsoluteBase;
    unsigned int	fLayer;
    bool		fReadOnly;
};



#endif
