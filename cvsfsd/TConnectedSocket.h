/***************************************************************************
                          TConnectedSocket.h  -  description
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

#ifndef __CONNECTEDSOCKET_H__
#define __CONNECTEDSOCKET_H__

//#include <arpa/inet.h>
//#include <sys/socket.h>
//#include <sys/types.h>
#include <netinet/in.h>



class TConnectedSocket
{
  public:
    TConnectedSocket(int, sockaddr_in &);
    ~TConnectedSocket();

    int Send(const void *, int);
    int Recv(void *, int);

    char *GetIP();

    bool Close();

  private:
    int sockfd;
    sockaddr_in address;
};


#endif
