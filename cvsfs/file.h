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
#include "superblock.h"



extern struct file_operations cvsfs_file_operations;
extern struct inode_operations cvsfs_file_inode_operations;
extern struct address_space_operations cvsfs_file_aops;


struct cvsfs_fattr
{
  unsigned long	f_ino;
  umode_t	f_mode;
  nlink_t			f_nlink;
  uid_t				f_uid;
  gid_t				f_gid;
  kdev_t			f_rdev;
  off_t				f_size;
  time_t			f_atime;
  time_t			f_mtime;
  time_t			f_ctime;
  unsigned long			f_blksize;
  unsigned long			f_blocks;
  char				*f_version;
};



void cvsfs_init_root_dirent (struct cvsfs_sb_info *, struct cvsfs_fattr *);


#endif
