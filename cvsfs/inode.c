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

#include "inode.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/malloc.h>
#include <linux/smp_lock.h>

#include "dir.h"
#include "file.h"
#include "cache.h"
#include "proc.h"

#define CVSFS_SUPER_MAGIC	0xED79ED79
#define CVSFS_ROOT_INODE	1


/* forward references - super operations */
static void cvsfs_delete_inode (struct inode *);
static void cvsfs_put_super (struct super_block *);
static int cvsfs_statfs (struct super_block *, struct statfs *);

struct super_operations cvsfs_sops = {
					      put_inode:	force_delete,
					      delete_inode:	cvsfs_delete_inode,
					      put_super:	cvsfs_put_super,
					      statfs:		cvsfs_statfs,
					    };



/* forward references - address space operations */
/*static int cvsfs_readpage (struct file *, struct page *);
static int cvsfs_writepage (struct page *);
static int cvsfs_prepare_write (struct file *, struct page *, unsigned, unsigned);
static int cvsfs_commit_write (struct file *, struct page *, unsigned, unsigned);
					    					    					
struct address_space_operations cvsfs_aops = {
						      readpage:		cvsfs_readpage,
						      writepage:	cvsfs_writepage,
						      prepare_write:	cvsfs_prepare_write,
						      commit_write:	cvsfs_commit_write,
						    };
*/

/* local forward references */
static void cvsfs_set_inode_attr (struct inode *, struct cvsfs_fattr *);



struct inode * cvsfs_iget (struct super_block *sb, struct cvsfs_fattr *fattr)
{
  struct inode *inode = new_inode (sb);

  printk (KERN_DEBUG "cvsfs: cvsfs_iget\n");

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
/*
    case S_IFLNK:*/			/* symlink */
/*      inode->i_op = &cvsfs_symlink_inode_operations;
      break;
*/
    default:
/*        init_special_inode (inode, mode, dev); */
      inode->i_fop = &cvsfs_file_operations;
  }

  insert_inode_hash (inode);

  return inode;
}



static void cvsfs_set_inode_attr (struct inode *dest, struct cvsfs_fattr *src)
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
}



struct super_block *cvsfs_read_super (struct super_block *sb, void *data, int silent)
{
  struct inode		*inode;
  struct cvsfs_sb_info	*info;
  struct cvsfs_fattr	root;

  printk (KERN_DEBUG "cvsfs: cvsfs_read_super\n");

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
  info->sock = NULL;
  init_MUTEX (&info->sem);

  if (cvsfs_parse_options (info, data) < 0)
    printk (KERN_ERR "cvsfs read_super: invalid options\n");
  else
  {
    if (cvsfs_connect (info, 1) < 0)
      printk (KERN_ERR "cvsfs read_super: could not connect to %u.%u.%u.%u\n",
                       info->address.sin_addr.s_addr & 0xff,
                       (info->address.sin_addr.s_addr >> 8) & 0xff,
                       (info->address.sin_addr.s_addr >> 16) & 0xff,
                       (info->address.sin_addr.s_addr >> 24) & 0xff);
    else
    {
      cvsfs_init_root_dirent (info, &root);

      inode = cvsfs_iget (sb, &root);
      if (inode)
      {
        sb->s_root = d_alloc_root (inode);

        if (sb->s_root)
          return sb;
      }
      iput (inode);
    }
  }

  printk (KERN_ERR "cvsfs read_super: get root inode failed\n");

  kfree (info);

  return NULL;
}



static void cvsfs_delete_inode (struct inode * inode)
{
  lock_kernel ();

  clear_inode (inode);

  unlock_kernel ();
}



static void cvsfs_put_super (struct super_block * sb)
{
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) sb->u.generic_sbp;

  cvsfs_disconnect (info);
  cvsfs_cache_empty ();
  kfree (info);
}



static int cvsfs_statfs (struct super_block * sb, struct statfs * attr)
{
  attr->f_type = CVSFS_SUPER_MAGIC;
  attr->f_bsize = 1024;
  attr->f_blocks = 0;
  attr->f_namelen = CVSFS_MAXPATHLEN;
  attr->f_files = -1;
  attr->f_bavail = -1;

  return 0;
}


/*
static int cvsfs_readpage (struct file * file, struct page * page)
{
  if (!Page_Uptodate (page))
  {
    memset (kmap (page), 0, PAGE_CACHE_SIZE);
    kunmap (page);
    flush_dcache_page (page);
    SetPageUptodate (page);
  }

  UnlockPage (page);

  return 0;
}



static int cvsfs_writepage (struct page * page)
{
  SetPageDirty (page);
  UnlockPage (page);

  return 0;
}



static int cvsfs_prepare_write (struct file * file, struct page * page,
                                unsigned offset, unsigned to)
{
  void *addr = kmap (page);

  if (!Page_Uptodate (page))
  {
    memset (addr, 0, PAGE_CACHE_SIZE);
    flush_dcache_page (page);
    SetPageUptodate (page);
  }

  SetPageDirty (page);

  return 0;
}



static int cvsfs_commit_write (struct file * file, struct page * page,
                               unsigned offset, unsigned to)
{
  struct inode *inode = page->mapping->host;
  loff_t pos = ((loff_t) page->index << PAGE_CACHE_SHIFT) + to;

  kunmap (page);

  if (pos > inode->i_size)
    inode->i_size = pos;

  return 0;
}
*/
