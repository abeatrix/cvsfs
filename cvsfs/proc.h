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
struct cvsfs_sb_info;
struct cvsfs_fattr;
struct cvsfs_directory;
struct cvsfs_dir_entry;



void cvsfs_init_root_dirent (struct cvsfs_sb_info *, struct cvsfs_fattr *);
int cvsfs_parse_options (struct cvsfs_sb_info *, void *);
inline void cvsfs_lock (struct cvsfs_sb_info *);
inline void cvsfs_unlock (struct cvsfs_sb_info *);
int cvsfs_get_fattr (struct cvsfs_sb_info *, char *, struct cvsfs_dir_entry *);
int cvsfs_loaddir (struct cvsfs_sb_info *, char *, struct cvsfs_directory *, char *);
int cvsfs_get_name (struct dentry *, char *);
int cvsfs_get_attr (struct dentry *, struct cvsfs_fattr *, struct cvsfs_sb_info *);
int cvsfs_read (struct dentry *, unsigned long offset, unsigned long count, char *);



#endif
