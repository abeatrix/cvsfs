/***************************************************************************
                          dir.h  -  description
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

#ifndef __DIR_H__
#define __DIR_H__

#include <linux/fs.h>



struct cvsfs_dir_entry
{
  umode_t       mode;
  char          *name;
  off_t         size;
  nlink_t       nlink;
  unsigned long blocksize;
  unsigned long blocks;
  time_t        date;
  char          *version;
};

		

extern struct file_operations cvsfs_dir_operations;
extern struct inode_operations cvsfs_dir_inode_operations;



#endif
