/***************************************************************************
                          TFileData.cpp  -  description
                             -------------------
    begin                : Thu Jun 14 22:44:11 CEST 2002
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

#include "TFileData.h"

#include <sys/stat.h>



TFileData::TFileData ()
: fSize (0), fAttrib (0), fAtime (0), fMtime (0), fCtime (0)
{
}



TFileData::TFileData (unsigned long size, int mode,
                      int atime, int mtime, int ctime)
: fSize (size), fAttrib (mode & (S_IRWXU | S_IRWXG | S_IRWXO)),
  fAtime (atime), fMtime (mtime), fCtime (ctime)
{
}



TFileData::TFileData (const TFileData & clone)
: fSize (clone.fSize), fAttrib (clone.fAttrib),
  fAtime (clone.fAtime), fMtime (clone.fMtime), fCtime (clone.fCtime)
{
}



void TFileData::SetAttribute (int attr)
{
  fAttrib = attr & (S_IRWXU | S_IRWXG | S_IRWXO);
}



void TFileData::streamData (std::ostream & str, int filetype, bool readonly) const
{
  int attr = filetype | fAttrib;

  if (readonly)
    attr &= ~(S_IWUSR | S_IWGRP | S_IWOTH);

  str << attr << " "
      << fSize << " "
      << fAtime << " "
      << fMtime << " "
      << fCtime;
}
