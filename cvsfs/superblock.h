/***************************************************************************
                          superblock.h  -  description
                             -------------------
    begin                : Mon May 27 2002
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

#ifndef __SUPERBLOCK_H__
#define __SUPERBLOCK_H__

//#include <linux/time.h>
//#include <linux/types.h>
//#include <linux/kernel.h>
#include <linux/fs.h>
// #include <linux/proc_fs.h>
#include <linux/in.h>
#include <asm/semaphore.h>
#include "devfs.h"		/* definition of struct device_info */
#include "procfs.h"		/* definition of struct procfs_info */
#include "proc.h"

/* forward references */
//struct semaphore;
//struct cvsfs_device_info;
//struct cvsfs_procfs_info;
//struct cvsfs_proc_info;



struct cvsfs_connection_info
{
  struct sockaddr_in	address;
  char			*user;
  char			*pass;
  char			*root;
  char			*server;
  char			*project;
};

struct cvsfs_mount_info
{
  __kernel_uid_t	uid;		/* files presented with this uid */
  __kernel_gid_t	gid;		/* files presended with this gid */
  __kernel_mode_t	file_mode;
  __kernel_mode_t	dir_mode;
  char			*mountpoint;
};

struct cvsfs_sb_info
{
  struct cvsfs_sb_info		*next;
  struct cvsfs_sb_info		*prev;
  struct cvsfs_connection_info	connection;
  struct cvsfs_mount_info	mount;
  struct cvsfs_device_info	device;		/* /dev relevant data */
  struct cvsfs_procfs_info	procfs;		/* /proc relevant data */
  struct cvsfs_proc_info	proc;
  __kernel_uid_t		mount_uid;	/* mount issued by this uid */
  __kernel_gid_t		mount_gid;	/* mount issued by this gid */
  int				id;
  char				idstring[16];
  struct semaphore		sem;
  unsigned long			blocksize;
  unsigned char			blocksize_bits;
  char				*parm_string;
};



/* invoked when a new mount is to be established */
struct super_block * cvsfs_read_super (struct super_block *, void *, int);

/* gain access to list of superblocks managed by this kernel module */
/* returns a pointer to the first entry of super blocks             */
struct cvsfs_sb_info * cvsfs_read_lock_superblock_list (void);

/* release access to list of superblocks managed by this kernel module */
void cvsfs_read_unlock_superblock_list (void);



#endif
