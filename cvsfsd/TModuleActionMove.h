/***************************************************************************
                          TModuleActionMove.h  -  description
                             -------------------
    begin                : Wed Sep 11 2002
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

#ifndef __TMODULEACTIONMOVE_H__
#define __TMODULEACTIONMOVE_H__

#include "TModuleAction.h"



class TModuleActionMove : public TModuleAction
{
  public:
    TModuleActionMove ();
    virtual ~TModuleActionMove ();

    virtual bool doit (TCvsInterface &);
};



#endif
