/***************************************************************************
                          TModuleActionPut.h  -  description
                             -------------------
    begin                : Thu Aug 22 2002
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

#ifndef __TMODULEACTIONPUT_H__
#define __TMODULEACTIONPUT_H__

#include "TModuleAction.h"



class TModuleActionPut : public TModuleAction
{
  public:
    TModuleActionPut ();
    virtual ~TModuleActionPut ();

    virtual bool doit (TCvsInterface &);
};



#endif
