/***************************************************************************
                          devfs.h  -  cvsfs devfs interface
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

#ifndef __DEVFS_H__
#define __DEVFS_H__

#include <linux/config.h>
#include <linux/wait.h>
//#include <linux/spinlock.h>
#include <asm/semaphore.h>

/* forward references */
struct cvsfs_sb_info;
//struct __wait_queue_head;
#ifdef CONFIG_DEVFS_FS
struct devfs_entry;
#endif

struct cvsfs_queue
{
  wait_queue_head_t	full;
  wait_queue_head_t	empty;
  int			size;
  int			readpos;
  char			*data;
};

struct cvsfs_device_info
{
#ifdef CONFIG_DEVFS_FS
  struct devfs_entry	*handle;
#else
  int			major;
#endif
  struct cvsfs_queue	in;	/* receive buffer - data from daemon */
  struct cvsfs_queue	out;	/* send buffer - data to daemon */
  int			in_use;	/* device has been opened */
//  spinlock_t		lock;
  struct semaphore	lock;
  int			daemon_notified;
  int			unmount_in_process;
};



int cvsfs_devfs_init ();
void cvsfs_devfs_cleanup ();
int cvsfs_devfs_user_init (struct cvsfs_sb_info *);
void cvsfs_devfs_user_cleanup (struct cvsfs_sb_info *);

/* queue handling routines */
int cvsfs_add_request (struct cvsfs_sb_info *, const char *, int);
int cvsfs_retrieve_data (struct cvsfs_sb_info *, char **);



#endif
