/***************************************************************************
                          TFileVersions.h  -  description
                             -------------------
    begin                : Thu Jun 14 00:15:41 CEST 2002
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

#ifndef __TFILEVERSIONS_H__
#define __TFILEVERSIONS_H__

#include <string>
#include <map>
#include "TFile.h"



// list of file versions

class TFileVersions
{
  public:
    TFileVersions ();
    TFileVersions (const TFileVersions &);
    ~TFileVersions ();

    TFileVersions & operator = (const TFileVersions &);

    const TFile * FindVersion (const std::string &) const;
    const TFile * GetVersion (int) const;

    void AddVersion (const std::string &, TFile *);
    void RemoveVersion (const std::string &);

  private:
    typedef std::map<std::string, TFile *> MapType;
    typedef MapType::value_type ValuePair;

    MapType	fVersions;
};



#endif
