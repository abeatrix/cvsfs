/***************************************************************************
                          TCvsInterfacePserver.h  -  description
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

#ifndef __TCVSINTERFACEPSERVER_H__
#define __TCVSINTERFACEPSERVER_H__

#include "TCvsInterface.h"
#include "TCvsConnectionPserver.h"
#include "TMountParameters.h"
#include "TCacheVersionedFiles.h"
#include "TCacheWorkFiles.h"

// forward references
class TFile;
class TVersionedFile;

// This class implements the interface to a direct pserver communication
// to a cvs server.

class TCvsInterfacePserver : public TCvsInterface
{
  public:
    TCvsInterfacePserver (const TMountParameters &);

    bool Test ();
    virtual const TEntry * GetEntry (const std::string &, int);
    virtual const TEntry * GetFullEntry (const std::string &, const std::string &);
    virtual int GetFile (const std::string &, const std::string &, unsigned long, int, char *);
    virtual const TEntry * MakeDirectory (const std::string &, const std::string &, int);
    virtual int RemoveDirectory (const std::string &, const std::string &);
    virtual const TEntry * MakeFile (const std::string &, const std::string &, int);

  private:
    typedef enum
    {
      RDIFF_FILE,
      RDIFF_DIR,
      RDIFF_ELSE
    } RDiffResult;

    typedef enum
    {
      CO_MODTIME,
      CO_ATTRIB,
      CO_SIZE,
      CO_ELSE
    } CoResult;

    TCvsConnectionPserver	fConnection;
    TCacheVersionedFiles	fVersionedCache;
    TCacheWorkFiles		fWorkCache;

    bool LoadTree ();
    bool LoadCvsTree ();
    TEntry *FindEntry (TDirectory *, const std::string &) const;
    void RemoveEntry (const std::string &);

    TDirectory *AllocateDir (TDirectory *, const std::string &) const;
    TVersionedFile *AddVersionedFile (TDirectory *, const std::string &, const std::string &) const;
    RDiffResult AnalyzeRDiffLine (const std::string &, std::string &, std::string &) const;

    bool LoadAttribute (const std::string &, const std::string &, TVersionedFile &);
    CoResult AnalyzeCoLine (const std::string &, std::string &) const;
    int ConvertTime (const std::string &) const;
    int ConvertAttr (const std::string &) const;

    int LoadFile (const std::string &, const std::string &, TVersionedFile &, unsigned long, int, char *);
    TDirectory *GetParentDirectory (const std::string &, std::string &);
};



#endif
