/***************************************************************************
                          TConnectedSocket.cpp  -  description
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

#include "TConnectedSocket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	// RedHat 7.2 requires this to provide 'memcpy'



TConnectedSocket::TConnectedSocket(int fd, sockaddr_in & addr)
: sockfd (fd)
{
  memcpy (&address, &addr, sizeof (struct sockaddr_in));
}



TConnectedSocket::~TConnectedSocket()
{
  Close ();
}



int TConnectedSocket::Send (const void *message, int length)
{
  int bytesSent;
  fd_set rset;
  struct timeval tv;

  FD_ZERO (&rset);
  FD_SET (sockfd, &rset);
  tv.tv_sec = 20;
  tv.tv_usec = 0;

  select (sockfd + 1, NULL, &rset, NULL, &tv);

  if (FD_ISSET (sockfd, &rset))
    bytesSent = send (sockfd, message, length, 0);
  else
    bytesSent = -1;

  return bytesSent;
}



int TConnectedSocket::Recv (void *buffer, int length)
{
  int bytesRecv;
  fd_set rset;
  struct timeval tv;

  FD_ZERO (&rset);
  FD_SET (sockfd, &rset);
  tv.tv_sec = 10;
  tv.tv_usec = 0;

  select (sockfd + 1, &rset, NULL, NULL, &tv);

  if (FD_ISSET (sockfd, &rset))
    bytesRecv = recv (sockfd, buffer, length, 0);
  else
    bytesRecv = -1;

  return bytesRecv;
}



bool TConnectedSocket::Close ()
{
  int result;
  
  result = shutdown (sockfd, SHUT_RDWR);

  if (result == -1) 
    return false;

  return true;
}



char *TConnectedSocket::GetIP ()
{
  return inet_ntoa (address.sin_addr);
}
