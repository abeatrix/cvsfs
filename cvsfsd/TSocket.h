/***************************************************************************
                          TSocket.h  -  description
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

#ifndef __TSOCKET_H__
#define __TSOCKET_H__

#include <netinet/in.h>
#include "TConnectedSocket.h"



class TSocket
{
  public:
    TSocket () : sockfd (0) {}
    ~TSocket ();

    bool Create ();
    bool Create (int);

    bool Listen ();
   
    TConnectedSocket *Accept ();
    TConnectedSocket *Connect (int, const char *);

    bool Close ();

  private:
    int sockfd;
    sockaddr_in address;
};



#endif
