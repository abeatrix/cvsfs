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
// #include <linux/proc_fs.h>
#include <linux/in.h>
#include <asm/semaphore.h>

#define CVSFS_MAXPATHLEN	1024
#define CVSFS_MAXVERLEN		16
#define CVSFS_MAXPROJECT	64
#define CVSFS_MAX_USER		64
#define CVSFS_MAX_PASS		64
#define IPV4_MAX_ADDR		16



struct cvsfs_mount_data
{
  int			version;
  __kernel_uid_t	uid;
  __kernel_gid_t	gid;
  __kernel_mode_t	file_mode;
  __kernel_mode_t	dir_mode;
  char			server[IPV4_MAX_ADDR];
  char			root[CVSFS_MAXPATHLEN];
  char			project[CVSFS_MAXPROJECT];
  char			mountpoint[CVSFS_MAXPATHLEN];
};

struct cvsfs_sb_info
{
  struct sockaddr_in		address;
  struct cvsfs_mount_data	mnt;
  char				user[CVSFS_MAX_USER];
  char				pass[CVSFS_MAX_PASS];
  char				cachedir[CVSFS_MAXPATHLEN];
  __kernel_uid_t		mount_uid;
  __kernel_gid_t		mount_gid;
  struct semaphore		sem;
};

struct cvsfs_versioninfo
{
  char          *version;
};

struct cvsfs_fattr
{
  unsigned long			f_ino;
  umode_t			f_mode;
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
  struct cvsfs_versioninfo	f_info;
};



void cvsfs_init_vfs ();
struct super_block * cvsfs_read_super (struct super_block *, void *, int);
struct inode * cvsfs_iget (struct super_block *, struct cvsfs_fattr *);



#endif
