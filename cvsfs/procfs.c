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
#include "superblock.h"
#include "util.h"
#include "proc.h"



//#define __DEBUG__

#define MAX_MOUNTS	32000



const char *cvsfs_procfs_root		= "cvsfs";
const char *cvsfs_procfs_mounts		= "mounts";
const char *cvsfs_procfs_status		= "status";
const char *cvsfs_procfs_view		= "view";
const char *cvsfs_procfs_control	= "control";

static struct proc_dir_entry *cvsfs_root = NULL;



/* forward references */
static int proc_cvsfs_read_mounts (char *, char **, off_t, int, int *, void *);
static int proc_cvsfs_read_status (char *, char **, off_t, int, int *, void *);
static int proc_cvsfs_read_view (char *, char **, off_t, int, int *, void *);
static int proc_cvsfs_write_view (struct file *, const char *, unsigned long, void *);
static int proc_cvsfs_write_control (struct file *, const char *, unsigned long, void *);



/* called when cvsfs module is loaded - initialize procfs entries */
int
cvsfs_procfs_init ()
{
  cvsfs_root = proc_mkdir (cvsfs_procfs_root, NULL);
  if (cvsfs_root != NULL)
  {
    struct proc_dir_entry *entry;
    
    cvsfs_root->owner = THIS_MODULE;
    
    entry = create_proc_entry (cvsfs_procfs_mounts, S_IFREG | S_IRUGO, cvsfs_root);
    if (entry != NULL)
    {
      entry->read_proc = proc_cvsfs_read_mounts;
      entry->owner = THIS_MODULE;

      return 0;
    }
    else
      remove_proc_entry (cvsfs_procfs_root, NULL);
  }

  printk (KERN_ERR "cvsfs: procfs_init - failed\n");

  return -1;
}



/* called when cvsfs module is to be unloaded - cleanup procfs */
void
cvsfs_procfs_cleanup ()
{
  if (cvsfs_root != NULL)
  {
    remove_proc_entry (cvsfs_procfs_mounts, cvsfs_root);
    remove_proc_entry (cvsfs_procfs_root, NULL);
  }
}



/* call when a mount is issued - create mount specific procfs entries */
int
cvsfs_procfs_user_init (struct cvsfs_sb_info * sb)
{
  struct proc_dir_entry *base;

  if (cvsfs_root == NULL)
    return -1;

  sb->procfs.data = NULL;

  if (sb->id == 0)
  {
#ifdef __DEBUG__
    printk (KERN_ERR "cvsfs: procfs_user_init - no valid id assigned\n");
#endif

    return -1;		/* no more ids available - exit */
  }

  base = proc_mkdir (sb->idstring, cvsfs_root);
  sb->procfs.data = base;
  if (base != NULL)
  {
    struct proc_dir_entry *element;

    base->owner = THIS_MODULE;
    base->data = sb;
    
    if ((element = create_proc_entry (cvsfs_procfs_status,
                                      S_IFREG | S_IRUGO, base)) != NULL)
    {
      element->owner = THIS_MODULE;
      element->data = sb;
      element->uid = sb->mount.uid;
      element->gid = sb->mount.gid;
      element->read_proc = proc_cvsfs_read_status;
      
      if ((element = create_proc_entry (cvsfs_procfs_view,
                                        S_IFREG | S_IWUSR | S_IRUGO, base)) != NULL)
      {
        element->owner = THIS_MODULE;
        element->data = sb;
        element->uid = sb->mount.uid;
        element->gid = sb->mount.gid;
        element->read_proc = proc_cvsfs_read_view;
        element->write_proc = proc_cvsfs_write_view;

        if ((element = create_proc_entry (cvsfs_procfs_control,
                                          S_IFREG | S_IWUSR, base)) != NULL)
        {
          element->owner = THIS_MODULE;
          element->data = sb;
          element->uid = sb->mount.uid;
          element->gid = sb->mount.gid;
          element->write_proc = proc_cvsfs_write_control;

#ifdef __DEBUG__
          printk (KERN_ERR "cvsfs: procfs_user_init - ID %d successful\n", sb->id);
#endif
	  
	  return 0;		/* successful exit */
        }
	remove_proc_entry (cvsfs_procfs_view, base);
      }
      remove_proc_entry (cvsfs_procfs_status, base);
    }
  }

  remove_proc_entry (sb->idstring, cvsfs_root);

  printk (KERN_ERR "cvsfs: procfs_user_init - failed\n");

  return -1;
}



