/***************************************************************************
                          TCacheSystem.cpp  -  description
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

#include "TCacheSystem.h"

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include "TEntry.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TFileData.h"



TCacheSystem::TCacheSystem (const std::string & base, bool readOnly)
: fBase (base), fAbsoluteBase (base), fLayer (-1), fReadOnly (readOnly)
{
}



TCacheSystem::~TCacheSystem ()
{
}



void TCacheSystem::SetRoot (const std::string & root)
{
  fAbsoluteBase = root + fBase;

  DirCreate ("", S_IRUSR | S_IWUSR | S_IXUSR);	// root-only access !
}



bool TCacheSystem::LoadTree (TDirectory & root) const
{
  return TreeLoad (root, "");
}



bool TCacheSystem::HaveFile (const std::string & path) const
{
  return FileExists (path);
}



bool TCacheSystem::HaveDirectory (const std::string & dir) const
{
  return DirExists (dir);
}



bool TCacheSystem::CreateDirectory (const std::string & dir, int mode) const
{
  return DirCreate (dir, mode);
}



bool TCacheSystem::CreateFile (const std::string & path, int mode) const
{
  return FileCreate (path, mode);
}



bool TCacheSystem::DeleteDirectory (const std::string & dir) const
{
  return DirDelete (dir);
}



bool TCacheSystem::DeleteFile (const std::string & path) const
{
  return FileDelete (path);
}



bool TCacheSystem::FileAttribute (const std::string & path, TFileData & data) const
{
  return FileAttribs (path, data);
}



std::ifstream * TCacheSystem::In (const std::string & path, int mode) const
{
  // Prerequisite: File must exist
  // must be created previously by a call to CreateFile

  if (!HaveFile (path))
    return 0;

  return new std::ifstream ((fAbsoluteBase + "/" + path).c_str (), mode | ios::in);
}



std::ofstream * TCacheSystem::Out (const std::string & path, int mode) const
{
  // Prerequisite: File must exist
  // must be created previously by a call to CreateFile

  if (!HaveFile (path))
    return 0;

  return new std::ofstream ((fAbsoluteBase + "/" + path).c_str (), mode | ios::out);
}



bool TCacheSystem::TreeLoad (TDirectory & root, const std::string & path) const
{
  std::string dir = fAbsoluteBase + "/" + path;
  DIR *cachedir = opendir (dir.c_str ());
  struct dirent *direntry;

  if (cachedir == NULL)
    return true;

  while ((direntry = readdir (cachedir)) != NULL)
  {
    if ((strcmp (direntry->d_name, ".") == 0) ||
        (strcmp (direntry->d_name, "..") == 0))
      continue;

    std::string filename = direntry->d_name;

    std::string filepath = dir + "/" + filename;
    struct stat info;

    // obtain file attributes
    TFileData data;
    if (FileAttribute (path + "/" + filename, data))
    {
      TEntry * entry;

      if (S_ISDIR (info.st_mode))
        entry = AddDir (root, filename, data);
      else
        entry = AddFile (root, filename, data);

      if (entry)
        // we have now a valid entry to the current file/directory
        if (entry->isA () == TEntry::DirEntry)
	  // this is a directory - dig into this one recursively
          TreeLoad (*static_cast<TDirectory *> (entry), path + "/" + filename);
    }
  }

  closedir (cachedir);

  return true;
}



bool TCacheSystem::DirExists (const std::string & dir) const
{
  std::string path = fAbsoluteBase + "/" + dir;
  struct stat info;

  if (lstat (path.c_str (), &info) != 0)
    return false;

  return S_ISDIR (info.st_mode);
}



bool TCacheSystem::FileExists (const std::string & name) const
{
  std::string path = fAbsoluteBase + "/" + name;
  struct stat info;

  if (lstat (path.c_str (), &info) != 0)
    return false;

  return S_ISREG (info.st_mode);
}



bool TCacheSystem::DirCreate (const std::string & dir, int mode) const
{
  std::string path = fAbsoluteBase;
  std::string::size_type pos;
  std::string dirpart;
  int dirmode = S_IFDIR | mode;

  if ((mode & S_IRUSR) != 0)
    dirmode |= S_IXUSR;

  if ((mode & S_IRGRP) != 0)
    dirmode |= S_IXGRP;

  if ((mode & S_IROTH) != 0)
    dirmode |= S_IXOTH;

  if (dir.length () != 0)
    path += "/" + dir;

  pos = 0;
  do
  {
    pos = path.find ('/', pos + 1);

    dirpart = path;
    if (pos != std::string::npos)
      dirpart.erase (pos);

    struct stat info;

    if (lstat (dirpart.c_str (), &info) != 0)
    {
      int result = mkdir (dirpart.c_str (), dirmode);
      if (result != 0)
        return false;
    }
    else
      if (!S_ISDIR (info.st_mode))
        return false;

  } while (pos != std::string::npos);

  return true;
}



bool TCacheSystem::DirDelete (const std::string & dir) const
{
  std::string path = fAbsoluteBase + "/" + dir;
  std::string::size_type pos;

  pos = path.rfind ('/');
  if (pos == std::string::npos)
    return false;		// root '/' - nonsense

  if (rmdir (path.c_str ()) != 0)
    return false;

  return true;
}



bool TCacheSystem::FileCreate (const std::string & name, int mode) const
{
  std::string path = fAbsoluteBase + "/" + name;
  std::string::size_type pos;
  int dirmode = S_IFDIR | mode;

  if ((mode & S_IRUSR) != 0)
    dirmode |= S_IXUSR;

  if ((mode & S_IRGRP) != 0)
    dirmode |= S_IXGRP;

  if ((mode & S_IROTH) != 0)
    dirmode |= S_IXOTH;

  pos = path.rfind ('/');
  if (pos == std::string::npos)
    return false;		// root '/' - nonsense

  // now create the directory
  std::string dirpart = path;

  pos = path.find ('/', 1);
  while (pos != std::string::npos)
  {
    if (pos == path.length ())		// trailing slash - skip it
      break;

    dirpart.erase (pos);

    struct stat info;

    if (lstat (dirpart.c_str (), &info) != 0)
    {
      int result = mkdir (dirpart.c_str (), dirmode);
      if (result != 0)
        return false;
    }
    else
      if (!S_ISDIR (info.st_mode))
        return false;

    dirpart = path;
    pos = path.find ('/', pos + 1);
  };

  int fd = creat (path.c_str (), S_IFREG | mode);
  if (fd < 0)
    return false;

  close (fd);

  return true;
}



bool TCacheSystem::FileDelete (const std::string & name) const
{
  std::string path = fAbsoluteBase + "/" + name;
  std::string::size_type pos;

  pos = path.rfind ('/');
  if (pos == std::string::npos)
    return false;		// root '/' - nonsense

  if (unlink (path.c_str ()) != 0)
    return false;

  return true;
}



bool TCacheSystem::FileAttribs (const std::string & path, TFileData & data) const
{
  std::string fullpath = fAbsoluteBase + "/" + path;
  struct stat info;

  if (lstat (fullpath.c_str (), &info) != 0)
    return false;

  data.SetSize (info.st_size);
  data.SetAttribute (info.st_mode);
  data.SetAtime (info.st_atime);
  data.SetMtime (info.st_mtime);
  data.SetCtime (info.st_ctime);

  return true;
}



TEntry * TCacheSystem::AddDir (TDirectory & root,
			       const std::string & name,
                               TFileData & data) const
{
  TEntry *entry = root.FindEntry (name);
  if (entry)
  {
    if ((entry->isA () == TEntry::DirEntry) &&
        (entry->GetLayer () < fLayer))
      return entry;	// keep directory entries of lower layers

    root.RemoveEntry (entry);	// hide (overwrite) old entry
  }

  TDirectory * dir = new TDirectory (name);
  if (!dir)
    return 0;

  dir->SetLayer (fLayer);
  dir->SetData (data);

  root.AddEntry (dir);

  return dir;
}



TEntry * TCacheSystem::AddFile (TDirectory & root,
				const std::string & name,
				TFileData & data) const
{
  TEntry *entry = root.FindEntry (name);
  if (entry)
  {
    if (entry->GetLayer () > fLayer)
      return entry;		// keep higher layers entries

    root.RemoveEntry (entry);	// hide (overwrite) old entry
  }

  TFile * file = new TFile (name, "");
  if (!file)
    return 0;

  file->SetLayer (fLayer);
  file->SetData (data);

  root.AddEntry (file);

  return file;
}
