/***************************************************************************
                          TCacheSystemSimple.cpp  -  description
                             -------------------
    begin                : Wed Aug 28 2002
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

#include "TCacheSystemSimple.h"

//#include <fstream.h>		// already defined in TCacheSystem.h



const std::string BaseDir = "/local";



TCacheSystemSimple::TCacheSystemSimple (const std::string & base, bool readOnly)
: TCacheSystem (BaseDir + base, readOnly)
{
}



bool TCacheSystemSimple::LoadTree (TDirectory & root) const
{
  return TreeLoad (root, "");
}



bool TCacheSystemSimple::HaveFile (const std::string & path) const
{
  return FileExists (path);
}



bool TCacheSystemSimple::HaveDirectory (const std::string & dir) const
{
  return DirExists (dir);
}



bool TCacheSystemSimple::CreateDirectory (const std::string & dir, int mode) const
{
  return DirCreate (dir, mode);
}



bool TCacheSystemSimple::CreateFile (const std::string & path, int mode) const
{
  return FileCreate (path, mode);
}



bool TCacheSystemSimple::DeleteDirectory (const std::string & dir) const
{
  return DirDelete (dir);
}



bool TCacheSystemSimple::DeleteFile (const std::string & path) const
{
  return FileDelete (path);
}



bool TCacheSystemSimple::Move (const std::string & from, const TCacheSystem & system, const std::string & to) const
{
  return MoveItem (from, system, to);
}



bool TCacheSystemSimple::FileAttribute (const std::string & path, TFileData & data) const
{
  bool dummy;

  return FileAttribs (path, data, dummy);
}



std::ifstream * TCacheSystemSimple::In (const std::string & path, int mode) const
{
  // Prerequisite: File must exist
  // must be created previously by a call to CreateFile

  if (!HaveFile (path))
    return 0;

  return new std::ifstream ((fAbsoluteBase + "/" + path).c_str (), mode | ios::in);
}



std::ofstream * TCacheSystemSimple::Out (const std::string & path, int mode) const
{
  // Prerequisite: File must exist
  // must be created previously by a call to CreateFile

  if (!HaveFile (path))
    return 0;

  return new std::ofstream ((fAbsoluteBase + "/" + path).c_str (), mode | ios::out);
}
