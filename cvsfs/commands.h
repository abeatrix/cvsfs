/***************************************************************************
                          commands.h  -  description
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

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <linux/net.h>



/* forward references for structures */
struct cvsfs_sb_info;



int cvsfs_command_sequence_co (struct socket *, struct cvsfs_sb_info *, char *, char *, char *);
int cvsfs_command_sequence_rdiff (struct socket *, struct cvsfs_sb_info *, char * );



#endif
