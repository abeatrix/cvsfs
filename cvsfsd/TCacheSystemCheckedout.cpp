/***************************************************************************
                          TCacheSystemCheckedout.cpp  -  description
                             -------------------
    begin                : Thu Aug 29 2002
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

#include "TCacheSystemCheckedout.h"

//#include <fstream.h>		// already defined in TCacheSystem.h
#include <dirent.h>
#include "TEntry.h"
#include "TFile.h"



const std::string BaseDir = "/checkout";
const std::string VersionDelimiter = "@@";



TCacheSystemCheckedout::TCacheSystemCheckedout (const std::string & base)
: TCacheSystem (BaseDir + base, false)
{
}



bool TCacheSystemCheckedout::LoadTree (TDirectory & root) const
{
  return TreeLoad (root, "");
}



bool TCacheSystemCheckedout::HaveFile (const std::string & path) const
{
  std::string newpath;

  if (!EvaluateFullName (path, newpath))
    return false;

  return FileExists (newpath);
}



bool TCacheSystemCheckedout::HaveDirectory (const std::string & dir) const
{
  return DirExists (dir);
}



bool TCacheSystemCheckedout::CreateDirectory (const std::string & dir, int mode) const
{
  return DirCreate (dir, mode);
}



bool TCacheSystemCheckedout::CreateFile (const std::string & path, int mode) const
{
  std::string::size_type pos;

  if ((pos = path.rfind (VersionDelimiter)) == std::string::npos)
    return false;		// manage only versioned files

  return FileCreate (path, mode);
}



bool TCacheSystemCheckedout::DeleteDirectory (const std::string & dir) const
{
  return DirDelete (dir);
}



bool TCacheSystemCheckedout::DeleteFile (const std::string & path) const
{
  std::string newpath;

  if (!EvaluateFullName (path, newpath))
    return false;

  return FileDelete (newpath);
}



bool TCacheSystemCheckedout::Move (const std::string & from, const TCacheSystem & system, const std::string & to) const
{
  std::string newpath;

  if (!EvaluateFullName (from, newpath))
    return false;

  return MoveItem (newpath, system, to);
}



bool TCacheSystemCheckedout::FileAttribute (const std::string & path, TFileData & data) const
{
  std::string newpath;

  if (!EvaluateFullName (path, newpath))
    return false;

  bool dummy;

  return FileAttribs (newpath, data, dummy);
}



void TCacheSystemCheckedout::FullPath (const std::string & path, std::string & fullpath) const
{
  std::string newpath;

  if (!EvaluateFullName (path, newpath))
    newpath = path;

  TCacheSystem::FullPath (newpath, fullpath);
}



std::ifstream * TCacheSystemCheckedout::In (const std::string & path, int mode) const
{
  // Prerequisite: File must exist
  // must be created previously by a call to CreateFile
  std::string newpath;

  if (!EvaluateFullName (path, newpath))
    return 0;

  if (!HaveFile (newpath))
    return 0;

  return new std::ifstream ((fAbsoluteBase + "/" + newpath).c_str (), mode | ios::in);
}



std::ofstream * TCacheSystemCheckedout::Out (const std::string & path, int mode) const
{
  // Prerequisite: File must exist
  // must be created previously by a call to CreateFile
  std::string newpath;

  if (!EvaluateFullName (path, newpath))
    return 0;

  if (!HaveFile (newpath))
    return 0;

  return new std::ofstream ((fAbsoluteBase + "/" + newpath).c_str (), mode | ios::out);
}



TEntry * TCacheSystemCheckedout::AddFile (TDirectory & root,
					  const std::string & name,
					  TFileData & data) const
{
  // the name is always a full versioned one like '<file name>@@<version>'

  // first split off the version
  std::string filename = name;
  std::string version = name;
  std::string::size_type pos = name.find (VersionDelimiter);
  if (pos == std::string::npos)		// to be sure !
    return 0;

  filename.erase (pos);
  version.erase (0, pos + VersionDelimiter.length ());

  TEntry * entry = TCacheSystem::AddFile (root, filename, data);
  if ((entry) && (entry->isA () == TEntry::FileEntry))
  {
    TFile * file = static_cast<TFile *> (entry);

    file->SetVersion (version);
  }

  return entry;
}



bool TCacheSystemCheckedout::EvaluateFullName (const std::string & path,
					       std::string & fullpath) const
{
  std::string::size_type pos = path.rfind (VersionDelimiter);

  if (pos != std::string::npos)
  {
    fullpath = path;

    return true;
  }

  // no exact version given - do a search
  std::string name = path;
  std::string dir = fAbsoluteBase;

  fullpath = path;
  pos = fullpath.rfind ('/');
  if (pos != std::string::npos)
  {
    fullpath.erase (pos);	// strip the file name
    dir += "/";			// append the requested directory
    dir += fullpath;
    name.erase (0, pos + 1);	// keep file name alone
  }
  else
    fullpath = "";

  name += VersionDelimiter;

  // now do the directory scan
  struct dirent *direntry;
  DIR *cachedir = opendir (dir.c_str ());
  while ((direntry = readdir (cachedir)) != NULL)
  {
    if (strncmp (name.c_str (), direntry->d_name, name.length ()) == 0)
      break;
  }

  if (direntry != NULL)
  {				// file found
    if (fullpath.length () > 0)
      fullpath += "/";

    fullpath += direntry->d_name;	// append full file name
  }

  closedir (cachedir);

  return direntry != NULL;
}
