/***************************************************************************
                          TModuleActionGet.h  -  description
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

#ifndef __TMODULEACTIONGET_H__
#define __TMODULEACTIONGET_H__

#include "TModuleAction.h"



class TModuleActionGet : public TModuleAction
{
  public:
    TModuleActionGet ();
    virtual ~TModuleActionGet ();

    virtual bool doit (TCvsInterface &);
};



#endif
