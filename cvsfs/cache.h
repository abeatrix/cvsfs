/***************************************************************************
                          cache.h  -  description
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

#ifndef __CHACHE_H__
#define __CHACHE_H__

#include <linux/types.h>
//#include "inode.h"



struct cvsfs_dir_entry
{
  umode_t	mode;
  char		*name;
  off_t		size;
  nlink_t	nlink;
  unsigned long	blocksize;
  unsigned long	blocks;
  time_t	date;
  char          *version;
};

struct cvsfs_dirlist_node
{
  struct cvsfs_dirlist_node	*prev;
  struct cvsfs_dirlist_node	*next;
  struct cvsfs_dir_entry	entry;
};

struct cvsfs_directory
{
  struct cvsfs_dirlist_node	*head;
  int				valid;
  time_t			time;
  char				*name;
};



struct cvsfs_sb_info;



int cvsfs_cache_init ();
int cvsfs_cache_empty ();
struct cvsfs_directory *cvsfs_cache_get (struct cvsfs_sb_info *, char *, char *);



#endif
