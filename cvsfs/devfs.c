/***************************************************************************
                          util.c  -  cvsfs devfs support
                             -------------------
    begin                : Thu Nov 29 2001
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
#include "devfs.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
//#include <linux/ctype.h>

//#include <net/ip.h>

#include "inode.h"
//#include "cache.h"
//#include "socket.h"

#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif


/* list to keep the assignments  mount point <--> minor numbers */

struct devfs_mount_entry
{
  struct cvsfs_sb_info	*info;
#ifdef CONFIG_DEVFS_FS
  devfs_handle_t	handle;
#endif
};

#define MAX_DEVICES 256
const int max_devices = MAX_DEVICES;
static struct devfs_mount_entry	mounts[MAX_DEVICES];

/* the name under which the device firms */
const char *devfs_name = "cvsfs";

/* keeps the major device id returned by the kernel */
static int cvsfs_device_major = 0;
#ifdef CONFIG_DEVFS_FS
static devfs_handle_t cvsfs_devfs_dir;
#endif



/* forward references to file operations on device */
ssize_t cvsfs_devfs_read (struct file *, char *, size_t, loff_t *);
int cvsfs_devfs_open (struct inode *, struct file *);
int cvsfs_devfs_release (struct inode *, struct file *);

struct file_operations cvsfs_devfs_fops = {
        				    read:	cvsfs_devfs_read,
					    open:	cvsfs_devfs_open,
					    release:	cvsfs_devfs_release,
					    owner:	THIS_MODULE
					  };



/* local routines to keep the list of mounts up to date */

int
cvsfs_devfs_add_mount (struct cvsfs_sb_info * info)
{
  int loop;

  printk (KERN_DEBUG "cvsfs: devfs_add_mount - mount point %s\n", info->mnt.mountpoint);

  // should be enclosed in a semaphore
  for (loop = 0; loop < max_devices; ++loop)
    if (mounts[loop].info == NULL)
    {
      mounts[loop].info = info;
      
      printk (KERN_DEBUG "cvsfs: devfs_add_mount - allocated at index %d\n", loop);

      return loop;
    }
      
  printk (KERN_DEBUG "cvsfs: devfs_add_mount - not allocated\n");

  return -1;
}



int
cvsfs_devfs_remove_mount (struct cvsfs_sb_info * info)
{
  printk (KERN_DEBUG "cvsfs: devfs_remove_mount - mount point %s\n", info->mnt.mountpoint);

  // should be enclosed in a semaphore
  if (info->mnt.device_id >= 0)
  {
    mounts[info->mnt.device_id].info = NULL;	// mark as free

    printk (KERN_DEBUG "cvsfs: devfs_remove_mount - removed from index %d\n", info->mnt.device_id);
      
    return info->mnt.device_id;
  }
      
  printk (KERN_DEBUG "cvsfs: devfs_remove_mount - not removed\n");

  return -1;
}



/* init /dev entries as such */
int
cvsfs_devfs_init ()
{
  int result;

  printk (KERN_DEBUG "cvsfs: devfs_init - start\n");

  memset (&mounts, 0, sizeof (mounts));

#ifdef CONFIG_DEVFS_FS
  cvsfs_devfs_dir = devfs_mk_dir (NULL, devfs_name, NULL);  
#else
  if ((result = register_chrdev (0, devfs_name, &cvsfs_devfs_fops)) < 0)
  {
    printk (KERN_WARNING "cvsfs: devfs_init - can not allocate device entry\n");
    
    return result;
  }
  
  cvsfs_device_major = result;  
#endif
      
  printk (KERN_DEBUG "cvsfs: devfs_init - successful exit\n");

  return 0;
}



void
cvsfs_devfs_cleanup ()
{
  printk (KERN_DEBUG "cvsfs: devfs_cleanup - start\n");
  
#ifdef CONFIG_DEVFS_FS
  devfs_unregister (cvsfs_devfs_dir);  
#else
  if (cvsfs_device_major != 0)
    unregister_chrdev (cvsfs_device_major, devfs_name);
#endif

  printk (KERN_DEBUG "cvsfs: devfs_cleanup - successful exit\n");
}



int
cvsfs_devfs_user_init (struct cvsfs_sb_info * info)
{
  int minor;
#ifdef CONFIG_DEVFS_FS
  char buffer[32];
#endif
  
  printk (KERN_DEBUG "cvsfs: devfs_user_init - mount point %s\n", info->mnt.mountpoint);

  minor = cvsfs_devfs_add_mount (info);

  info->mnt.device_id = minor;

  if (minor >= 0)
  {
#ifdef CONFIG_DEVFS_FS
    sprintf (buffer, "cvsfs%i", minor);
    mounts[minor].handle = devfs_register (cvsfs_devfs_dir, buffer,
                                           DEVFS_FL_AUTO_DEVNUM,
                                           0, 0, S_IFCHR | S_IRUGO | S_IWUGO,
  					   &cvsfs_devfs_fops, info);
#else
    // setting the info part of the file structure must be done in the 'open' call
#endif
  }

  printk (KERN_DEBUG "cvsfs: devfs_user_init - successful exit\n");

  return 0;
}



void
cvsfs_devfs_user_cleanup (struct cvsfs_sb_info * info)
{
  int minor;

  printk (KERN_DEBUG "cvsfs: devfs_user_cleanup - mount point %s\n", info->mnt.mountpoint);

  minor = cvsfs_devfs_remove_mount (info);
  
#ifdef CONFIG_DEVFS_FS
  if (minor >= 0)
    devfs_unregister (mounts[minor].handle);
#else
#endif

  printk (KERN_DEBUG "cvsfs: devfs_user_cleanup - successful exit\n");
}



int
cvsfs_devfs_get_major ()
{
#ifdef CONFIG_DEVFS_FS
  return -1;
#else
  return cvsfs_device_major;
#endif
}



ssize_t
cvsfs_devfs_read (struct file * f, char * buf, size_t size, loff_t * offset)
{
  return -EINVAL;
}



int
cvsfs_devfs_open (struct inode * ino, struct file * f)
{
#ifndef CONFIG_DEVFS_FS
  int			minor;
  struct cvsfs_sb_info	*info;
#endif

  printk (KERN_DEBUG "cvsfs: devfs_open - start\n");

#ifdef CONFIG_DEVFS_FS
#else
  minor = MINOR (ino->i_rdev);

  printk (KERN_DEBUG "cvsfs: devfs_open - minor device %d\n", minor);

  info = mounts[minor].info;
  
  if (info == 0)
  {
    printk (KERN_WARNING "cvsfs: devfs_open - minor %d of this device not assigned\n", minor);
    
    return -EINVAL;
  }

  printk (KERN_DEBUG "cvsfs: devfs_open - mount point %s\n", info->mnt.mountpoint);

  f->private_data = info;
#endif

  MOD_INC_USE_COUNT;

  return 0;
}



int
cvsfs_devfs_release (struct inode * ino, struct file * f)
{
  MOD_DEC_USE_COUNT;

  printk (KERN_DEBUG "cvsfs: devfs_release - i am here\n");
  
  return 0;
}
