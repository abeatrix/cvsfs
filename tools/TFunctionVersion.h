/***************************************************************************
                          TFunctionVersion.h  -  description
                             -------------------
    begin                : Thu Sep 5 05:34:27 CEST 2002
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

#ifndef __TFUNCTIONVERSION_H__
#define __TFUNCTIONVERSION_H__

#include "TFunction.h"



class TFunctionVersion : public TFunction
{
  public:
    TFunctionVersion ();

    virtual bool DoIt (int, char * []);
};



#endif
