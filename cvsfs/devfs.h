/***************************************************************************
                          devfs.h  -  cvsfs devfs interface
                             -------------------
    begin                : Thu Nov 29 2001
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

#ifndef __DEVFS_H__
#define __DEVFS_H__

//include <linux/fs.h>			/* char dev definitions here */

// forward reference
struct cvsfs_sb_info;



int cvsfs_devfs_init ();
void cvsfs_devfs_cleanup ();
int cvsfs_devfs_user_init (struct cvsfs_sb_info *);
void cvsfs_devfs_user_cleanup (struct cvsfs_sb_info *);

int cvsfs_devfs_get_major ();



#endif
