/***************************************************************************
                          TCvsPserverCommand.h  -  description
                             -------------------
    begin                : Thu Sep 26 2002
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

#ifndef __TCVSPSERVERCOMMAND_H__
#define __TCVSPSERVERCOMMAND_H__

#include <string>

// forward reference
class TCvsSessionPserver;



class TCvsPserverCommand
{
  public:
    TCvsPserverCommand () {}
    virtual ~TCvsPserverCommand ();

    virtual bool execute (TCvsSessionPserver &) = 0;

  protected:
    bool processData (TCvsSessionPserver &);
    virtual bool processLine (TCvsSessionPserver &, const std::string &) = 0;

    int CvsAttr2SysAttr (const std::string &);
    void SysAttr2CvsAttr (int, std::string &);
};



#endif
