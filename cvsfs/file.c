/***************************************************************************
                          file.c  -  description
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

#include "cvsfs_config.h"
#include "file.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/pagemap.h>

#include "proc.h"
#include "util.h"



/* forward references - file operations */
static int cvsfs_file_open (struct inode *, struct file *);
static int cvsfs_file_release (struct inode *, struct file *);
// static int cvsfs_file_ioctl (struct inode *, struct file *, unsigned int, unsigned long);

struct file_operations cvsfs_file_operations = {
						 read:		generic_file_read,
//						 ioctl:		cvsfs_file_ioctl,
						 mmap:		generic_file_mmap,
						 open:		cvsfs_file_open,
						 release:	cvsfs_file_release,
					       };


/* forward references - file operations */
static int cvsfs_file_permission (struct inode *, int);
					
struct inode_operations cvsfs_file_inode_operations = {
							permission:	cvsfs_file_permission,
						      };


/* forward references - file operations */
static int cvsfs_file_readpage (struct file *, struct page *);

struct address_space_operations cvsfs_file_aops = {
						    readpage:	cvsfs_file_readpage,
						  };

					

static int
cvsfs_file_open (struct inode * inode, struct file * file)
{
  return 0;
}



static int
cvsfs_file_release (struct inode * inode, struct file * file)
{
  return 0;
}



//static int
//cvsfs_file_ioctl (struct inode * inode, struct file * file, unsigned int, unsigned long)
//{
//  return 0;
//}



static int
cvsfs_file_permission (struct inode * inode, int mask)
{
//  int mode = inode->i_mode;

//  mode >>= 6;

//  if ((mode & 7 & mask) != mask)
//    return -EACCES;

  return 0;
}



static int
cvsfs_file_readpage (struct file * file, struct page * page)
{
  struct dentry *dentry = file->f_dentry;
  struct inode *inode = dentry->d_inode;
  struct super_block *sb = inode->i_sb;
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) sb->u.generic_sbp;
  char *buffer;
  char *version;
  unsigned long offset;
  unsigned long count;
  int result;
  char name[512];

  if (cvsfs_get_name (dentry, name, sizeof (name)) != 0)
    return -EIO;
  version = inode->u.generic_ip;

  get_page (page);

  buffer = page_address (page);
  offset = page->index << PAGE_CACHE_SHIFT;
  count = PAGE_SIZE;

  do
  {
    result = cvsfs_read (info, name, version, offset, count, buffer);

    if (result < 0)
    {
      result = -EIO;

      goto io_error;
    }

    count -= result;
    offset += result;
    buffer += result;
    inode->i_atime = CURRENT_TIME;

    if (!result)
      break;
  } while (count);

  memset (buffer, 0, count);
  flush_dcache_page (page);
  SetPageUptodate (page);

  result = 0;

io_error:

  UnlockPage (page);
  put_page (page);

  return result;
}



/* init the root directory entry for this file system */
void
cvsfs_init_root_dirent (struct cvsfs_sb_info * server, struct cvsfs_fattr * fattr)
{
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: cvsfs_init_root_dirent\n");
#endif
  memset (fattr, 0, sizeof (struct cvsfs_fattr));
    
  fattr->f_nlink = 1;
  fattr->f_uid = server->mount.uid;
  fattr->f_gid = server->mount.gid;
  fattr->f_blksize = 512;
  fattr->f_ino = 2;
  fattr->f_mtime = CURRENT_TIME;
  fattr->f_mode = server->mount.dir_mode;
  fattr->f_size = 512;
  fattr->f_blocks = 0;
}
