/***************************************************************************
                          TCvsPserverCommandTree.h  -  description
                             -------------------
    begin                : Wed Oct 2 2002
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

#ifndef __TCVSPSERVERCOMMANDTREE_H__
#define __TCVSPSERVERCOMMANDTREE_H__

#include "TCvsPserverCommand.h"
#include <string>
#include <iostream>

// forward reference
class TDirectory;



class TCvsPserverCommandTree : public TCvsPserverCommand
{
  public:
    TCvsPserverCommandTree (const std::string &, TDirectory &);
    virtual ~TCvsPserverCommandTree ();

    virtual bool execute (TCvsSessionPserver &);

  private:
    std::string		fRootPath;
    TDirectory		&fTree;
    TDirectory		*fOldTree;
    TDirectory		*fCurrentDir;
    TDirectory		*fCurrentOldDir;
    std::string		fCurrentPath;

    virtual bool processLine (TCvsSessionPserver &, const std::string &);
};



#endif
