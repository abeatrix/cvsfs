/***************************************************************************
                          TCachedFile.cpp  -  description
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

#include "TCachedFile.h"

#include <sys/stat.h>
#include <fstream.h>



TCachedFile::TCachedFile (const std::string & dir, const std::string & name)
: fDir (dir), fName (name)
{
}



bool TCachedFile::HaveFile () const
{
  return FileExist ();
}



std::ostream * TCachedFile::OpenForWrite () const
{
  if (!HaveFile ())
    if (!DirExist ())
      if (!CreateDir ())
        return 0;

  return new std::ofstream ((fDir + "/" + fName).c_str (), ios::out, S_IRUSR | S_IWUSR);
}



std::istream * TCachedFile::OpenForRead () const
{
  if (!HaveFile ())
    return 0;

  return new std::ifstream ((fDir + "/" + fName).c_str ());
}



bool TCachedFile::DirExist () const
{
  struct stat info;

  if (lstat (fDir.c_str (), &info) != 0)
    return false;

  return S_ISDIR (info.st_mode);
}



bool TCachedFile::FileExist () const
{
  std::string fullpath = fDir + "/" + fName;
  struct stat info;

  if (lstat (fullpath.c_str (), &info) != 0)
    return false;

  return S_ISREG (info.st_mode);
}



bool TCachedFile::CreateDir () const
{
  std::string::size_type pos;
  std::string dirpart;

  pos = 0;
  do
  {
    pos = fDir.find ('/', pos + 1);

    dirpart = fDir;
    if (pos != std::string::npos)
      dirpart.erase (pos);

    struct stat info;

    if (lstat (dirpart.c_str (), &info) != 0)
    {
      int result = mkdir (dirpart.c_str (), S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR);
      if (result != 0)
        return false;
    }
    else
      if (!S_ISDIR (info.st_mode))
        return false;

  } while (pos != std::string::npos);

  return true;
}
