/***************************************************************************
                          TCache.cpp  -  description
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

#include "TCache.h"

#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <unistd.h>



TCache::TCache (const std::string & root, bool cleanOnExit)
: fRoot (root), fCleanOnExit (cleanOnExit)
{
  // create base dir - only root is allowed to work here
  MakeDir ("", S_IRUSR | S_IWUSR | S_IXUSR | S_ISGID);
}



TCache::~TCache ()
{
  if (fCleanOnExit)
  {
    // remove all files below 'fRoot'

    // rmdir (fRoot);
  }
}



void TCache::GetPath (const std::string & path, std::string & res)
{
  res = fRoot + res;
}



bool TCache::MakeDir (const std::string & path, int mode)
{
  std::string dir = fRoot + path;
  std::string::size_type pos;
  int dirmode = S_IFDIR | mode;

  if ((mode & S_IRUSR) != 0)
    dirmode |= S_IXUSR;

  if ((mode & S_IRGRP) != 0)
    dirmode |= S_IXGRP;

  if ((mode & S_IROTH) != 0)
    dirmode |= S_IXOTH;

  pos = dir.rfind ('/');
  if (pos == std::string::npos)
    return false;		// root '/' - nonsense

  // now create the directory
  std::string dirpart;

  pos = 0;
  do
  {
    pos = dir.find ('/', pos + 1);

    if (pos == dir.length ())		// trailing slash - skip it
      break;

    dirpart = dir;
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



int TCache::RemoveDir (const std::string & path)
{
  std::string dir = fRoot + path;
  std::string::size_type pos;

  pos = dir.rfind ('/');
  if (pos == std::string::npos)
    return EIO;		// root '/' - nonsense

  if (rmdir (dir.c_str ()) != 0)
    return errno;

  return 0;
}



bool TCache::TouchFile (const std::string & path, int mode)
{
  std::string dir = fRoot + path;
  std::string::size_type pos;
  int dirmode = S_IFDIR | mode;

  if ((mode & S_IRUSR) != 0)
    dirmode |= S_IXUSR;

  if ((mode & S_IRGRP) != 0)
    dirmode |= S_IXGRP;

  if ((mode & S_IROTH) != 0)
    dirmode |= S_IXOTH;

  pos = dir.rfind ('/');
  if (pos == std::string::npos)
    return false;		// root '/' - nonsense

  // now create the directory
  std::string dirpart = dir;

  pos = dir.find ('/', 1);
  while (pos != std::string::npos)
  {
    if (pos == dir.length ())		// trailing slash - skip it
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

    dirpart = dir;
    pos = dir.find ('/', pos + 1);
  };

  int fd = creat (dir.c_str (), mode);
  if (fd < 0)
    return false;

  close (fd);

  return true;
}



int TCache::DeleteFile (const std::string & path)
{
  std::string dir = fRoot + path;
  std::string::size_type pos;

  pos = dir.rfind ('/');
  if (pos == std::string::npos)
    return EIO;		// root '/' - nonsense

  if (unlink (dir.c_str ()) != 0)
    return errno;

  return 0;
}
