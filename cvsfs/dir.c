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
#include <linux/smp_lock.h>
#include <linux/slab.h>

//#include <asm/uaccess.h>
//#include <asm/system.h>

#include "inode.h"
#include "cache.h"
#include "proc.h"



/* forward references - directory operations */
static int cvsfs_readdir (struct file *, void *, filldir_t);
static int cvsfs_dir_open (struct inode *, struct file *);

struct file_operations cvsfs_dir_operations = {
						read:		generic_read_dir,
						readdir:	cvsfs_readdir,
						open:		cvsfs_dir_open,
					      };

					      					
/* forward references - directory inode operations */
static struct dentry * cvsfs_lookup (struct inode *, struct dentry *);

struct inode_operations cvsfs_dir_inode_operations = {
						       lookup:	cvsfs_lookup,
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
							

						
static int cvsfs_readdir (struct file * f, void * dirent, filldir_t filldir)
{
  struct dentry *dentry = f->f_dentry;
  struct inode *inode = dentry->d_inode;
  struct super_block *sb = inode->i_sb;
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) sb->u.generic_sbp;
  char buf[CVSFS_MAXPATHLEN];
  int result;
  int pos;

  struct cvsfs_directory *dir;
  struct cvsfs_dirlist_node *file;

  if (cvsfs_get_name (dentry, buf) < 0)
    return -1;

  result = 0;

  switch ((unsigned int) f->f_pos)
  {
    case 0:
      if (filldir (dirent, ".", 1, 0, inode->i_ino, DT_DIR) < 0)
        return result;

      f->f_pos = 1;

    case 1:
      if (filldir (dirent, "..", 2, 1, dentry->d_parent->d_inode->i_ino, DT_DIR) < 0)
        return result;

      f->f_pos = 2;

    default:
      cvsfs_lock (info);
      dir = cvsfs_cache_get (info, buf);
      cvsfs_unlock (info);

      if (!dir)
        return -1;

      pos = 2;
      for (file = dir->head; file != NULL; file = file->next)
      {
        if (pos == f->f_pos)
        {
          struct qstr qname;
          unsigned long ino;

          qname.name = file->entry.name;
          qname.len = strlen (qname.name);

          ino = find_inode_number (dentry, &qname);

          if (!ino)
            ino = iunique (dentry->d_sb, 2);

          if (filldir (dirent, qname.name, qname.len, f->f_pos, ino, DT_UNKNOWN) >= 0)
            ++(f->f_pos);
        }

        ++pos;
      }
  }

  return result;
}



static int cvsfs_dir_open (struct inode * inode, struct file * file)
{
  return 0;
}



static struct dentry * cvsfs_lookup (struct inode * dir, struct dentry * dentry)
{
  struct cvsfs_fattr fattr;
  struct inode       *inode;

  if (cvsfs_get_attr (dentry, &fattr, (struct cvsfs_sb_info *) dir->i_sb->u.generic_sbp) < 0)
  {
    return ERR_PTR (-ENOENT);
  }

  fattr.f_ino = iunique (dentry->d_sb, 2);
  inode = cvsfs_iget (dir->i_sb, &fattr);

  if (inode)
  {
    dentry->d_op = &cvsfs_dentry_operations;

    d_add (dentry, inode);
  }

  kfree (fattr.f_info.version);

  return NULL;
}



static int cvsfs_lookup_validate (struct dentry * dentry, int flags)
{
  struct inode *inode = dentry->d_inode;
  int valid = 1;

  if (inode)
  {
    lock_kernel ();

    if (is_bad_inode (inode))
      valid = 0;

    unlock_kernel ();
  }

  return valid;
}



static int cvsfs_hash_dentry (struct dentry * qentry, struct qstr * str)
{
  unsigned long hash;
  int i;

  hash = init_name_hash ();

  for (i = 0; i < str->len; ++i)
    hash = partial_name_hash (tolower (str->name[i]), hash);

  str->hash = end_name_hash (hash);

  return 0;
}



static int cvsfs_compare_dentry (struct dentry * dentry, struct qstr * a, struct qstr * b)
{
  int i;

  if (a->len != b->len)
    return 1;

  for (i = 0; i < a->len; ++i)
    if (tolower (a->name[i]) != tolower (b->name[i]))
      return 1;

  return 0;
}



static int cvsfs_delete_dentry (struct dentry * dentry)
{
  if (dentry->d_inode)
    if (is_bad_inode (dentry->d_inode))
      return 1;

  return 0;
}