/* called in case of umount - remove mount specific procfs entries */
void
cvsfs_procfs_user_cleanup (struct cvsfs_sb_info * sb)
{
  if ((cvsfs_root != NULL) && (sb->procfs.data != NULL))
  {
    struct proc_dir_entry *base;
    
    base = sb->procfs.data;
    
    remove_proc_entry (cvsfs_procfs_status, base);
    remove_proc_entry (cvsfs_procfs_view, base);
    remove_proc_entry (cvsfs_procfs_control, base);
    remove_proc_entry (sb->idstring, cvsfs_root);
  }
}



static int
proc_cvsfs_read_mounts (char *buffer, char **start,
                        off_t offset, int size, int *eof, void *data)
{
  int len;
  int pos;
  struct cvsfs_sb_info *scan;

  *eof = 0;

  pos = 0;
  len = 0;
  scan = cvsfs_read_lock_superblock_list ();
  /* scan the superblock list for a free id - they were assigned sequentially */
  while (scan != NULL)
  {
    int itemsize = strlen (scan->idstring) + strlen (scan->mount.mountpoint) + 2;
    
    if (pos >= offset)
    {
      if ((pos + itemsize + 1) >= size)
        break;
	
      len += sprintf (buffer, "%s %s\n", scan->idstring, scan->mount.mountpoint);
    }
    
    pos += size;
    scan = scan->next;
  }

  if (scan == NULL)
    *eof = 1;

  cvsfs_read_unlock_superblock_list ();

  return len;
}



static int
proc_cvsfs_read_status (char *buffer, char **start,
                        off_t offset, int size, int *eof, void *data)
{
  int len;
  struct cvsfs_sb_info *info;

  if (data == NULL)
    return 0;

  info = (struct cvsfs_sb_info *) data;

  len = sprintf (buffer, "Server ................ %s\n", info->connection.server);
  len += sprintf (buffer + len, "CVS root .............. %s\n", info->connection.root);
  len += sprintf (buffer + len, "Project ............... %s\n", info->connection.project);
  len += sprintf (buffer + len, "Mountpoint ............ %s\n", info->mount.mountpoint);
  len += sprintf (buffer + len, "Fileattribs ........... %04o\n", (info->mount.file_mode & 0xfff));
  len += sprintf (buffer + len, "Dir attribs ........... %04o\n", (info->mount.dir_mode & 0xfff));
  if (info->id >= 1)
  {
#ifdef CONFIG_DEVFS_FS
    len += sprintf (buffer + len, "Device ................ /dev/cvsfs/%i\n", info->id);
#else
    len += sprintf (buffer + len, "Device (major/minor id) %i/%i\n", info->device.major, info->id);
#endif
  }
  else		/* should never go there - for sanity only */
    len += sprintf (buffer + len, "No Device (out of numbers)\n");
  len += sprintf (buffer + len, "Daemon attached ....... %s\n", info->device.in_use == 0 ? "no" : "yes");

  *eof = 1;

  return len;
}



static int
proc_cvsfs_read_view (char *buffer, char **start,
                      off_t offset, int size, int *eof, void *data)
{
  int len;
  char *view;
  struct cvsfs_sb_info *info;

  if (data == NULL)
    return 0;

  info = (struct cvsfs_sb_info *) data;

  len = cvsfs_get_view (info, &view);
  if ((len <= 0) || (offset >= len))
  {
    *eof = 1;
    len = 0;
  }
  else
  {
    int chunk;

    if ((offset + size) < len)
    {			/* we have to send in parts */
      *eof = 0;
      chunk = size;
    }
    else
    {
      *eof = 1;
      chunk = len - offset;
    }

    memcpy (buffer, &(view[offset]), chunk);
  }

  return len;
}



static int
proc_cvsfs_write_view (struct file *file, const char *buffer,
                       unsigned long size, void *data)
{
  struct cvsfs_sb_info *sb;
  char *line;
  char *filemask;
  char *rule;
  char *ptr;
  int len;

  if (data == NULL)
    return -EINVAL;

  sb = (struct cvsfs_sb_info *) data;

  if (file->private_data == NULL)
    cvsfs_reset_viewrule (sb);

  ++(file->private_data);	/* this expects 'private_data' initialized to 0 at open */

  for (ptr = (char *) buffer, len = 0;
       (*ptr != '\n') && (*ptr != '\0') && (len <= size); ++ptr, ++len);

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



static int
proc_cvsfs_write_control (struct file *file, const char *buffer,
                          unsigned long size, void *data)
{
  struct cvsfs_sb_info *sb;
  char *line;
  char *command;
  char *parameter;
  char *ptr;
  int len;

  if (data == NULL)
    return -EINVAL;

  sb = (struct cvsfs_sb_info *) data;

  ++(file->private_data);	/* this expects 'private_data' initialized to 0 at open */

  for (ptr = (char *) buffer, len = 0;
       (*ptr != '\n') && (*ptr != '\0') && (len <= size); ++ptr, ++len);

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
