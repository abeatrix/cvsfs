/***************************************************************************
                          procfs.c  -  cvsfs procfs interface
                             -------------------
    begin                : Mon Nov 12 2001
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
#include "procfs.h"

#include <linux/kernel.h>		/* printk */

#include <linux/module.h>
// #include <linux/ctype.h>		/* defines isspace */
#include <linux/proc_fs.h>		/* support for /proc file system */
#include "inode.h"
#include "util.h"



struct cvsfs_proc_tree
{
  struct cvsfs_proc_tree *next;
  struct cvsfs_proc_tree *prev;
  struct cvsfs_proc_tree *subdir;
  struct cvsfs_proc_tree *parent;
  struct proc_dir_entry  *proc;
  int			 usage_count;
  char			 name[1];
};



static struct cvsfs_proc_tree *cvsfs_tree;



const char *cvsfs_procfs_root		= "cvsfs";
const char *cvsfs_procfs_status		= "status";
const char *cvsfs_procfs_view		= "view";
const char *cvsfs_procfs_control	= "control";



/* forward references */
static int proc_cvsfs_read_status (char *, char **, off_t, int, int *, void *);
static int proc_cvsfs_read_view (char *, char **, off_t, int, int *, void *);
static int proc_cvsfs_write_view (struct file *, const char *, unsigned long, void *);
static int proc_cvsfs_write_control (struct file *, const char *, unsigned long, void *);



/* forward references of local functions */
static struct cvsfs_proc_tree * find_child (const char *, struct cvsfs_proc_tree *);
static struct cvsfs_proc_tree * find_subdir (const char *, struct cvsfs_proc_tree *);
static struct cvsfs_proc_tree * create_child (const char *, mode_t, struct cvsfs_proc_tree *);
static struct cvsfs_proc_tree * create_subdir (const char *, struct cvsfs_proc_tree *);
static void kill_child_tree (struct cvsfs_proc_tree *);
static void remove_child (const char *, struct cvsfs_proc_tree *);
static void remove_subdir (const char *, struct cvsfs_proc_tree *);



static struct cvsfs_proc_tree * find_child (const char * name, struct cvsfs_proc_tree * parent)
{
  struct cvsfs_proc_tree *loop;

  for (loop = parent->subdir; loop != NULL; loop = loop->next)
    if (strcmp (loop->name, name) == 0)
      return loop;

  return NULL;
}



static struct cvsfs_proc_tree * find_subdir_recursive (char * path, struct cvsfs_proc_tree * parent)
{
  struct cvsfs_proc_tree *loop;
  char *ptr;

  if ((path == NULL) || (*path == '\0'))
    return parent;

  if ((ptr = strchr (path, '/')) != NULL)
  {
    *ptr = '\0';
    ++ptr;
  }

  if (*path != '\0')
  {
    loop = find_child (path, parent);

    if ((loop != NULL) && (ptr != NULL) && (*ptr != '\0'))
      loop = find_subdir_recursive (ptr, loop);
  }
  else
  {
    if ((ptr != NULL) && (*ptr != '\0'))
      loop = find_subdir_recursive (ptr, parent);
    else
      loop = NULL;
  }

  return loop;
}


static struct cvsfs_proc_tree * find_subdir (const char * path, struct cvsfs_proc_tree * base)
{
  char pathelem[CVSFS_MAXPATHLEN];

  strcpy (pathelem, path);

  return find_subdir_recursive (pathelem, base);
}



static struct cvsfs_proc_tree * create_child (const char * name, mode_t mode, struct cvsfs_proc_tree * parent)
{
  struct cvsfs_proc_tree *child;

  if ((parent == NULL) || (name == NULL) || (*name == '\0'))
    return NULL;

  if ((child = find_child (name, parent)) == NULL)
  {
    // child not found - create and insert one
    child = (struct cvsfs_proc_tree *) kmalloc (sizeof (struct cvsfs_proc_tree) +
                                                strlen (name), GFP_KERNEL);
    if (child == NULL)
    {
      printk (KERN_ERR "cvsfs: create_tree_entry - memory squeeze\n");

      return NULL;
    }

    child->proc = create_proc_entry (name, mode, parent->proc);
    if (child->proc == NULL)
    {
      kfree (child);

      printk (KERN_ERR "cvsfs: create_tree_entry - memory squeeze\n");

      return NULL;
    }

    child->proc->owner = THIS_MODULE;

    child->next = parent->subdir;
    child->prev = NULL;
    child->parent = parent;
    child->subdir = NULL;
    child->usage_count = 1;
    strcpy (child->name, name);

    if (child->next != NULL)
      child->next->prev = child;

    parent->subdir = child;
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: create_child - child '%s' created\n", name);
#endif
  }
  else
  {
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: create_child - child '%s' re-used\n", name);
#endif
    ++(child->usage_count);
  }

