/***************************************************************************
                          devfs.c  -  cvsfs device/devfs support
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
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/wait.h>		/* wait queue support */

#include "superblock.h"

#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif

#ifndef CONFIG_DEVFS_FS
/* module parameters */
static int device_id = 245;
MODULE_PARM (device_id, "i");
MODULE_PARM_DESC (device_id, "The device id the cvsfs device will be registered (default: 245)");
#endif

//#define __DEBUG__

/* the name under which the device firms */
const char *devfs_name = "cvsfs";

/* keeps the major device id returned by the kernel */
static int cvsfs_device_major = 0;
#ifdef CONFIG_DEVFS_FS
static devfs_handle_t cvsfs_devfs_dir;
static devfs_handle_t cvsfs_devfs_control;
#endif

static wait_queue_head_t cvsfs_restart_signal;


/* forward references to file operations on control device */
ssize_t cvsfs_devfs_control_read (struct file *, char *, size_t, loff_t *);
int cvsfs_devfs_control_release (struct inode *, struct file *);
unsigned int cvsfs_devfs_control_poll (struct file *, struct poll_table_struct *);

struct file_operations cvsfs_devfs_control_fops = {
        					    read:	cvsfs_devfs_control_read,
    						    release:	cvsfs_devfs_control_release,
						    poll:	cvsfs_devfs_control_poll,
						    owner:	THIS_MODULE
						  };



/* forward references to file operations on device */
ssize_t cvsfs_devfs_read (struct file *, char *, size_t, loff_t *);
ssize_t cvsfs_devfs_write (struct file *, const char *, size_t, loff_t *);
int cvsfs_devfs_open (struct inode *, struct file *);
int cvsfs_devfs_release (struct inode *, struct file *);
unsigned int cvsfs_devfs_poll (struct file *, struct poll_table_struct *);

struct file_operations cvsfs_devfs_fops = {
        				    read:	cvsfs_devfs_read,
					    write:	cvsfs_devfs_write,
					    open:	cvsfs_devfs_open,
					    release:	cvsfs_devfs_release,
					    poll:	cvsfs_devfs_poll,
					    owner:	THIS_MODULE
					  };



/* cleanup allocated space when the queues are shut down */
static int cvsfs_down_queues (struct cvsfs_device_info * dev)
{
  /* dispose queue contents */
  if (dev->in.size != 0)
  {
    kfree (dev->in.data);
    dev->in.data = NULL;
    dev->in.size = 0;
  }

  if (dev->out.size != 0)
  {
    kfree (dev->out.data);
    dev->out.data = NULL;
    dev->out.size = 0;
  }

  return 0;
}



/* add a transfer block to a queue */
static int
cvsfs_handle_add_to_buffer (struct cvsfs_device_info * dev, struct cvsfs_queue * dest, char * data, int size)
{
  int ret = 0;
  wait_queue_t wait;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: handle_add_to_buffer: adding (size=%d) data -->%s\n", size,  data);
#endif

  if (dev->in_use == 0)
  {
    kfree (data);
    return 0;			/* device closed - skip request */
  }

//  spin_lock (&(dev->lock));
  if (down_interruptible (&(dev->lock)))
  {				/* lock can not be obtained */
    kfree (data);
    return 0;
  }

  init_waitqueue_entry (&wait, current);

  add_wait_queue (&(dest->empty), &wait);
  while (1)
  {
    set_current_state (TASK_INTERRUPTIBLE);

    if (dest->size == 0)
      break;

    if (dev->in_use == 0)
      break;			/* device closed while waiting */

//    spin_unlock (&(dev->lock));
    up (&(dev->lock));

    schedule ();

//    spin_lock (&(dev->lock));
    if (down_interruptible (&(dev->lock)))
    {
      set_current_state (TASK_RUNNING);
      remove_wait_queue (&(dest->empty), &wait);
      kfree (data);
      
      return 0;
    }
  }
  set_current_state (TASK_RUNNING);
  remove_wait_queue (&(dest->empty), &wait);

  if (dev->in_use == 0)
  {				/* device closed - remove request */
    kfree (data);
    ret = 0;
  }
  else
  {
    dest->data = data;
    dest->size = size;
    dest->readpos = 0;

    ret = size;			/* all went well - return the number of bytes */
  }

//  spin_unlock (&(dev->lock));
  up (&(dev->lock));

  if (ret > 0)
    wake_up (&(dest->full));

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: handle_add_to_buffer: exit code %d\n", ret);
#endif

  return ret;
}



