/***************************************************************************
                          dir.c  -  description
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
#include "dir.h"

#include <linux/ctype.h>

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/smp_lock.h>
#include <linux/slab.h>
//#include <linux/dcache.h>

#include "inode.h"
#include "file.h"
#include "proc.h"
#include "util.h"

//#define __DEBUG__


/* forward references - directory operations */
static int cvsfs_readdir (struct file *, void *, filldir_t);
static int cvsfs_dir_open (struct inode *, struct file *);

struct file_operations cvsfs_dir_operations = {
						read:		generic_read_dir,
						readdir:	cvsfs_readdir,
						open:		cvsfs_dir_open,
					      };

					      					
/* forward references - directory inode operations */
static int cvsfs_create (struct inode *, struct dentry *, int);
static struct dentry * cvsfs_lookup (struct inode *, struct dentry *);
static int cvsfs_unlink (struct inode *, struct dentry *);
static int cvsfs_mkdir (struct inode *, struct dentry *, int);
static int cvsfs_rmdir (struct inode *, struct dentry *);
//static int cvsfs_dir_permission (struct inode *, int);

struct inode_operations cvsfs_dir_inode_operations = {
						       create:		cvsfs_create,
						       lookup:		cvsfs_lookup,
						       unlink:		cvsfs_unlink,
						       mkdir:		cvsfs_mkdir,
						       rmdir:		cvsfs_rmdir,
//						       permission:	cvsfs_dir_permission,
						     };


/* forward references - directory inode operations */
static int cvsfs_lookup_validate (struct dentry *, int);
static int cvsfs_hash_dentry (struct dentry *, struct qstr *);
static int cvsfs_compare_dentry (struct dentry *, struct qstr *, struct qstr *);
static int cvsfs_delete_dentry (struct dentry *);

static struct dentry_operations cvsfs_dentry_operations = {
							    d_revalidate:	cvsfs_lookup_validate,
							    d_hash:		cvsfs_hash_dentry,
							    d_compare:		cvsfs_compare_dentry,
							    d_delete:		cvsfs_delete_dentry,
							  };
							



/* returns a complete directory contents */						
static int
cvsfs_readdir (struct file * f, void * dirent, filldir_t filldir)
{
  struct dentry *dentry = f->f_dentry;
  struct inode *inode = dentry->d_inode;
  struct super_block *sb = inode->i_sb;
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) sb->u.generic_sbp;
  struct qstr qname;
  unsigned long ino;
  char buf[CVSFS_MAX_PATH];
  char * name;

  if (cvsfs_get_name (dentry, buf, sizeof (buf)) < 0)
    return -EIO;

  switch ((unsigned int) f->f_pos)
  {
    case 0:
      if (filldir (dirent, ".", 1, 0, inode->i_ino, DT_DIR) < 0)
        return -EIO;
#ifdef __DEBUG__
      printk (KERN_DEBUG "cvsfs(%d): readdir - file %s, parent inode %lu\n", info->id, ".", inode->i_ino);
#endif
      f->f_pos = 1;
      break;

    case 1:
      if (filldir (dirent, "..", 2, 1, dentry->d_parent->d_inode->i_ino, DT_DIR) < 0)
        return -EIO;
#ifdef __DEBUG__
      printk (KERN_DEBUG "cvsfs(%d): readdir - file %s, parent inode %lu\n", info->id, "..", inode->i_ino);
#endif
      f->f_pos = 2;
      break;

    default:
      /* ask cvsfs daemon to get the directory contents */
      name = cvsfs_get_file (info, buf, f->f_pos);
      if (!name)
        return -ENOENT;	/* there are no files in this directory */

      qname.name = name;
      qname.len = strlen (name);

      ino = find_inode_number (dentry, &qname);
      if (!ino)
        ino = iunique (dentry->d_sb, 2);

      if (filldir (dirent, qname.name, qname.len, f->f_pos, ino, DT_UNKNOWN) >= 0)
        ++(f->f_pos);
#ifdef __DEBUG__
      printk (KERN_DEBUG "cvsfs(%d): readdir - file %s, parent inode %lu\n", info->id, name, inode->i_ino);
#endif
      kfree (name);
  }

  return 0;
}