  return child;
}



static struct cvsfs_proc_tree * create_subdir_recursive (char * path, struct cvsfs_proc_tree * parent)
{
  struct cvsfs_proc_tree *element;
  char *ptr;
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: create_subdir_recursive - path = '%s'\n", path);
#endif
  if (*path == '/')
    return create_subdir_recursive (&(path[1]), parent);

  if ((ptr = strchr (path, '/')) != NULL)
  {
    *ptr = '\0';
    ++ptr;
  }

  element = create_child (path, S_IFDIR | S_IRUGO | S_IXUGO, parent);
  if (element == NULL)
  {
    remove_child (path, parent);
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: create_subdir_recursive - element creation of '%s' failed\n", path);
#endif
    return NULL;
  }
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: create_subdir_recursive - element '%s' created\n", path);
#endif
  if ((ptr != NULL) && (*ptr != '\0'))
    element = create_subdir_recursive (ptr, element);

  return element;
}



static struct cvsfs_proc_tree * create_subdir (const char * path, struct cvsfs_proc_tree * parent)
{
  char pathelem[CVSFS_MAXPATHLEN];

  strcpy (pathelem, path);

  return create_subdir_recursive (pathelem, parent);
}



static void kill_child_tree (struct cvsfs_proc_tree * start)
{
  struct cvsfs_proc_tree *loop;

  if (start == NULL)
    return;

  while ((loop = start->subdir) != NULL)
  {
    kill_child_tree (loop);
    remove_proc_entry (loop->name, start->proc);

    start->subdir = loop->next;
    kfree (loop);
  }
}



static void remove_child (const char * name, struct cvsfs_proc_tree * parent)
{
  struct cvsfs_proc_tree *loop;

  if (parent == NULL)
    return;

  if ((loop = find_child (name, parent)) != NULL)
  {
    if (loop->usage_count > 1)
    {
      --(loop->usage_count);
#ifdef __DEBUG__
      printk (KERN_DEBUG "cvsfs: remove_child - child usage of '%s' decremented\n", name);
#endif
    }
    else
    {
      // cleanup complete subdirectory
      kill_child_tree (loop);

      // remove from chain
      if (loop == parent->subdir)         // first in list ?
        parent->subdir = loop->next;
      else
        loop->prev->next = loop->next;

      // kill procfs entry
      remove_proc_entry (name, parent->proc);

      kfree (loop);
#ifdef __DEBUG__
      printk (KERN_DEBUG "cvsfs: remove_child - child '%s' removed\n", name);
#endif
    }
  }
}



static void remove_subdir_recursive (char * path, struct cvsfs_proc_tree * parent)
{
  struct cvsfs_proc_tree *sub;
  char *ptr;

  if ((parent == NULL) || (path == NULL) || (*path == '\0'))
    return;

  if (*path == '/')
  {
    remove_subdir_recursive (&(path[1]), parent);

    return;
  }

  if ((ptr = strchr (path, '/')) != NULL)
  {
    *ptr = '\0';
    ++ptr;
  }

  if ((sub = find_child (path, parent)) != NULL)
  {
    if ((ptr != NULL) && (*ptr != '\0'))
      remove_subdir_recursive (ptr, sub);

    remove_child (path, parent);
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: remove_subdir_recursive - element '%s' removed\n", path);
#endif
  }
#ifdef __DEBUG__
  else
    printk (KERN_DEBUG "cvsfs: remove_subdir_recursive - element '%s' not found\n", path);
#endif
}



static void remove_subdir (const char * path, struct cvsfs_proc_tree * base)
{
  char pathelem[CVSFS_MAXPATHLEN];

  strcpy (pathelem, path);

  remove_subdir_recursive (pathelem, base);
}