/* retrieve a transfer block from a queue */
static int
cvsfs_handle_get_from_buffer (struct cvsfs_device_info * dev, struct cvsfs_queue * source, char ** data)
{
  int ret = 0;
  wait_queue_t wait;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: handle_get_from_buffer - entry\n");
#endif

//  spin_lock (&(dev->lock));
  if (down_interruptible (&(dev->lock)))
    return 0;

  if (dev->in_use == 0)
    ret = 0;			/* device closed - skip request */

  init_waitqueue_entry (&wait, current);

  add_wait_queue (&(source->full), &wait);
  while (1)
  {
    set_current_state (TASK_INTERRUPTIBLE);

    if (source->size != 0)
      break;

    if (dev->in_use == 0)
      break;

//    spin_unlock (&(dev->lock));
    up (&(dev->lock));

    schedule ();

//    spin_lock (&(dev->lock));
    if (down_interruptible (&(dev->lock)))
    {
      set_current_state (TASK_RUNNING);
      remove_wait_queue (&(source->full), &wait);
      
      return 0;
    }
  }
  set_current_state (TASK_RUNNING);
  remove_wait_queue (&(source->full), &wait);

  if (dev->in_use == 0)
    ret = 0;			/* device closed - skip request */
  else
  {
    *data = source->data;
    ret = source->size;
    
    source->data = NULL;
    source->size = 0;
    source->readpos = 0;
  }

//  spin_unlock (&(dev->lock));
  up (&(dev->lock));

  if (ret > 0)
    wake_up (&(source->empty));

#ifdef __DEBUG__
  if (ret > 0)
    printk (KERN_DEBUG "cvsfs: handle_get_from_buffer: (size=%d) data -->%s\n", ret, (const char *) *data);
#endif

  return ret;
}



/* send a transfer block to the daemon */
/* returns: 0 ..... nothing placed - no daemon present */
/*          >0 .... data inserted                      */
/*          <0 .... memory squeeze                     */
/* ownership of 'data' was not taken over !            */
int
cvsfs_add_request (struct cvsfs_sb_info * info, const char * data, int size)
{
  char * buf;

  if (!data)
    return -1;

  buf = kmalloc (size, GFP_KERNEL);
  if (!buf)
    return -1;			/* memory squeeze - abort */

  memcpy (buf, data, size);

  return cvsfs_handle_add_to_buffer (&(info->device), &(info->device.out), buf, size);
}



/* retrieve a transfer block from the daemon           */
/* returns: 0 ..... nothing placed - no daemon present */
/*          >0 .... data retrieved                     */
int
cvsfs_retrieve_data (struct cvsfs_sb_info * info, char ** data)
{
  return cvsfs_handle_get_from_buffer (&(info->device), &(info->device.in), data);
}



/* put a received block from daemon into buffer */
/* the ownership of 'data' will be taken over ! */
static int
cvsfs_put_data (struct cvsfs_sb_info * info, char * data, int size)
{
  return cvsfs_handle_add_to_buffer (&(info->device), &(info->device.in), data, size);
}


/* get a block to transmit to the daemon from the buffer */
static int
cvsfs_get_data (struct cvsfs_sb_info * info, char ** data)
{
  return cvsfs_handle_get_from_buffer (&(info->device), &(info->device.out), data);
}



/* init /dev entries as such */
int
cvsfs_devfs_init ()
{
  int result;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_init - start\n");
#endif

#ifdef CONFIG_DEVFS_FS
  cvsfs_devfs_dir = devfs_mk_dir (NULL, devfs_name, NULL);  
  cvsfs_devfs_control = devfs_register (cvsfs_devfs_dir, "0",
                                        DEVFS_FL_AUTO_DEVNUM,
                                        0, 0, S_IFCHR | S_IRUSR | S_IWUSR,
  					&cvsfs_devfs_control_fops, 0);
#else
  if ((result = register_chrdev (device_id, devfs_name, &cvsfs_devfs_fops)) < 0)
  {
    printk (KERN_WARNING "cvsfs: devfs_init - can not allocate device entry\n");
    
    return result;
  }
  
  cvsfs_device_major = device_id;  

  init_waitqueue_head (&(cvsfs_restart_signal));
#endif
      
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_init - successful completed\n");
#endif

  return 0;
}



/* shut down /dev entries */
void
cvsfs_devfs_cleanup ()
{
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_cleanup - start\n");
#endif
  
#ifdef CONFIG_DEVFS_FS
  devfs_unregister (cvsfs_devfs_control);
  devfs_unregister (cvsfs_devfs_dir);  
#else
  if (cvsfs_device_major != 0)
    unregister_chrdev (cvsfs_device_major, devfs_name);
    
  cvsfs_device_major = 0;
#endif

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_cleanup - successful completed\n");
#endif
}



