/***************************************************************************
                          TFileVersions.cpp  -  description
                             -------------------
    begin                : Thu Jun 14 18:23:22 CEST 2002
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

#include "TFileVersions.h"



TFileVersions::TFileVersions ()
: fValue (new VersionValue)
{
}



TFileVersions::TFileVersions (const TFileVersions & clone)
: fValue (clone.fValue)
{
  ++(fValue->fRefCount);
}



TFileVersions::~TFileVersions ()
{
  if ((--(fValue->fRefCount)) == 0)
    delete fValue;
}



TFileVersions & TFileVersions::operator = (const TFileVersions & rhs)
{
  if (fValue == rhs.fValue)
    return *this;

  if ((--(fValue->fRefCount)) <= 0)
    delete fValue;

  fValue = rhs.fValue;
  ++(fValue->fRefCount);

  return *this;
}



const TFile * TFileVersions::FindVersion (const std::string & version) const
{
  MapType::iterator iter;
  
  iter = fValue->fVersions.find (version);
  if (iter != fValue->fVersions.end ())
    return &((*iter).second);

  return 0;
}



const TFile * TFileVersions::GetVersion (int index) const
{
  MapType::iterator iter;
  int count;

  for (iter = fValue->fVersions.begin (), count = 0;
       (iter != fValue->fVersions.end ()); ++iter, ++count)
    if (count == index)
      return &((*iter).second);

  return 0;
}



void TFileVersions::AddVersion (const std::string & version, const TFile & data)
{
  fValue->fVersions.insert (ValuePair (version, data));
}



void TFileVersions::RemoveVersion (const std::string & version)
{
  fValue->fVersions.erase (version);
}



TFileVersions::VersionValue::VersionValue ()
: fRefCount (0)
{
}



TFileVersions::VersionValue::~VersionValue ()
{
}
