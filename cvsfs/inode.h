/***************************************************************************
                          inode.h  -  description
                             -------------------
    begin                : Sat Apr 7 2001
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

#ifndef __INODE_H__
#define __INODE_H__

#include <linux/fs.h>

/* forward references */
struct cvsfs_fattr;



struct inode * cvsfs_iget (struct super_block *, struct cvsfs_fattr *);
void cvsfs_delete_inode (struct inode *);
void cvsfs_clear_inode (struct inode *);

#endif
