/***************************************************************************
                          TModuleServer.h  -  description
                             -------------------
    begin                : Sun Jun 9 18:32:49 CEST 2002
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

#ifndef __TMODULESERVER_H__
#define __TMODULESERVER_H__

#include <string>
#include <map>
#include <stdio.h>
#include "TModuleAction.h"
#include "TMountParameters.h"

// forward reference
class TCvsInterface;



class TModuleServer
{
  public:
    TModuleServer ();
    TModuleServer (const std::string &);
    ~TModuleServer ();

    void AddAction (const std::string &, TModuleAction *);

    bool ready ();
    void run (TCvsInterface &);

    void write (const char *, int);
    int read (char *, int, int = 0);
    int readLine (char *, int);
    int readItem (char *, int);

    const TMountParameters & parameters () const { return *fMountParameters; }

  private:
    void init ();

    typedef std::map<std::string, TModuleAction *> MapType;
    typedef MapType::value_type ValuePair;

    std::string fDevice;
    MapType fActions;
    FILE *fInDeviceFile;
    FILE *fOutDeviceFile;
    TMountParameters *fMountParameters;
    bool fInitSequence;
};



#endif
