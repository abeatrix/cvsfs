/***************************************************************************
                          file.h  -  description
                             -------------------
    begin                : Thu May 17 2001
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

#ifndef __FILE_H__
#define __FILE_H__

#include <linux/fs.h>



extern struct file_operations cvsfs_file_operations;
extern struct inode_operations cvsfs_file_inode_operations;
extern struct address_space_operations cvsfs_file_aops;



#endif
