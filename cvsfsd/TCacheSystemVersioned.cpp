/***************************************************************************
                          TCacheSystemVersioned.cpp  -  description
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

#include "TCacheSystemVersioned.h"

//#include <fstream.h>		// already defined in TCacheSystem.h



const std::string BaseDir = "/remote";
const std::string VersionDelimiter = "@@";



TCacheSystemVersioned::TCacheSystemVersioned (const std::string & base)
: TCacheSystem (BaseDir + base, true)
{
}



bool TCacheSystemVersioned::LoadTree (TDirectory & root) const
{
  return true;		// never load the tree
}



bool TCacheSystemVersioned::HaveFile (const std::string & path) const
{
  std::string::size_type pos;

  if ((pos = path.rfind (VersionDelimiter)) == std::string::npos)
    return false;		// manage only versioned files

  return FileExists (path);
}



bool TCacheSystemVersioned::HaveDirectory (const std::string & dir) const
{
  return DirExists (dir);
}



bool TCacheSystemVersioned::CreateDirectory (const std::string & dir, int mode) const
{
  return DirCreate (dir, mode);
}



bool TCacheSystemVersioned::CreateFile (const std::string & path, int mode) const
{
  std::string::size_type pos;

  if ((pos = path.rfind (VersionDelimiter)) == std::string::npos)
    return false;		// manage only versioned files

  return FileCreate (path, mode);
}



bool TCacheSystemVersioned::DeleteDirectory (const std::string & dir) const
{
  return DirDelete (dir);
}



bool TCacheSystemVersioned::DeleteFile (const std::string & path) const
{
  std::string::size_type pos;

  if ((pos = path.rfind (VersionDelimiter)) == std::string::npos)
    return false;		// manage only versioned files

  return FileDelete (path);
}



bool TCacheSystemVersioned::Move (const std::string & from, const TCacheSystem & system, const std::string & to) const
{
  std::string::size_type pos;

  if ((pos = from.rfind (VersionDelimiter)) == std::string::npos)
    return false;		// manage only versioned files

  return MoveItem (from, system, to);
}



bool TCacheSystemVersioned::FileAttribute (const std::string & path, TFileData & data) const
{
  std::string::size_type pos;

  if ((pos = path.rfind (VersionDelimiter)) == std::string::npos)
    return false;		// manage only versioned files

  bool dummy;

  return FileAttribs (path, data, dummy);
}



std::ifstream * TCacheSystemVersioned::In (const std::string & path, int mode) const
{
  // Prerequisite: File must exist
  // must be created previously by a call to CreateFile

  if (!HaveFile (path))
    return 0;

  return new std::ifstream ((fAbsoluteBase + "/" + path).c_str (), mode | ios::in);
}



std::ofstream * TCacheSystemVersioned::Out (const std::string & path, int mode) const
{
  // Prerequisite: File must exist
  // must be created previously by a call to CreateFile

  if (!HaveFile (path))
    return 0;

  return new std::ofstream ((fAbsoluteBase + "/" + path).c_str (), mode | ios::out);
}
