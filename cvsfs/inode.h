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

//#define __KERNEL__

//#include <linux/time.h>
//#include <linux/types.h>
//#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/in.h>
#include <asm/semaphore.h>

#define CVSFS_MAXPATHLEN	1024
#define CVSFS_MAX_USER		64
#define CVSFS_MAX_PASS		64



struct cvsfs_mount_data
{
  int			version;
  __kernel_uid_t	uid;
  __kernel_gid_t	gid;
  __kernel_mode_t	file_mode;
  __kernel_mode_t	dir_mode;
  char			root[CVSFS_MAXPATHLEN];
  char			project[CVSFS_MAXPATHLEN];
};

struct cvsfs_sb_info
{
  struct sockaddr_in		address;
  struct cvsfs_mount_data	mnt;
  char				user[CVSFS_MAX_USER];
  char				pass[CVSFS_MAX_PASS];
  struct semaphore		sem;
  struct socket			*sock;
};

struct cvsfs_fattr
{
  unsigned long	f_ino;
  umode_t	f_mode;
  nlink_t	f_nlink;
  uid_t		f_uid;
  gid_t		f_gid;
  kdev_t	f_rdev;
  off_t		f_size;
  time_t	f_atime;
  time_t	f_mtime;
  time_t	f_ctime;
  unsigned long	f_blksize;
  unsigned long	f_blocks;
};



struct super_block * cvsfs_read_super (struct super_block *, void *, int);
struct inode * cvsfs_iget (struct super_block *, struct cvsfs_fattr *);



#endif
