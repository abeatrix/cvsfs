/***************************************************************************
                          XException.h  -  description
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

#ifndef __XEXCEPTION_H__
#define __XEXCEPTION_H__

#include <iostream>



class XException
{
  public:
    XException () {};
    virtual ~XException ();

    friend std::ostream & operator << (std::ostream &, XException &);

  protected:
    virtual void StreamData (std::ostream &) = 0;
};



#endif
