/***************************************************************************
                          TDirectory.cpp  -  description
                             -------------------
    begin                : Thu Jun 13 22:33:23 CEST 2002
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

#include "TDirectory.h"
#include <sys/stat.h>



TDirectory::TDirectory (const std::string & name)
: TEntry (name), fValidData (false)
{
}



TDirectory::~TDirectory ()
{
  MapType::iterator iter;

  for (iter = fContents.begin (); iter != fContents.end (); ++iter)
    delete (*iter).second;
}



TEntry * TDirectory::FindEntry (const std::string & name) const
{
  MapType::const_iterator iter;

  iter = fContents.find (name);
  if (iter != fContents.end ())
    return (*iter).second;

  return 0;
}



TEntry * TDirectory::GetEntry (int index) const
{
  MapType::const_iterator iter;
  int count;

  for (iter = fContents.begin (), count = 0; iter != fContents.end (); ++iter, ++count)
    if (count == index)
      return (*iter).second;

  return 0;
}



TEntry::EntryType TDirectory::isA () const
{
  return TEntry::DirEntry;
}



void TDirectory::AddEntry (TEntry * entry)
{
  fContents.insert (ValuePair (entry->GetName (), entry));
}



void TDirectory::RemoveEntry (TEntry * entry)
{
  fContents.erase (entry->GetName ());

  delete entry;
}



void TDirectory::SetData (const TFileData & data)
{
  fFileData = data;
  
  fValidData = true;
}



void TDirectory::streamData (std::ostream & str) const
{
  fFileData.streamData (str, S_IFDIR, false);
}
