/***************************************************************************
                          util.h  -  description
                             -------------------
    begin                : Thu Nov 15 2001
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

#ifndef __UTIL_H__
#define __UTIL_H__



/* forward reference */
struct dentry;



char * strdup (const char *);
char * cvsfs_skip_whitespace (char *);
char * cvsfs_rtrim (char *);
int cvsfs_get_name (struct dentry *, char *, int);



#endif