static int
cvsfs_dir_open (struct inode * inode, struct file * file)
{
  return 0;
}



/* create a file */
static int cvsfs_create (struct inode * dir, struct dentry * dentry, int mode)
{
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) dir->i_sb->u.generic_sbp;
  struct cvsfs_fattr fattr;
  struct inode       *inode;
  char buf[CVSFS_MAX_PATH];
  int res;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): create - mode %d\n", info->id, mode);
#endif
  if (cvsfs_get_name (dentry, buf, sizeof (buf)) < 0)
    return -ENOMEM;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): create - name %s\n", info->id, buf);
#endif
  if ((res = cvsfs_create_file (info, buf, mode, &fattr)) < 0)
    return res;

  inode = cvsfs_iget (dir->i_sb, &fattr);
  kfree (fattr.f_version);
    
  if (!inode)
    return -EACCES;
    
  dentry->d_op = &cvsfs_dentry_operations;
  dir->i_nlink++;
  inode->i_nlink = 2;  

  if (dir->i_mode & S_ISGID)
    inode->i_mode |= S_ISGID;

  d_instantiate (dentry, inode);

  return 0;
  return -EACCES;
}



/* delete a file */
static int cvsfs_unlink (struct inode * dir, struct dentry * dentry)
{
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) dir->i_sb->u.generic_sbp;
  struct inode       *inode;
  char buf[CVSFS_MAX_PATH];
  int retval;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): rmdir\n", info->id);
#endif
  if (cvsfs_get_name (dentry, buf, sizeof (buf)) < 0)
    return -EIO;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): rmdir - file %s\n", info->id, buf);
#endif
  retval = cvsfs_remove_file (info, buf);
  if (retval < 0)
    return retval;

  inode = dentry->d_inode;
  inode->i_nlink = 0;
  inode->i_ctime = dir->i_ctime = dir->i_mtime = CURRENT_TIME;
//  dir->i_nlink--;

  d_delete (dentry);
  
  return 0;
}



/* called from kernel when searching for a file (name is in dentry) */
static struct dentry *
cvsfs_lookup (struct inode * dir, struct dentry * dentry)
{
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) dir->i_sb->u.generic_sbp;
  struct cvsfs_fattr fattr;
  struct inode       *inode;
  char buf[CVSFS_MAX_PATH];

  if (cvsfs_get_name (dentry, buf, sizeof (buf)) < 0)
    return ERR_PTR (-ENOENT);

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): lookup - search for name %s\n", info->id, buf);
#endif
  /* do we have the file ? */
  if (cvsfs_get_attr (info, buf, &fattr) >= 0)
  {
    fattr.f_ino = iunique (dentry->d_sb, 2);
    inode = cvsfs_iget (dir->i_sb, &fattr);
    kfree (fattr.f_version);

#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs(%d): lookup - inode %lu\n", info->id, fattr.f_ino);
#endif
    if (!inode)
      return ERR_PTR (-EACCES);
    
    dentry->d_op = &cvsfs_dentry_operations;
    dentry->d_time = 0;

    d_add (dentry, inode);
  }
  
  return NULL;
}



/* create a new directory */
static int cvsfs_mkdir (struct inode * dir, struct dentry * dentry, int mode)
{
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) dir->i_sb->u.generic_sbp;
  struct cvsfs_fattr fattr;
  struct inode       *inode;
  char buf[CVSFS_MAX_PATH];
  int res;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): mkdir - mode %d\n", info->id, mode);
#endif
  if (cvsfs_get_name (dentry, buf, sizeof (buf)) < 0)
    return -EIO;

  if ((res = cvsfs_create_dir (info, buf, mode, &fattr)) < 0)
    return res;

  inode = cvsfs_iget (dir->i_sb, &fattr);
  kfree (fattr.f_version);
    
  if (!inode)
    return -EACCES;
    
  dentry->d_op = &cvsfs_dentry_operations;
  dentry->d_time = 0;			/* mark dentry values valid */
  dir->i_nlink++;
  inode->i_nlink = 2;  

  if (dir->i_mode & S_ISGID)
    inode->i_mode |= S_ISGID;

  d_instantiate (dentry, inode);

  return 0;
}



