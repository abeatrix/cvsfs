/***************************************************************************
                          TModuleActionMkdir.h  -  description
                             -------------------
    begin                : Mon Aug 19 2002
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

#ifndef __TMODULEACTIONMKDIR_H__
#define __TMODULEACTIONMKDIR_H__

#include "TModuleAction.h"



class TModuleActionMkdir : public TModuleAction
{
  public:
    TModuleActionMkdir ();
    virtual ~TModuleActionMkdir ();

    virtual bool doit (TCvsInterface &);
};



#endif
