/***************************************************************************
                          inode.c  -  description
                             -------------------
    begin                : Fri Apr 6 2001
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

#include "cvsfs_config.h"
#include "inode.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>

//#include "socket.h"
#include "dir.h"
#include "file.h"
//#include "cache.h"
//#include "proc.h"
//#include "procfs.h"
//#include "devfs.h"

#define CVSFS_ROOT_INODE	1



/* local forward references */
static void cvsfs_set_inode_attr (struct inode *, struct cvsfs_fattr *);



struct inode *
cvsfs_iget (struct super_block *sb, struct cvsfs_fattr *fattr)
{
  struct inode *inode = new_inode (sb);
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: cvsfs_iget\n");
#endif
  if (!inode)
    return NULL;

  inode->i_ino = fattr->f_ino;
  cvsfs_set_inode_attr (inode, fattr);

  switch (inode->i_mode & S_IFMT)
  {
    case S_IFREG:			/* regular file */
      printk (KERN_DEBUG "cvsfs: regular file\n");
      inode->i_op = &cvsfs_file_inode_operations;
      inode->i_fop = &cvsfs_file_operations;
      inode->i_data.a_ops = &cvsfs_file_aops;
      break;

    case S_IFDIR:			/* directory */
      printk (KERN_DEBUG "cvsfs: directory\n");
      inode->i_op = &cvsfs_dir_inode_operations;
      inode->i_fop = &cvsfs_dir_operations;
      break;

//    case S_IFLNK:*/			/* symlink */
//      inode->i_op = &cvsfs_symlink_inode_operations;
//      break;

    default:
//        init_special_inode (inode, mode, dev);
      inode->i_fop = &cvsfs_file_operations;
  }

  insert_inode_hash (inode);

  return inode;
}



static void
cvsfs_set_inode_attr (struct inode *dest, struct cvsfs_fattr *src)
{
  dest->i_mode		= src->f_mode;
  dest->i_nlink		= src->f_nlink;
  dest->i_uid		= src->f_uid;
  dest->i_gid		= src->f_gid;
  dest->i_rdev		= src->f_rdev;
  dest->i_ctime		= src->f_ctime;
  dest->i_atime		= src->f_atime;
  dest->i_mtime		= src->f_mtime;
  dest->i_blksize	= src->f_blksize;
  dest->i_blocks	= src->f_blocks;
  dest->i_size		= src->f_size;
  dest->u.generic_ip	= NULL;
  
  if (src->f_version != NULL)
  {
    dest->u.generic_ip = kmalloc (strlen (src->f_version) + 1, GFP_KERNEL);
    if (dest->u.generic_ip == NULL)
      printk (KERN_ERR "cvsfs cvsfs_set_inode_attr: memory squeeze\n");
    else
      strcpy (dest->u.generic_ip, src->f_version);
  }
}



void
cvsfs_delete_inode (struct inode * inode)
{
  lock_kernel ();

  clear_inode (inode);

  unlock_kernel ();
}



void
cvsfs_clear_inode (struct inode * inode)
{
#ifdef __DEBUG__
  printk (KERN_ERR "cvsfs cvsfs_clear_inode\n");
#endif
  if (inode->u.generic_ip != NULL)
    kfree ((char *) inode->u.generic_ip);	/* free version number if any */
}
