/***************************************************************************
                          TCacheManager.cpp  -  description
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "TCacheManager.h"

// #include <sys/stat.h>
// #include <fstream.h>
#include "TCacheSystem.h"



TCacheManager::~TCacheManager ()
{
  // kill managed caches
  for (CacheVector::iterator iter = fCaches.begin (); iter != fCaches.end (); ++iter)
    delete *iter;
}



bool TCacheManager::LoadTree (TDirectory & root) const
{
  for (CacheVector::const_iterator iter = fCaches.begin ();
       iter != fCaches.end (); ++iter)
    if (!((*iter)->LoadTree (root)))
      return false;

  return true;
}



int TCacheManager::AddCache (TCacheSystem * cache)
{
  cache->SetRoot (fRoot);
  cache->SetLayer (fCaches.size ());

  fCaches.push_back (cache);

  return fCaches.size () - 1;
}



TCacheSystem * TCacheManager::GetSystem (size_type idx)
{
  if ((idx < 0) || (idx >= fCaches.size ()))
    return 0;

  return fCaches[idx];
}


/*
bool TCacheManager::HasFile (const std::string & name) const
{
  for (CacheVector::const_reverse_iterator iter = fCaches.rbegin ();
       iter != fCaches.rend (); ++iter)
  {
    if ((*iter)->HaveFile (name))
      return true;
  }

  return false;
}



bool TCacheManager::HasFile (const std::string & name, size_type cacheidx) const
{
  if (cacheidx >= fCaches.size ())
    return false;

  return fCaches[cacheidx]->HaveFile (name);
}
*/


bool TCacheManager::HaveFile (const std::string & name) const
{
  for (CacheVector::const_reverse_iterator iter = fCaches.rbegin ();
       iter != fCaches.rend (); ++iter)
  {
    if ((*iter)->HaveFile (name))
      return true;
  }

  return false;
}



bool TCacheManager::HaveFile (size_type cacheidx, const std::string & name) const
{
  if ((cacheidx < 0) || (cacheidx >= fCaches.size ()))
    return false;

  return fCaches[cacheidx]->HaveFile (name);
}



bool TCacheManager::HaveDirectory (size_type cacheidx, const std::string & name) const
{
  if ((cacheidx < 0) || (cacheidx >= fCaches.size ()))
    return false;

  return fCaches[cacheidx]->HaveDirectory (name);
}



bool TCacheManager::CreateDirectory (size_type cacheidx, const std::string & name, int mode) const
{
  if ((cacheidx < 0) || (cacheidx >= fCaches.size ()))
    return false;

  return fCaches[cacheidx]->CreateDirectory (name, mode);
}



bool TCacheManager::CreateFile (size_type cacheidx, const std::string & name, int mode) const
{
  if ((cacheidx < 0) || (cacheidx >= fCaches.size ()))
    return false;

  return fCaches[cacheidx]->CreateFile (name, mode);
}



bool TCacheManager::DeleteDirectory (size_type cacheidx, const std::string & name) const
{
  if ((cacheidx < 0) || (cacheidx >= fCaches.size ()))
    return false;

  return fCaches[cacheidx]->DeleteDirectory (name);
}



bool TCacheManager::DeleteFile (size_type cacheidx, const std::string & name) const
{
  if ((cacheidx < 0) || (cacheidx >= fCaches.size ()))
    return false;

  return fCaches[cacheidx]->DeleteFile (name);
}



bool TCacheManager::Move (size_type oldcacheidx, const std::string & oldname,
                          size_type newcacheidx, const std::string & newname) const
{
  if ((oldcacheidx < 0) || (oldcacheidx >= fCaches.size ()) ||
      (newcacheidx < 0) || (newcacheidx >= fCaches.size ()))
    return false;

  return fCaches[oldcacheidx]->Move (oldname, *(fCaches[newcacheidx]), newname);
}



bool TCacheManager::FileAttribute (size_type cacheidx, const std::string & name, TFileData & data) const
{
  if ((cacheidx < 0) || (cacheidx >= fCaches.size ()))
    return false;

  return fCaches[cacheidx]->FileAttribute (name, data);
}



bool TCacheManager::SetAttribute (size_type cacheidx, const std::string & name, TFileData & data) const
{
  if ((cacheidx < 0) || (cacheidx >= fCaches.size ()))
    return false;

  return fCaches[cacheidx]->SetAttribute (name, data);
}



std::ifstream * TCacheManager::In (size_type cacheidx, const std::string & name, std::ios::openmode mode) const
{
  if ((cacheidx < 0) || (cacheidx >= fCaches.size ()))
    return 0;

  return fCaches[cacheidx]->In (name, mode);
}



std::ofstream * TCacheManager::Out (size_type cacheidx, const std::string & name, std::ios::openmode mode) const
{
  if ((cacheidx < 0) || (cacheidx >= fCaches.size ()))
    return 0;

  return fCaches[cacheidx]->Out (name, mode);
}
