/***************************************************************************
                          TCvsConnectionPserver.h  -  description
                             -------------------
    begin                : Sat Aug 10 2002
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

#ifndef __TCVSCONNECTIONPSERVER_H__
#define __TCVSCONNECTIONPSERVER_H__

#include "TCvsConnection.h"

// forward reference
class TCvsSessionPserver;



class TCvsConnectionPserver : public TCvsConnection
{
  public:
    TCvsConnectionPserver (const TMountParameters &);
    ~TCvsConnectionPserver ();

    const std::string & GetProject () const { return fProject; }

//    virtual TCvsSession *Open ();
    TCvsSessionPserver *Open ();

  private:
    TSocket     fSocket;
};



#endif
