/***************************************************************************
                          TCacheWorkFiles.cpp  -  description
                             -------------------
    begin                : Mon Aug 12 2002
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

#include "TCacheWorkFiles.h"

#include <sys/stat.h>
#include <dirent.h>
#include "TCachedFile.h"
#include "TDirectory.h"
#include "TFile.h"



TCacheWorkFiles::TCacheWorkFiles (const std::string & root, const std::string & mount)
: TCache (root + "/local", false), fMountpoint (mount)
{
}



TCachedFile * TCacheWorkFiles::CachedFile (const std::string & path) const
{
  std::string dir = fRoot + fMountpoint + "/" + path;
  std::string::size_type pos;

  pos = dir.rfind ('/');
  if (pos == std::string::npos)
    return 0;			// file to be at root '/' - nonsense

  std::string name = dir;

  dir.erase (pos);
  name.erase (0, pos + 1);

  return new TCachedFile (dir, name);
}



bool TCacheWorkFiles::LoadTree (TDirectory & root)
{
  return LoadDir (root, "");
}



bool TCacheWorkFiles::FileData (const std::string & path, TFileData & data)
{
  std::string fullpath = fRoot + fMountpoint + "/" + path;
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



bool TCacheWorkFiles::MakeDirectory (const std::string & path, int mode)
{
  return MakeDir (fMountpoint + "/" + path, mode);
}



int TCacheWorkFiles::RemoveDirectory (const std::string & path)
{
  return RemoveDir (fMountpoint + "/" + path);
}



bool TCacheWorkFiles::MakeFile (const std::string & path, int mode)
{
  return TouchFile (fMountpoint + "/" + path, mode);
}



// load one directory - called recursive if necessary
bool TCacheWorkFiles::LoadDir (TDirectory & root, const std::string & path)
{
  std::string dir = fRoot + fMountpoint + path;
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
    if (lstat (filepath.c_str (), &info) == 0)
    {
      TEntry *entry = root.FindEntry (filename);
      if (!entry)
      {			// not found - create one
        if (S_ISDIR (info.st_mode))
        {		// we have a directory
          TDirectory *d = new TDirectory (filename);
          if (!d)
          {
            closedir (cachedir);
            return false;
          }

	  TFileData data (info.st_size, info.st_mode,
                          info.st_atime, info.st_mtime, info.st_ctime);
          d->SetData (data);

          entry = d;
        }
        else
        {		// we have a file
          TFile *f = new TFile (filename, "");

          if (!f)
          {
            closedir (cachedir);
            return false;
          }

	  TFileData data (info.st_size, info.st_mode,
                          info.st_atime, info.st_mtime, info.st_ctime);
          f->SetData (data);

          entry = f;
        }

        entry->SetSource (TEntry::Local);

        root.AddEntry (entry);
      }
      else
      {
        TFileData data (info.st_size, info.st_mode,
                          info.st_atime, info.st_mtime, info.st_ctime);

        if (entry->isA () == TEntry::VersionedFileEntry)
        {		// local work copy replaces the remote version
          root.RemoveEntry (entry);

          TFile *f = new TFile (filename, "");

          if (!f)
          {
            closedir (cachedir);
            return false;
          }

          f->SetData (data);

          entry = f;

          entry->SetSource (TEntry::Local);

          root.AddEntry (entry);
        }
        else
          if (entry->isA () == TEntry::FileEntry)	// update info
            static_cast<TFile *> (entry)->SetData (data);
      }

      // we have now a valid entry to the current file/directory
      if (entry->isA () == TEntry::DirEntry)
	// this is a directory - dig into this one recursively
        LoadDir (*static_cast<TDirectory *> (entry), path + "/" + filename);
    }
  }

  closedir (cachedir);

  return true;
}
