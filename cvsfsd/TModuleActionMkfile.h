/***************************************************************************
                          TModuleActionMkfile.h  -  description
                             -------------------
    begin                : Tue Aug 20 2002
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

#ifndef __TMODULEACTIONMKFILE_H__
#define __TMODULEACTIONMKFILE_H__

#include "TModuleAction.h"



class TModuleActionMkfile : public TModuleAction
{
  public:
    TModuleActionMkfile ();
    virtual ~TModuleActionMkfile ();

    virtual bool doit (TCvsInterface &);
};



#endif
