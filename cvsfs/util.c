/***************************************************************************
                          util.c  -  description
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

#include "cvsfs_config.h"
#include "util.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/dcache.h>
#include <linux/slab.h>

//#include <net/ip.h>

//#include "inode.h"
//#include "cache.h"
//#include "socket.h"



/* duplicate string in newly allocated space */
char *
strdup (const char * s)
{
  char * p;
  
  p = kmalloc (strlen (s) + 1, GFP_KERNEL);
  if (p)
    strcpy (p, s);

  return p;
}



/* skips whitespaces (blancs and tabs) */
char *
cvsfs_skip_whitespace (char * str)
{
  for (; (*str != '\0') && (*str == ' ') && (*str == '\t'); ++str);
  
  return str;
}



/* trim the end of the string (kills blancs and tabs) */
char *
cvsfs_rtrim (char * str)
{
  int len;
  char *ptr;
  
  if ((len = strlen (str)) == 0)
    return str;

  for (ptr = &(str[len - 1]); (len > 0) && (*ptr == ' ') && (*ptr == '\t'); --ptr, --len)
    *ptr = '\0';

  return str;
}



/* evaluate absolute path from dentry structure */
int
cvsfs_get_name (struct dentry * d, char * name, int maxsize)
{
  int len = 0;
  struct dentry *p;
  
  /* first step up to the top level and calculate path size */
  for (p = d; p != p->d_parent; p = p->d_parent)
    len += p->d_name.len + 1;
    
  if (len >= maxsize)
    return -1;

  /* special case: we are already at top level */    
  if (len == 0)
  {
    name[0] = '/';
    name[1] = '\0';
  }
  else
  {
    name[len] = '\0';
    for (p = d; p != p->d_parent; p = p->d_parent)
    {
      len -= p->d_name.len;
      strncpy (&(name[len]), p->d_name.name, p->d_name.len);
      --len;
      name[len] = '/';
    }
  }
  
  return 0;
}