int cvsfs_procfs_init ()
{
  struct proc_dir_entry *cvsfs_root;

  cvsfs_tree = NULL;
  cvsfs_root = proc_mkdir (cvsfs_procfs_root, NULL);
  if (cvsfs_root != NULL)
  {
    cvsfs_tree = (struct cvsfs_proc_tree *) kmalloc (sizeof (struct cvsfs_proc_tree) +
                                                     strlen (cvsfs_procfs_root), GFP_KERNEL);
    if (cvsfs_tree != NULL)
    {
      cvsfs_tree->next = NULL;
      cvsfs_tree->prev = NULL;
      cvsfs_tree->subdir = NULL;
      cvsfs_tree->parent = NULL;
      cvsfs_tree->proc = cvsfs_root;
      cvsfs_tree->usage_count = 1;
      strcpy (cvsfs_tree->name, cvsfs_procfs_root);

      return 0;
    }
    else
      kfree (cvsfs_root);
  }

  printk (KERN_ERR "cvsfs: procfs_init - failed\n");

  return -1;
}



void cvsfs_procfs_cleanup ()
{
  if (cvsfs_tree != NULL)
  {
    kill_child_tree (cvsfs_tree);
    remove_proc_entry (cvsfs_procfs_root, 0);
    kfree (cvsfs_tree);
    cvsfs_tree = NULL;
  }
}



int cvsfs_procfs_user_init (const char * root, void * sb)
{
  struct cvsfs_proc_tree *base;

  if (cvsfs_tree == NULL)
    return -1;

  base = create_subdir (root, cvsfs_tree);
  if (base != NULL)
  {
    struct cvsfs_proc_tree *element;

    if ((element = create_child (cvsfs_procfs_status,
                                 S_IFREG | S_IRUGO, base)) != 0)
    {
      element->proc->data = sb;
      element->proc->read_proc = proc_cvsfs_read_status;
      if ((element = create_child (cvsfs_procfs_view,
                                   S_IFREG | S_IWUGO | S_IRUGO, base)) != 0)
      {
        element->proc->data = sb;
        element->proc->read_proc = proc_cvsfs_read_view;
        element->proc->write_proc = proc_cvsfs_write_view;

        if ((element = create_child (cvsfs_procfs_control,
                                     S_IFREG | S_IWUGO, base)) != 0)
        {
          element->proc->data = sb;
          element->proc->write_proc = proc_cvsfs_write_control;

          return 0;
        }
	remove_child (cvsfs_procfs_view, base);
      }
      remove_child (cvsfs_procfs_status, base);
    }
  }

  remove_subdir (root, cvsfs_tree);

  printk (KERN_ERR "cvsfs: procfs_user_init - failed\n");

  return -1;
}



void cvsfs_procfs_user_cleanup (const char * root)
{
  struct cvsfs_proc_tree *base;

  if (cvsfs_tree)
  {
    base = find_subdir (root, cvsfs_tree);
    if (base != NULL)
    {
      remove_child (cvsfs_procfs_status, base);
      remove_child (cvsfs_procfs_view, base);
      remove_child (cvsfs_procfs_control, base);
    }

    remove_subdir (root, cvsfs_tree);
  }
}



static int proc_cvsfs_read_status (char *buffer, char **start, off_t offset, int size, int *eof, void *data)
{
  int len;
  struct super_block *sb;
  struct cvsfs_sb_info *info;

  if (data == NULL)
    return 0;

  sb = (struct super_block *) data;
  info = (struct cvsfs_sb_info *) sb->u.generic_sbp;

  len = sprintf (buffer, "cvsfs running\n");
  len += sprintf (buffer + len, "  Server ....... %s\n", info->mnt.server);
  len += sprintf (buffer + len, "  CVS root ..... %s\n", info->mnt.root);
  len += sprintf (buffer + len, "  Project ...... %s\n", info->mnt.project);
  len += sprintf (buffer + len, "  Mountpoint ... %s\n", info->mnt.mountpoint);
  len += sprintf (buffer + len, "  Fileattribs .. %04o\n", (info->mnt.file_mode & 0xfff));
  len += sprintf (buffer + len, "  Dir attribs .. %04o\n", (info->mnt.dir_mode & 0xfff));

  return len;
}



