/***************************************************************************
                          XPserverTimeout.h  -  description
                             -------------------
    begin                : Fri Aug 16 2002
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

#ifndef __XPSERVERTIMEOUT_H__
#define __XPSERVERTIMEOUT_H__

#include "XException.h"



class XPserverTimeout : public XException
{
  public:
    XPserverTimeout () : XException () {};

  protected:
    virtual void StreamData (std::ostream &);
};



#endif
