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
#include <linux/malloc.h>
#include <linux/smp_lock.h>

#include "socket.h"
#include "dir.h"
#include "file.h"
#include "cache.h"
#include "proc.h"
#include "procfs.h"

#define CVSFS_SUPER_MAGIC	0xED79ED79
#define CVSFS_ROOT_INODE	1



/* forward references - super operations */
static void cvsfs_delete_inode (struct inode *);
static void cvsfs_put_super (struct super_block *);
static int cvsfs_statfs (struct super_block *, struct statfs *);
static void cvsfs_clear_inode (struct inode *);

struct super_operations cvsfs_sops = {
					put_inode:	force_delete,
					delete_inode:	cvsfs_delete_inode,
					put_super:	cvsfs_put_super,
					statfs:		cvsfs_statfs,
					clear_inode:	cvsfs_clear_inode,
				     };



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
      inode->i_op = &cvsfs_file_inode_operations;
      inode->i_fop = &cvsfs_file_operations;
      inode->i_data.a_ops = &cvsfs_file_aops;
      break;

    case S_IFDIR:			/* directory */
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
  struct cvsfs_versioninfo *info;

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
  
  if (src->f_info.version != NULL)
  {
    info = kmalloc (sizeof (struct cvsfs_versioninfo), GFP_KERNEL);
    if (info == NULL)
      printk (KERN_ERR "cvsfs cvsfs_set_inode_attr: memory squeeze\n");
    else
    {
      info->version = kmalloc (strlen (src->f_info.version) + 1, GFP_KERNEL);
      if (info->version == NULL)
      {
        printk (KERN_ERR "cvsfs cvsfs_set_inode_attr: memory squeeze\n");
	kfree (info);	// dummy behaviour - don't store version
      }
      else
      {
        strcpy (info->version, src->f_info.version);
	dest->u.generic_ip = info;
      }
    }
  }
}



/* called when a mount is issued - initializes super block */
struct super_block *
cvsfs_read_super (struct super_block *sb, void *data, int silent)
{
  struct inode		*inode;
  struct cvsfs_sb_info	*info;
  struct cvsfs_fattr	root;
  struct socket		*sock;
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: cvsfs_read_super\n");
#endif
  info = kmalloc (sizeof (struct cvsfs_sb_info), GFP_KERNEL);
  if (!info)
  {
    printk (KERN_ERR "cvsfs read_super: can not allocate info block - memory squeeze\n");

    return NULL;
  }

  cvsfs_cache_init ();

  memset (info, 0, sizeof (struct cvsfs_sb_info));

  sb->u.generic_sbp = info;		/* store network info */

  sb->s_blocksize = 1024; /* PAGE_CACHE_SIZE; */
  sb->s_blocksize_bits = 10; /* PAGE_CHACHE_SHIFT; */
  sb->s_magic = CVSFS_SUPER_MAGIC;
/*  sb->s_flags = 0;  */
  sb->s_flags |= MS_RDONLY;
  sb->s_op = &cvsfs_sops;

  info->mnt.version = 1;
  info->mnt.file_mode = S_IRWXU | S_IRGRP | S_IROTH | S_IFREG;
  info->mnt.dir_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IFDIR;
  info->mnt.uid = current->uid;
  info->mnt.gid = current->gid;
  init_MUTEX (&info->sem);

  if (cvsfs_parse_options (info, data) < 0)
    printk (KERN_ERR "cvsfs read_super: invalid options\n");
  else
  {
    sock = NULL;
      
    if (cvsfs_connect (&sock, info->user, info->pass, info->mnt.root, info->address, 1) < 0)
      printk (KERN_ERR "cvsfs read_super: could not connect to %s\n", info->mnt.server);
    else
    {
      cvsfs_init_root_dirent (info, &root);

      inode = cvsfs_iget (sb, &root);
      if (inode)
      {
        sb->s_root = d_alloc_root (inode);

        if (sb->s_root)
	{
	  cvsfs_procfs_user_init (info->mnt.mountpoint, sb);

          cvsfs_cache_get_dir (info, "/", NULL);

	  printk (KERN_INFO "cvsfs: project '%s' mounted\n", info->mnt.project);

          return sb;
	}
      }
      iput (inode);
    }
  }

  printk (KERN_ERR "cvsfs read_super: get root inode failed\n");

  kfree (info);

  return NULL;
}



static void
cvsfs_delete_inode (struct inode * inode)
{
  lock_kernel ();

  clear_inode (inode);

  unlock_kernel ();
}



/* called when a umount is issued - shuts down the super block */
static void
cvsfs_put_super (struct super_block * sb)
{
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) sb->u.generic_sbp;

  cvsfs_cache_empty ();
  
  cvsfs_procfs_user_cleanup (info->mnt.mountpoint);

  printk (KERN_INFO "cvsfs: project '%s' unmounted\n", info->mnt.project);

  kfree (info);
}



static int
cvsfs_statfs (struct super_block * sb, struct statfs * attr)
{
  attr->f_type = CVSFS_SUPER_MAGIC;
  attr->f_bsize = 1024;
  attr->f_blocks = 0;
  attr->f_namelen = CVSFS_MAXPATHLEN;
  attr->f_files = -1;
  attr->f_bavail = -1;

  return 0;
}



static void
cvsfs_clear_inode (struct inode * inode)
{
//  char buff[256];
  
//  strncpy (buff, inode->i_dentry.d_name.name, inode->i_dentry.d_name.len);
//  buff[inode->i_dentry.d_name.len] = '\0';
#ifdef __DEBUG__
  printk (KERN_ERR "cvsfs cvsfs_clear_inode\n");
#endif
  if (inode->u.generic_ip != NULL)
  {
    kfree (((struct cvsfs_versioninfo *) (inode->u.generic_ip))->version);
    kfree ((struct cvsfs_versioninfo *) (inode->u.generic_ip));
    inode->u.generic_ip = NULL;
  }
}