/* init mountpoint specific device in /dev */
int
cvsfs_devfs_user_init (struct cvsfs_sb_info * info)
{
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_user_init - mount point %s\n", info->mount.mountpoint);
#endif

  if (info->id >= 1)
  {
#ifdef CONFIG_DEVFS_FS
    info->device.handle = devfs_register (cvsfs_devfs_dir, info->idstring,
                                          DEVFS_FL_AUTO_DEVNUM,
                                          0, 0, S_IFCHR | S_IRUSR | S_IWUSR,
  					  &cvsfs_devfs_fops, info);
#else
    info->device.major = cvsfs_device_major;

    if (info->id > 255)
    {
      printk (KERN_DEBUG "cvsfs: devfs_user_init failed - too many mounts active\n");
      
      return -1;
    }
    /* setting the info part of the file structure must be done in the 'open' call */
#endif
  }

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_user_init - device allocated\n");
#endif

  init_waitqueue_head (&(info->device.out.full));
  init_waitqueue_head (&(info->device.out.empty));
  init_waitqueue_head (&(info->device.in.full));
  init_waitqueue_head (&(info->device.in.empty));
  info->device.out.size = 0;
  info->device.out.data = NULL;
  info->device.in.size = 0;
  info->device.in.data = NULL;

//  spin_lock_init (&(info->device.lock));
  init_MUTEX (&(info->device.lock));
  info->device.in_use = 0;			/* device not opened */

  info->device.unmount_in_process = 0;
  
  info->device.daemon_notified = 0;		/* control daemon not notified */
  wake_up_interruptible_sync (&cvsfs_restart_signal);  /* do not schedule */

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_user_init - successful completed\n");
#endif

  return 0;
}



/* release mountpoint specific device in /dev */
void
cvsfs_devfs_user_cleanup (struct cvsfs_sb_info * info)
{
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_user_cleanup - mount point %s\n", info->mount.mountpoint);
#endif

//  spin_lock (&(info->device.lock));
  down_interruptible (&(info->device.lock));

  info->device.unmount_in_process = 1;

  /* signal daemon (if one is attached) to exit */
  if (info->device.in_use != 0)
  {
    if (info->device.out.size != 0)
      kfree (info->device.out.data);
      
    info->device.out.data = kmalloc (5, GFP_KERNEL);
    if (info->device.out.data)
    {
      strcpy (info->device.out.data, "quit");
      info->device.out.size = 5;
    }

//    spin_unlock (&(info->device.lock));
    up (&(info->device.lock));

    wake_up (&(info->device.out.full));

#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: devfs_user_cleanup - wait for daemon to exit\n");
#endif

    /* wait until the data was sent to the daemon */
    wait_event_interruptible ((info->device.out.empty), ((info->device.in_use == 0) || (info->device.out.size == 0)));

#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: devfs_user_cleanup - daemon exited\n");
#endif
  }
  else
//    spin_unlock (&(info->device.lock));
    up (&(info->device.lock));

#ifdef CONFIG_DEVFS_FS
  if (info->id >= 1)
    devfs_unregister (info->device.handle);
#else
  /* nothing to do for standard /dev file system */
#endif

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_user_cleanup - successful completed\n");
#endif
}



ssize_t
cvsfs_devfs_control_read (struct file * f, char * buf, size_t size, loff_t * offset)
{
  struct cvsfs_sb_info	*info;
  char buffer[64];

  /* i expect that the buffer presented is at least 64 Bytes in size - so no check done */
  size = 0;

  info = cvsfs_read_lock_superblock_list ();  

  while (info != NULL)
  {
    if ((info->device.daemon_notified == 0) &&
        (info->device.in_use == 0) &&
	(info->device.unmount_in_process == 0))
      break;

    info = info->next;
  }

  if (info != NULL)
  {
    size = sprintf (buffer, "%s\n", info->idstring);

    info->device.daemon_notified = 1;	/* notification handled */
  }
  
  cvsfs_read_unlock_superblock_list ();

  if (size > 0)
    if (copy_to_user (buf, buffer, size))
      return -EFAULT;

  return size;
}



