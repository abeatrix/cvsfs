/***************************************************************************
                          procfs.h  -  cvsfs procfs interface
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

#ifndef __PROCFS_H__
#define __PROCFS_H__

//include <linux/fs.h>			/* char dev definitions here */

/* forward reference to superblock.h and proc_fs.h */
struct cvsfs_sb_info;
struct proc_dir_entry;

/* procfs data */
struct cvsfs_procfs_info
{
//  int			id;		/* sequence number */
//  char			dir[8];		/* dito. as string */
  struct proc_dir_entry	*data;		/* procfs base dir */
};



int cvsfs_procfs_init ();
void cvsfs_procfs_cleanup ();
int cvsfs_procfs_user_init (struct cvsfs_sb_info *);
void cvsfs_procfs_user_cleanup (struct cvsfs_sb_info *);



#endif
