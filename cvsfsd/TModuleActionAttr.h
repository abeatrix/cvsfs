/***************************************************************************
                          TModuleActionAttr.h  -  description
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

#ifndef __TMODULEACTIONATTR_H__
#define __TMODULEACTIONATTR_H__

#include "TModuleAction.h"



class TModuleActionAttr : public TModuleAction
{
  public:
    TModuleActionAttr ();
    virtual ~TModuleActionAttr ();

    virtual bool doit (TCvsInterface &);
};



#endif
