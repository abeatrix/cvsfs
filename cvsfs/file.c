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
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "proc.h"
#include "util.h"
#include "../include/cvsfs_ioctl.h"

//#define __DEBUG__



/* forward references - file operations */
static ssize_t cvsfs_file_write (struct file *, const char *, size_t, loff_t *);
static int cvsfs_file_open (struct inode *, struct file *);
static int cvsfs_file_release (struct inode *, struct file *);
static int cvsfs_file_ioctl (struct inode *, struct file *, unsigned int, unsigned long);

struct file_operations cvsfs_file_operations = {
						 read:		generic_file_read,
						 write:		cvsfs_file_write,
						 ioctl:		cvsfs_file_ioctl,
						 mmap:		generic_file_mmap,
						 open:		cvsfs_file_open,
						 release:	cvsfs_file_release,
					       };


/* forward references - file operations */
static int cvsfs_file_permission (struct inode *, int);
static int cvsfs_file_setattr (struct dentry *, struct iattr *);
					
struct inode_operations cvsfs_file_inode_operations = {
							permission:	cvsfs_file_permission,
							setattr:	cvsfs_file_setattr,
						      };


/* forward references - file operations */
static int cvsfs_file_readpage (struct file *, struct page *);

struct address_space_operations cvsfs_file_aops = {
						    readpage:	cvsfs_file_readpage,
						  };

					

static int
cvsfs_file_open (struct inode * inode, struct file * file)
{
  struct dentry *dentry = file->f_dentry;
  struct super_block *sb = inode->i_sb;
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) sb->u.generic_sbp;
  char namebuf[CVSFS_MAX_PATH];
  char *version;
  int ret = 0;
  
  file->private_data = inode->i_sb->u.generic_sbp;

  if (cvsfs_get_name (dentry, namebuf, sizeof (namebuf)) < 0)
    return -ENOMEM;

  version = inode->u.generic_ip;
  if ((version != NULL) && (strlen (version) > 0))
  {						/* append version - if any */
    if (sizeof (namebuf) < (strlen (namebuf) + strlen (version) + 3))
      return -ENOMEM;
      
    strcat (namebuf, "@@");
    strcat (namebuf, version);
  }
    
#ifdef __DEBUG__    
  printk (KERN_DEBUG "cvsfs(%d): file_open %s with mode %d and flags %d\n", info->id, namebuf, file->f_mode, file->f_flags);
#endif

  if ((file->f_flags & O_TRUNC) != 0)
  {
#ifdef __DEBUG__    
    printk (KERN_DEBUG "cvsfs(%d): file_open truncate file\n", info->id);
#endif
    ret = cvsfs_truncate_file (info, namebuf);

    dentry->d_time = 0;		/* file attributes dirty - reread them */
  }

#ifdef __DEBUG__    
    printk (KERN_DEBUG "cvsfs(%d): file_open returns %d\n", info->id, ret);
#endif
  
  return ret;
}



static int
cvsfs_file_release (struct inode * inode, struct file * file)
{
#ifdef __DEBUG__    
  struct dentry *dentry = file->f_dentry;
  
  printk (KERN_DEBUG "cvsfs: file_release from mode %d and flags %d for inode %lu, dentry invalid = %lu\n", file->f_mode, file->f_flags, inode->i_ino, dentry->d_time);
#endif

  return 0;
}



static ssize_t
cvsfs_file_write (struct file * file, const char * buffer, size_t count, loff_t * offset)
{
  struct dentry *dentry = file->f_dentry;
  struct inode *inode = dentry->d_inode;
  struct super_block *sb = inode->i_sb;
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) sb->u.generic_sbp;
  char namebuf[CVSFS_MAX_PATH];
  char *version;
  char *block;
  loff_t start;
  size_t size;
  int ret = 0;
  
  if (cvsfs_get_name (dentry, namebuf, sizeof (namebuf)) < 0)
    return -ENOMEM;

  version = inode->u.generic_ip;
  if ((version != NULL) && (strlen (version) > 0))
  {						/* append version - if any */
    if (sizeof (namebuf) < (strlen (namebuf) + strlen (version) + 3))
      return -ENOMEM;
      
    strcat (namebuf, "@@");
    strcat (namebuf, version);
  }

#ifdef __DEBUG__    
  printk (KERN_DEBUG "cvsfs(%d): file_write %s at offset %lli (%d Bytes), inode %lu\n", info->id, namebuf, *offset, count, inode->i_ino);
#endif
  
  block = kmalloc (GFP_KERNEL, 4096);
  if (!block)
    return -ENOMEM;

  start = 0;
  while (start < count)
  {
    size = count - start;
    if (size > 4096)
      size = 4096;   
    
    ret = -ENOMEM;
    if (copy_from_user (block, buffer + start, size) != 0)
      break;
  
    ret = cvsfs_write (info, namebuf, version, *offset, size, block);
    if (ret < 0)
      break;
    
    *offset += size;
    start += size;
  }

  kfree (block);

#ifdef __DEBUG__    
    printk (KERN_DEBUG "cvsfs(%d): file_write completed - exit code %d\n", info->id, ret);
#endif
  
  if (ret < 0)
    return ret;

  dentry->d_time = 0;	// system should evaluate the actual file attributes

  return count;
}



static int
cvsfs_file_ioctl (struct inode * inode, struct file * file,
		  unsigned int cmd, unsigned long arg)
{
  struct dentry *dentry = file->f_dentry;
  struct super_block *sb = inode->i_sb;
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) sb->u.generic_sbp;
  char namebuf[CVSFS_MAX_PATH];
  char fullnamebuf[CVSFS_MAX_PATH];
  int err = 0;
  int size;
  int command;
  char *retval = NULL;
  char *version;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): file_ioctl signature %d, function %d, direction %d, size %d\n",
	  info->id, _IOC_TYPE (cmd), _IOC_NR (cmd), _IOC_DIR (cmd), _IOC_SIZE (cmd));