unsigned int
cvsfs_devfs_control_poll (struct file * f, struct poll_table_struct * wait)
{
  struct cvsfs_sb_info	*info;
  unsigned int		mask = 0;

  info = cvsfs_read_lock_superblock_list ();  

  while (info != NULL)
  {
    if ((info->device.daemon_notified == 0) &&
        (info->device.in_use == 0) &&
	(info->device.unmount_in_process == 0))
      break;
      
    info = info->next;
  }

  cvsfs_read_unlock_superblock_list ();
  
  if (info != NULL)
    mask |= POLLIN | POLLRDNORM;
    
  poll_wait (f, &cvsfs_restart_signal, wait);
      
  return  mask;
}



int
cvsfs_devfs_control_release (struct inode * ino, struct file * f)
{
  struct cvsfs_sb_info	*info;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_release\n");
#endif

  /* reset all notification flags */
  info = cvsfs_read_lock_superblock_list ();  

  while (info != NULL)
  {
    if (info->device.daemon_notified != 0)
      info->device.daemon_notified = 0;
      
    info = info->next;
  }

  cvsfs_read_unlock_superblock_list ();

  MOD_DEC_USE_COUNT;
 
  printk (KERN_DEBUG "cvsfs: devfs_release - device /dev/cvsfs/0 closed\n");
  
  return 0;
}



ssize_t
cvsfs_devfs_read (struct file * f, char * buf, size_t size, loff_t * offset)
{
  struct cvsfs_sb_info	*info;
  int ret;

#ifndef CONFIG_DEVFS_FS
  if (f->private_data == NULL)
    return cvsfs_devfs_control_read (f, buf, size, offset);
#endif

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_read - entry (at offset %d)\n", (int) *offset);
#endif

  info = (struct cvsfs_sb_info *) f->private_data;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_read - claim mutex\n");
#endif

//  spin_lock (&(info->device.lock));
  if (down_interruptible (&(info->device.lock)))
    return -ERESTARTSYS;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_read - mutex obtained\n");
#endif

  if (info->device.out.size == 0)
  {
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: devfs_read - no data to send\n");
#endif

//    spin_unlock (&(info->device.lock));
    up (&(info->device.lock));
    
    if (f->f_flags & O_NONBLOCK)
      return -EAGAIN;

#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: devfs_read - wait for new data\n");
#endif

    if (wait_event_interruptible (info->device.out.full, (info->device.out.size > 0)))
      return -ERESTARTSYS;
    
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: devfs_read - claim mutex\n");
#endif

//    spin_lock (&(info->device.lock));
    if (down_interruptible (&(info->device.lock)))
      return -ERESTARTSYS;
  }
      
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_read - data available\n");
#endif

  if (info->device.out.readpos < info->device.out.size)
  {
    int chunk;
    
    chunk = info->device.out.size - info->device.out.readpos;
    if (chunk > size)
      chunk = size;
    
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: devfs_read - (size=%d) data -->%s\n", chunk, info->device.out.data);
#endif

    if (copy_to_user (buf, &((info->device.out.data)[info->device.out.readpos]), chunk))
    {
//      spin_unlock (&(info->device.lock));
      up (&(info->device.lock));
      
      ret = -EFAULT;
    }
    
    info->device.out.readpos += chunk;
    *offset += chunk;
    ret = chunk;
      
    /* last chunk read - release the buffer */
    if (info->device.out.readpos >= info->device.out.size)
    {
      char * dummy;
	
//      spin_unlock (&(info->device.lock));
      up (&(info->device.lock));
      cvsfs_get_data (info, &dummy);
	
      kfree (dummy);
      
#ifdef __DEBUG__
      printk (KERN_DEBUG "cvsfs: devfs_read (last block) - exit code %d\n", ret);
#endif

      return chunk;
    }
        
    ret = chunk;
  }
  else
    ret = 0;

//  spin_unlock (&(info->device.lock));
  up (&(info->device.lock));

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_read - exit code %d\n", ret);
#endif

  return ret;
}



ssize_t
cvsfs_devfs_write (struct file * f, const char * buf, size_t size, loff_t * offset)
{
  struct cvsfs_sb_info	*info;
  char * data;
  int ret;  

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_write - writing (size=%d) data -->%s\n", size, buf);
#endif

  if (f->private_data == NULL)
    return -EINVAL;

  info = (struct cvsfs_sb_info *) f->private_data;

  data = kmalloc (size + 1, GFP_KERNEL);
  if (data)
  {
    if (copy_from_user (data, buf, size))
      ret = -EFAULT;
    else
    {
      data[size] = '\0';
      ret = cvsfs_put_data (info, data, size);
    }
  }
  else
    ret = -EFAULT;

  if (ret == 0)		/* should never happen ! */
    ret = size;		/* indicate full transfer to prevent endless loop */

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_write - exit code %d\n", ret);
#endif

  return ret;
}



