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
  SetReadOnly ();
}



TVersionedFile::TVersionedFile (const TVersionedFile & clone)
: TEntry (clone), fHeadVersion (clone.fHeadVersion),
  fDataValid (clone.fDataValid), fFileVersions (clone.fFileVersions)
{
}



TVersionedFile::~TVersionedFile ()
{
}



TEntry * TVersionedFile::Clone () const
{
  return new TVersionedFile (*this);
}



const TFile * TVersionedFile::FindVersion (const std::string & version) const
{
  return fFileVersions.FindVersion (version);
}



const TFile * TVersionedFile::GetVersion (int index) const
{
  return fFileVersions.GetVersion (index);
}



const TFileData & TVersionedFile::GetData () const
{
  static TFileData dummyVersion;

  const TFile *file = FindVersion (fHeadVersion);

  if (!file)
    return dummyVersion;

  return file->GetData ();
}



TEntry::EntryType TVersionedFile::isA () const
{
  return TEntry::VersionedFileEntry;
}



void TVersionedFile::AddVersion (const std::string & version, const TFileData & data)
{
  TFile * item = new TFile (GetName (), version);

  item->SetLayer (fLayer);
  item->SetData (data);

  if (fReadOnly)
    item->SetReadOnly ();

  fFileVersions.AddVersion (version, item);

  if (version == fHeadVersion)
    fDataValid = true;
}



void TVersionedFile::SetHeadVersion (const std::string & version)
{
  fHeadVersion = version;

  if (!fFileVersions.FindVersion (version))
    fDataValid = false;
}



void TVersionedFile::SetData (const TFileData & data)
{
  TFile * item = new TFile (GetName (), fHeadVersion);

  item->SetLayer (fLayer);
  item->SetData (data);

  if (fReadOnly)
    item->SetReadOnly ();

  fFileVersions.AddVersion (fHeadVersion, item);

  fDataValid = true;
}



void TVersionedFile::streamData (std::ostream & str) const
{
  const TFile *data = FindVersion (fHeadVersion);

  if (data != 0)
    data->operator << (str);
}
