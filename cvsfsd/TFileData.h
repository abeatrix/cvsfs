/***************************************************************************
                          TEntryData.h  -  description
                             -------------------
    begin                : Thu Jun 14 22:43:54 CEST 2002
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

#ifndef __TFILEDATA_H__
#define __TFILEDATA_H__

#include <map>
#include <iostream>



class TFileData
{
  public:
    TFileData ();
    TFileData (unsigned long, int, int, int, int);
    TFileData (const TFileData &);
    ~TFileData () {}

    const unsigned long GetSize () const { return fSize; }
    const int GetAttribute () const { return fAttrib; }
    const int GetAtime () const { return fAtime; }
    const int GetMtime () const { return fMtime; }
    const int GetCtime () const { return fCtime; }
    void SetSize (unsigned long size) { fSize = size; }
    void SetAttribute (int attr);
    void SetAtime (int time) { fAtime = time; }
    void SetMtime (int time) { fMtime = time; }
    void SetCtime (int time) { fCtime = time; }

    void streamData (std::ostream &, int, bool) const;

  private:
    unsigned long	fSize;
    int			fAttrib;
    int			fAtime;
    int			fMtime;
    int			fCtime;
};



#endif