int
cvsfs_devfs_open (struct inode * ino, struct file * f)
{
  int			retval = 0;
  struct cvsfs_sb_info	*info;
#ifndef CONFIG_DEVFS_FS
  int			minor;
#endif

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_open - start\n");
#endif

#ifdef CONFIG_DEVFS_FS
  if (f->private_data == NULL)
    return -EINVAL;

  info = (struct cvsfs_sb_info *) f->private_data;
#else
  minor = MINOR (ino->i_rdev);

  if (minor == 0)
  {
    MOD_INC_USE_COUNT;
  
    printk (KERN_DEBUG "cvsfs: devfs_open - device /dev/cvsfs/0 opened\n");
  }
  else
  {
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: devfs_open - minor device %d\n", minor);
#endif

    info = cvsfs_read_lock_superblock_list ();
    while ((info != 0) && (info->id != minor))
      info = info->next;
    cvsfs_read_unlock_superblock_list ();
      
    if (info == 0)
    {
      printk (KERN_WARNING "cvsfs: devfs_open - minor %d of this device not assigned\n", minor);
    
      return -EINVAL;
    }

#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: devfs_open - mount point %s\n", info->mount.mountpoint);
#endif

    f->private_data = info;
#endif  

    /* obtain the lock */
//    if (down_interruptible (&(info->device.lock)))
//      return -ERESTARTSYS;
  
    if (info->device.in_use != 0)
      retval = -EINVAL;			/* device already open - invalid */
    else
    {
      ++(info->device.in_use);
  
      retval = 0;
    }  

//  up (&(info->device.lock));
  
    if (retval == 0)
    {
      MOD_INC_USE_COUNT;

      /* place the parm string as first info block for the daemon */
      cvsfs_add_request (info, info->parm_string, strlen (info->parm_string) + 1);

      printk (KERN_DEBUG "cvsfs: devfs_open - device /dev/cvsfs/%s opened\n", info->idstring);
    }
#ifndef CONFIG_DEVFS_FS
  }
#endif

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_open - exit code %d\n", retval);
#endif
  
  return retval;
}



int
cvsfs_devfs_release (struct inode * ino, struct file * f)
{
  struct cvsfs_sb_info	*info;
#ifndef CONFIG_DEVFS_FS
  int			minor;
#endif

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: devfs_release\n");
#endif

#ifndef CONFIG_DEVFS_FS
  minor = MINOR (ino->i_rdev);

  if (minor == 0)
    return cvsfs_devfs_control_release (ino, f);
#endif

  if (f->private_data == NULL)
    return -EINVAL;

  info = (struct cvsfs_sb_info *) f->private_data;

  /* obtain the lock */
//  spin_lock (&(info->device.lock));
//  if (down_interruptible (&(info->device.lock)))
//    return -ERESTARTSYS;
  
  --(info->device.in_use);

  if (info->device.unmount_in_process == 0)
    info->device.daemon_notified = 0;		/* daemon must be restarted */
  wake_up_interruptible_sync (&cvsfs_restart_signal);  /* do not schedule */
  
  cvsfs_down_queues (&(info->device));
  
  /* release the lock */
//  spin_unlock (&(info->device.lock));
//  up (&(info->device.lock));

  MOD_DEC_USE_COUNT;

  printk (KERN_DEBUG "cvsfs: devfs_release - device /dev/cvsfs/%s closed\n", info->idstring);

  return 0;
}



unsigned int cvsfs_devfs_poll (struct file * f, struct poll_table_struct * wait)
{
  struct cvsfs_sb_info	*info;
  unsigned int		mask = 0;

#ifndef CONFIG_DEVFS_FS
  if (f->private_data == NULL)
    return cvsfs_devfs_control_poll (f, wait);
#endif

  info = (struct cvsfs_sb_info *) f->private_data;

//  spin_lock (&(info->device.lock));
  if (down_interruptible (&(info->device.lock)))
    return -ERESTARTSYS;

  poll_wait (f, &(info->device.in.empty), wait);
  poll_wait (f, &(info->device.out.full), wait);

  if (info->device.in.size == 0)
    mask |= POLLIN | POLLRDNORM;

  if (info->device.out.size != 0)
    mask |= POLLOUT | POLLWRNORM;

//  spin_unlock (&(info->device.lock));
  up (&(info->device.lock));

  return  mask;
}
