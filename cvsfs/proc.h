/***************************************************************************
                          proc.h  -  description
                             -------------------
    begin                : Fri May 18 2001
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

#ifndef __PROC_H__
#define __PROC_H__

#include <linux/fs.h>



/* forward references for structures */
struct semaphore;
struct cvsfs_sb_info;
struct cvsfs_fattr;

struct cvsfs_proc_info
{
  struct semaphore	lock;
  char			*view;
};



/* functions required by file system */
char * cvsfs_get_file (struct cvsfs_sb_info *, char *, int);
int cvsfs_get_attr (struct cvsfs_sb_info *, char *, struct cvsfs_fattr *);
int cvsfs_read (struct cvsfs_sb_info *, char *, char *, loff_t, size_t, char *);
int cvsfs_write (struct cvsfs_sb_info *, char *, char *, loff_t, size_t, char *);
int cvsfs_create_dir (struct cvsfs_sb_info *, char *, int, struct cvsfs_fattr *);
int cvsfs_remove_dir (struct cvsfs_sb_info *, char *);
int cvsfs_create_file (struct cvsfs_sb_info *, char *, int, struct cvsfs_fattr *);
int cvsfs_remove_file (struct cvsfs_sb_info *, char *);
int cvsfs_truncate_file (struct cvsfs_sb_info *, char *);
int cvsfs_move (struct cvsfs_sb_info *, char *, char *);
int cvsfs_ioctl (struct cvsfs_sb_info *, int, char *, char **);

/* functions required by procfs interface */
int cvsfs_get_view (struct cvsfs_sb_info *, char **);
void cvsfs_reset_viewrule (struct cvsfs_sb_info *);
int cvsfs_append_viewrule (struct cvsfs_sb_info *, char *, char *);
int cvsfs_control_command (struct cvsfs_sb_info *, char *, char *);



#endif