/* remove a directory */
static int cvsfs_rmdir (struct inode * dir, struct dentry * dentry)
{
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) dir->i_sb->u.generic_sbp;
  struct inode       *inode;
  char buf[CVSFS_MAX_PATH];
  int retval;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): rmdir\n", info->id);
#endif
  if (cvsfs_get_name (dentry, buf, sizeof (buf)) < 0)
    return -EIO;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): rmdir - file %s\n", info->id, buf);
#endif
  retval = cvsfs_remove_dir (info, buf);
  if (retval < 0)
    return retval;

  inode = dentry->d_inode;
  inode->i_nlink = 0;
  inode->i_ctime = dir->i_ctime = dir->i_mtime = CURRENT_TIME;
  dir->i_nlink--;

//  d_delete (dentry);
  
  return 0;
}



/* check permission */
//static int cvsfs_dir_permission (struct inode * dir, int mode)
//{
//  printk (KERN_DEBUG "cvsfs: permission - mask %d, parent inode %lu\n", mode, dir->i_ino);

//  return 0;
//}



/* check whether the entry is valid anymore */
static int
cvsfs_lookup_validate (struct dentry * dentry, int flags)
{
#ifdef __DEBUG__
  char buf[128];
#endif
  struct inode *inode = dentry->d_inode;
  int valid = 1;

#ifdef __DEBUG__
  cvsfs_get_name (dentry, buf, sizeof (buf));
  printk (KERN_DEBUG "cvsfs: lookup_validate - file %s\n", buf);
#endif

  if (inode)
  {
    lock_kernel ();

    if (dentry->d_time != 0)	/* dentry values valid ? */
      valid = 0;

    if (is_bad_inode (inode))
      valid = 0;

    unlock_kernel ();
  }

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: lookup_validate - valid = %d\n", valid);
#endif

  return valid;
}



/* calculate a unique hash for the file name given */
static int
cvsfs_hash_dentry (struct dentry * qentry, struct qstr * str)
{
#ifdef __DEBUG__
  char buf[128];
#endif
  unsigned long hash;
  int i;

#ifdef __DEBUG__
  cvsfs_get_name (qentry, buf, sizeof (buf));
  printk (KERN_DEBUG "cvsfs: hash_dentry - parent %s, ", buf);
  memcpy (buf, str->name, str->len);
  buf[str->len] = '\0';
  printk (KERN_DEBUG "%s\n", buf);
#endif

  hash = init_name_hash ();

  for (i = 0; i < str->len; ++i)
    hash = partial_name_hash (tolower (str->name[i]), hash);

  str->hash = end_name_hash (hash);

  return 0;
}



/* compare the two qstrs */
static int
cvsfs_compare_dentry (struct dentry * dentry, struct qstr * a, struct qstr * b)
{
#ifdef __DEBUG__
  char buf[128];
#endif
  int i;

#ifdef __DEBUG__
  cvsfs_get_name (dentry, buf, sizeof (buf));
  printk (KERN_DEBUG "cvsfs: compare_dentry - parent %s,", buf);
  memcpy (buf, a->name, a->len);
  buf[a->len] = '\0';
  printk (KERN_DEBUG "name 1: %s, ", buf);
  memcpy (buf, b->name, b->len);
  buf[b->len] = '\0';
  printk (KERN_DEBUG "name 2: %s\n", buf);
#endif

  if (a->len != b->len)
    return 1;

  for (i = 0; i < a->len; ++i)
    if (tolower (a->name[i]) != tolower (b->name[i]))
      return 1;

  return 0;
}



/* dentry will be freed - chance to free d_fsdata element if used */
static int
cvsfs_delete_dentry (struct dentry * dentry)
{
  if (dentry->d_inode)
    if (is_bad_inode (dentry->d_inode))
      return 1;

  return 0;
}
