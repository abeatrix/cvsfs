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
  int mode = inode->i_mode;

  mode >>= 6;

  if ((mode & 7 & mask) != mask)
    return -EACCES;

  return 0;
}



static int
cvsfs_file_readpage (struct file * file, struct page * page)
{
  struct dentry *dentry = file->f_dentry;
  char *buffer;
  unsigned long offset;
  unsigned long count;
  int result;

  get_page (page);

  buffer = page_address (page);
  offset = page->index << PAGE_CACHE_SHIFT;
  count = PAGE_SIZE;

  do
  {
    result = cvsfs_read (dentry, offset, count, buffer);

    if (result < 0)
    {
      result = -EIO;

      goto io_error;
    }

    count -= result;
    offset += result;
    buffer += result;
    dentry->d_inode->i_atime = CURRENT_TIME;

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