#endif

  if (_IOC_TYPE (cmd) != CVSFS_IOC_MAGIC)
    return -ENOTTY;

  if (_IOC_DIR (cmd) & _IOC_READ)
    err = ! access_ok (VERIFY_WRITE, (void *) arg, _IOC_SIZE (cmd));
  else
    if (_IOC_DIR (cmd) & _IOC_WRITE)
      err = ! access_ok (VERIFY_READ, (void *) arg, _IOC_SIZE (cmd));
      
  if (err)
    return -EFAULT;

  if (cvsfs_get_name (dentry, namebuf, sizeof (namebuf)) < 0)
    return -ENOMEM;

  strcpy (fullnamebuf, namebuf);

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): file_ioctl request function for %s\n", info->id, fullnamebuf);
#endif

  version = inode->u.generic_ip;
  if ((version != NULL) && (strlen (version) > 0))
  {						/* append version - if any */
    if (sizeof (fullnamebuf) < (strlen (fullnamebuf) + strlen (version) + 3))
      return -ENOMEM;
      
    strcat (fullnamebuf, "@@");
    strcat (fullnamebuf, version);
  }

  command = _IOC_NR (cmd);

  switch (command)
  {
    case CVSFS_RESCAN:		/* re-read the data of this file */
      if ((err = cvsfs_ioctl (info, command, fullnamebuf, &retval)) < 0)
        break;

      dentry->d_time = 0;	/* mark dentry dirty - request update */
      break;

    case CVSFS_GET_VERSION:	/* obtain the revision number */
      {
        limited_string value; // = (limited_string *) arg;

        if (arg == 0)
          return -EFAULT;			/* faulty parameter */

	err = copy_from_user (&value, (limited_string *) arg, sizeof (limited_string));
        if (err < 0)
	  break;
	
#ifdef __DEBUG__
        printk (KERN_DEBUG "cvsfs(%d): file_ioctl - GET_VERSION\n", info->id);
#endif
        if (value.string == NULL)
          return -EFAULT;			/* faulty parameter */

        if ((err = cvsfs_ioctl (info, command, fullnamebuf, &retval)) < 0)
          break;

        size = strlen (retval) + 1;
        if (size > value.maxsize)
	  size = value.maxsize;
      
#ifdef __DEBUG__
        printk (KERN_DEBUG "cvsfs(%d): file_ioctl - version = >%s<\n", info->id, retval);
#endif
      
        err = copy_to_user (value.string, retval, size);

        if (err == 0)
          err = size;	/* return the number of bytes copied to the buffer */
      }
      break;

    case CVSFS_CHECKOUT:	/* checkout a file */
    case CVSFS_CHECKIN:		/* checkin a file */
    case CVSFS_UPDATE:		/* update a file */
      err = cvsfs_ioctl (info, command, fullnamebuf, &retval);
      dentry->d_time = 1;	/* mark dentry dirty - request update */
      break;
      
    case CVSFS_CHECKOUT_VERSION:	/* checkout a file (revision number given) */
    case CVSFS_CHECKIN_VERSION:		/* checkin a file (revision number given) */
      {
	limited_string data;
	
	err = copy_from_user (&data, (limited_string *) arg, sizeof (limited_string));
	if (err < 0)
	  break;
	  
        size = data.maxsize + strlen (namebuf) + 3;
	if (size > CVSFS_MAX_PATH)
	  return -ENOMEM;
	  
	copy_from_user (&fullnamebuf[strlen (namebuf) + 2], data.string, size);
        err = cvsfs_ioctl (info, command, fullnamebuf, &retval);
      }
      dentry->d_time = 0;	/* mark dentry dirty - request update */
      break;
      
    default:
      return -ENOTTY;
  }

  if (retval != NULL)
    kfree (retval);

  return err;
}



static int
cvsfs_file_permission (struct inode * inode, int mask)
{
//  int mode = inode->i_mode;

//  mode >>= 6;

//  if ((mode & 7 & mask) != mask)
//    return -EACCES;

  return 0;
}



static int cvsfs_file_setattr (struct dentry * dentry, struct iattr * attr)
{
  struct inode *inode = dentry->d_inode;
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) inode->i_sb->u.generic_sbp;
  char buf[CVSFS_MAX_PATH];
  char *version;
  int ret = 0;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): file_setattr - entry\n", info->id);
#endif

  if (attr->ia_valid & ATTR_SIZE)
  {
    ret = vmtruncate (inode, attr->ia_size);
    if (ret)
      return ret;
  }

  if (cvsfs_get_name (dentry, buf, sizeof (buf)) < 0)
    return -ENOMEM;
			  
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): file_setattr - file %s to mode 0x%x\n", info->id, buf, attr->ia_mode);
#endif
			    
  version = inode->u.generic_ip;
  if ((version != NULL) && (strlen (version) > 0))
  {                                             /* append version - if any */
    if (sizeof (buf) < (strlen (buf) + strlen (version) + 3))
      return -ENOMEM;
					    
    strcat (buf, "@@");
    strcat (buf, version);
  }

  if (attr->ia_valid & ATTR_MODE)
    ret = cvsfs_change_attr (info, buf, attr->ia_mode);

  dentry->d_time = 0;                   /* mark dentry invalid */
							      
  return ret;
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
  loff_t offset;
  size_t count;
  int result;
  char name[CVSFS_MAX_PATH];

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
  printk (KERN_DEBUG "cvsfs: init_root_dirent\n");
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
