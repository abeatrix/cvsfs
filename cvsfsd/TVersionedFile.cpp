/***************************************************************************
                          TVersionedFile.cpp  -  description
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "TVersionedFile.h"

#include <sys/stat.h>
#include "TFile.h"



TVersionedFile::TVersionedFile (const std::string & name, const std::string & version)
: TEntry (name), fHeadVersion (version), fDataValid (false)
{
}



TVersionedFile::~TVersionedFile ()
{
}



const TFile * TVersionedFile::FindVersion (const std::string & version) const
{
  return fFileVersions.FindVersion (version);
}



const TFile * TVersionedFile::GetVersion (int index) const
{
  return fFileVersions.GetVersion (index);
}



TEntry::EntryType TVersionedFile::isA () const
{
  return TEntry::FileEntry;
}



void TVersionedFile::AddVersion (const std::string & version, const TFileData & data)
{
  TFile item (GetName (), version);

  item.SetData (data);

  fFileVersions.AddVersion (version, item);

  if (version == fHeadVersion)
    fDataValid = true;
}



void TVersionedFile::streamData (std::ostream & str) const
{
  const TFile *data = FindVersion (fHeadVersion);

  if (data != 0)
    data->operator << (str);
}
