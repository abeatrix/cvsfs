/***************************************************************************
                          TSocket.cpp  -  description
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "TSocket.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



TSocket::~TSocket()
{
  Close ();
}



bool TSocket::Create()
{
  sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
    return false;

  return true;
};



bool TSocket::Create (int port)
{
  if (!Create ())
    return false;
   
  address.sin_family = AF_INET;
  address.sin_port = htons (port);
  address.sin_addr.s_addr = INADDR_ANY;
  bzero (&(address.sin_zero), 8);
   
  if (bind (sockfd, (struct sockaddr *) &address, sizeof (struct sockaddr)) == -1)
    return false;

  return true;
}



TConnectedSocket *TSocket::Connect (int port, const char *hostName)
{
  hostent *host;
  TConnectedSocket *conn;
	
  if ((host = gethostbyname (hostName)) == NULL)
    return 0;

  if (sockfd == 0)
  {
    if (!Create ())
      return 0;
  }

  address.sin_family = AF_INET;
  address.sin_port = htons (port);
  address.sin_addr = *((struct in_addr *) host->h_addr);
  bzero (&(address.sin_zero), 8);

  if (connect (sockfd, (struct sockaddr *) &address, sizeof (struct sockaddr)) == -1)
    return 0;

  conn = new TConnectedSocket (sockfd, address);

  Create ();

  return conn;
}



bool TSocket::Close ()
{
  int result;

  if (sockfd != 0)
  {
    result = close (sockfd);
    if (result == -1)
      return false;
  }

  return true;
}



bool TSocket::Listen ()
{
  if (listen (sockfd, 5) == -1)
    return false;
   
  return true;
}



TConnectedSocket *TSocket::Accept ()
{
  int newfd;
  struct sockaddr_in remote;
  size_t sin_size = sizeof (struct sockaddr_in);
  
  //sockaddr_in RemoteAddress;
  if ((newfd = accept (sockfd, (struct sockaddr *) &remote, &sin_size)) == -1)
    return 0;
  
  return new TConnectedSocket (newfd, remote);
}
