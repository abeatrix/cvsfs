/***************************************************************************
                          TFile.cpp  -  description
                             -------------------
    begin                : Thu Jun 14 00:40:46 CEST 2002
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

#include "TFile.h"

#include <sys/stat.h>
#include "TFileData.h"



TFile::TFile (const std::string & name, const std::string & version)
: TEntry (name), fVersion (version), fDataValid (false)
{
}



TFile::~TFile ()
{
}



TEntry::EntryType TFile::isA () const
{
  return TEntry::FileEntry;
}



void TFile::SetData (const TFileData & data)
{
  fData = data;

  fDataValid = true;
}



void TFile::streamData (std::ostream & str) const
{
  fData.streamData (str, S_IFREG, fSource == TEntry::RemoteSource);
  str << " " << fVersion;
}
