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
{
}



TFileVersions::TFileVersions (const TFileVersions & clone)
{
  MapType::const_iterator iter;

  for (iter = clone.fVersions.begin (); iter != clone.fVersions.end (); ++iter)
  {
    TFile * data = new TFile (*((*iter).second));

    fVersions.insert (ValuePair ((*iter).first, data));
  }
}



TFileVersions::~TFileVersions ()
{
  MapType::iterator iter;

  for (iter = fVersions.begin (); iter != fVersions.end (); ++iter)
    delete (*iter).second;
}



TFileVersions & TFileVersions::operator = (const TFileVersions & rhs)
{
  if (this == &rhs)
    return *this;

  MapType::iterator erase;

  for (erase = fVersions.begin (); erase != fVersions.end (); ++erase)
    delete (*erase).second;

  fVersions.clear ();

  MapType::const_iterator iter;

  for (iter = rhs.fVersions.begin (); iter != rhs.fVersions.end (); ++iter)
  {
    TFile * data = new TFile (*((*iter).second));

    fVersions.insert (ValuePair ((*iter).first, data));
  }

  return *this;
}



const TFile * TFileVersions::FindVersion (const std::string & version) const
{
  MapType::const_iterator iter;
  
  iter = fVersions.find (version);
  if (iter != fVersions.end ())
    return (*iter).second;

  return 0;
}



const TFile * TFileVersions::GetVersion (int index) const
{
  MapType::const_iterator iter;
  int count;

  for (iter = fVersions.begin (), count = 0;
       (iter != fVersions.end ()); ++iter, ++count)
    if (count == index)
      return (*iter).second;

  return 0;
}



void TFileVersions::AddVersion (const std::string & version, TFile * data)
{
  fVersions.insert (ValuePair (version, data));
}



void TFileVersions::RemoveVersion (const std::string & version)
{
  MapType::iterator iter = fVersions.find (version);

  if (iter != fVersions.end ())
  {
    delete (*iter).second;

    fVersions.erase (iter);
  }
}
