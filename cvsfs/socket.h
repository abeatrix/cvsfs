/***************************************************************************
                          socket.h  -  description
                             -------------------
    begin                : Thu Nov 8 2001
    copyright            : (C) 2001 by Petric Frank
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

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <linux/net.h>
#include <linux/in.h>



int sock_send (struct socket *, const void *, int);
int sock_recv (struct socket *, unsigned char *, int, unsigned);
int cvsfs_read_raw_data (struct socket *, unsigned long, char *);
int cvsfs_readline (struct socket *, char *, int);
int cvsfs_long_readline (struct socket *, char *, int);
int cvsfs_execute (struct socket *, char *);
int cvsfs_execute_command (struct socket *, ...);
int cvsfs_login (struct socket *, char *, char *, char *, int);
int cvsfs_connect (struct socket **, char *, char *, char *, struct sockaddr_in, int);
void cvsfs_disconnect (struct socket **);



#endif