static int proc_cvsfs_read_view (char *buffer, char **start, off_t offset, int size, int *eof, void *data)
{
  int len;

  if (data == NULL)
    return 0;

  len = sprintf (buffer, "* /CHECKEDOUT\n");
  len += sprintf (buffer + len, "* /MAIN/HEAD\n");

  return len;
}



void cvsfs_reset_viewrule (struct super_block * sb)
{
  printk (KERN_DEBUG "cvsfs: cvsfs_reset_viewrule'\n");
  printk (KERN_DEBUG "cvsfs: cvsfs_reset_viewrule - not implemented'\n");
}



int cvsfs_append_viewrule (struct super_block * sb, char *filemask, char *rule)
{
  printk (KERN_DEBUG "cvsfs: cvsfs_append_viewrule - filemask: '%s', rule: '%s'\n", filemask, rule);
  printk (KERN_DEBUG "cvsfs: cvsfs_append_viewrule - not implemented\n");

  return 0;
}



static int proc_cvsfs_write_view (struct file *file, const char *buffer, unsigned long size, void *data)
{
  struct super_block *sb;
  char *line;
  char *filemask;
  char *rule;
  char *ptr;
  int len;

  if (data == NULL)
    return -EINVAL;

  sb = (struct super_block *) data;

  if (file->private_data == NULL)
    cvsfs_reset_viewrule (sb);

  ++(file->private_data);

  for (ptr = (char *) buffer, len = 0; (*ptr != '\n') && (*ptr != '\0') && (len <= size); ++ptr, ++len);

  if (len == 0)
    return 1;

  ++len;

  line = (char *) kmalloc (len, GFP_KERNEL);
  if (line == NULL)
  {
    printk (KERN_ERR "cvsfs: procfs_cvsfs_write_view - memory squeeze\n");

    return -EINVAL;
  }

  strncpy (line, buffer, len);
  line[len - 1] = '\0';

  filemask = cvsfs_skip_whitespace (line);

  cvsfs_rtrim (filemask);

  if ((ptr = strpbrk (filemask, " \t")) != NULL)
  {
    *ptr = '\0';
    ++ptr;
    
    rule = cvsfs_skip_whitespace (ptr);
    
    if (cvsfs_append_viewrule (sb, filemask, rule) != 0)
      len = -EINVAL;
  }
  else
  {
    printk (KERN_ERR "cvsfs: procfs_cvsfs_write_view - line %ld: view spec missing for filemask '%s'\n", (unsigned long) file->private_data, filemask);

    len = -EINVAL;
  }

  kfree (line);

  return len;
}



int cvsfs_control_command (struct super_block * sb, char *command, char *parameter)
{
  if (parameter != NULL)
    printk (KERN_DEBUG "cvsfs: cvsfs_control_command - command: '%s', parameter: '%s'\n", command, parameter);
  else
    printk (KERN_DEBUG "cvsfs: cvsfs_control_command - command: '%s', no parameter\n", command);

  printk (KERN_DEBUG "cvsfs: cvsfs_control_command - not implemented\n");

  return 0;
}



static int proc_cvsfs_write_control (struct file *file, const char *buffer, unsigned long size, void *data)
{
  struct super_block *sb;
  char *line;
  char *command;
  char *parameter;
  char *ptr;
  int len;

  if (data == NULL)
    return -EINVAL;

  sb = (struct super_block *) data;

//  if (file->private_data == NULL)
//    cvsfs_reset_viewrule (sb);

  ++(file->private_data);

  for (ptr = (char *) buffer, len = 0; (*ptr != '\n') && (*ptr != '\0') && (len <= size); ++ptr, ++len);

  if (len == 0)
    return 1;

  ++len;

  line = (char *) kmalloc (len, GFP_KERNEL);
  if (line == NULL)
  {
    printk (KERN_ERR "cvsfs: procfs_cvsfs_write_view - memory squeeze\n");

    return -EINVAL;
  }

  strncpy (line, buffer, len);
  line[len - 1] = '\0';

  command = cvsfs_skip_whitespace (line);

  cvsfs_rtrim (command);

  if ((ptr = strpbrk (command, " \t")) != NULL)
  {
    *ptr = '\0';
    ++ptr;
    
    parameter = cvsfs_skip_whitespace (ptr);
  }
  else
    parameter = NULL;

  if (cvsfs_control_command (sb, command, parameter) != 0)
    len = -EINVAL;

  kfree (line);

  return len;
}

