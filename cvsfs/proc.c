/***************************************************************************
                          proc.c  -  description
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

/***************************************************************************
 *                                                                         *
 *   This module is a mediator between the requests the VFS driver has     *
 *   the device interface which communicate to a deamon on the other side. *
 *                                                                         *
 *   The function 'cvsfs_serialize_request' serializes all requests. It    *
 *   sends the requests in the format                                      *
 *                                                                         *
 *     <command> <options>                                                 *
 *                                                                         *
 *   and expects the responses in the format                               *
 *                                                                         *
 *     <size> <data>                                                       *
 *                                                                         *
 ***************************************************************************/

#include "cvsfs_config.h"
#include "proc.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/slab.h>

#include "dir.h"
#include "file.h"
#include "superblock.h"
#include "devfs.h"
#include "util.h"

//#define __DEBUG__



/* serializes all requests to the daemon                              */
/* returns: 0 when no data have been retrieved (e.g. daemon down)     */
/*          <0 in case of errors                                      */
/*          >0 on success respresenting the number of bytes retrieved */
static int
cvsfs_serialize_request (struct cvsfs_sb_info * info, char * buf, int size, char ** ret)
{
  int retval = 0;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: serialize_request - send command -->%s\n", buf);
#endif
  if (down_interruptible (&(info->proc.lock)))
    return -ERESTARTSYS;
    
  /* lock obtained - continue with work */
  *ret = NULL;

  if ((retval = cvsfs_add_request (info, buf, size)) >= 0)
  {
    if (retval > 0)
    {
      retval = cvsfs_retrieve_data (info, ret);
      if (retval > 0)
      {
        int size;	/* size of the expected data */
        int remain;
        char *ptr;

        size = simple_strtoul (*ret, &ptr, 0);
#ifdef __DEBUG__
        printk (KERN_DEBUG "cvsfs: serialize_request - size=%d, ptr=%s\n", size, ptr);
#endif
	if (size > 0)
	{
          *ptr = '\0';
          ++ptr;
          retval -= (strlen (*ret) + 1);
          remain = size - retval;

#ifdef __DEBUG__
          printk (KERN_DEBUG "cvsfs: serialize_request - after strip length: size=%d, ptr=%s\n", retval, ptr);
#endif
          if (remain > 0)
          {
            char *dummy = *ret;

            /* allocate full buffer */
            *ret = kmalloc (size + 1, GFP_KERNEL);
            if (*ret)
            {
	      (*ret)[size] = '\0';
              memcpy (*ret, ptr, retval);
              kfree (dummy);

              for (dummy = &((*ret)[retval]); remain > 0; remain -= retval, dummy += retval)
              {
                retval = cvsfs_retrieve_data (info, &ptr);
                if (retval == 0)
                  break;		/* exit the loop */

                memcpy (dummy, ptr, retval);
                kfree (ptr);
              }

              retval = size;	/* all expected bytes received */
            }
            else
	    {
	      *ret = dummy;
              retval = -ENOMEM;
	    }
          }
	  else
	    memmove (*ret, ptr, retval + 1);

          if ((retval < 0) && (*ret != NULL))
          {
            kfree (*ret);
            *ret = NULL;
          }
	}
	else
	{
          kfree (*ret);
          *ret = NULL;
	  retval = 0;
	}
      }
    }
  }
  else
    retval = -ENOMEM;

  up (&(info->proc.lock));
  
  return retval;
}



/* this function asks the daemon for a file name */
/* it sends                                      */
/*   ls <index> <directory>                      */
/* it expects                                    */
/*   <file name>                                 */
char *
cvsfs_get_file (struct cvsfs_sb_info * info, char * dir, int index)
{
  char * cmd;
  char *response;
  char number[32];	/* should cover 64 bit numbers (long long) */
  int size;
  int ret;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: get_file - index=%d, dir=%s\n", index, dir);
#endif
  sprintf (number, "%d", index);
  size = 5 + strlen (number) + strlen (dir);
  
  cmd = kmalloc (size, GFP_KERNEL);
  if (!cmd)
    return NULL;

  sprintf (cmd, "ls %d %s", index, dir);
  ret = cvsfs_serialize_request (info, cmd, size, &response);
  kfree (cmd);
  if (ret > 0)
  {
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: get_file - retrieved file name=%s\n", response);
#endif    
    return response;
  }

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: get_file - no more files.\n");
#endif    

  return NULL;
}



/* this function asks the daemon for the attributes of the given file */
/* the request sent to the daemon has this layout:                    */
/*   attr <full path of file>                                         */
/* the expected return is from the daemon is:                         */
/*   <mode> <size> <atime> <mtime> <ctime> <version> '\0'             */
/* all values (except version) are be represented as decimal numbers  */
int
cvsfs_get_attr (struct cvsfs_sb_info * info, char * name, struct cvsfs_fattr * attr)
{
  char * cmd;
  char * response;
  char * ptr;
  int size;
  int ret;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: get_attr for file %s\n", name);
#endif
  size = 6 + strlen (name);
  cmd = kmalloc (size, GFP_KERNEL);
  if (!cmd)
    return -1;

  sprintf (cmd, "attr %s", name);
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: get_attr - send request -->%s\n", cmd);
#endif    
  ret = cvsfs_serialize_request (info, cmd, size, &response);
  kfree (cmd);
  if (ret <= 0)		/* error, daemon not running or empty response */
    return -1;

  /* for security - not to access unallocated memory areas */
//  response[ret - 1] = '\0';

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: get_attr - returned from daemon -->%s\n", response);
#endif    

  /* set fixed value fields */
  attr->f_nlink = 1;
  attr->f_blksize = 1024;

  /* now analyze the string */
  attr->f_mode = simple_strtoul (response, &ptr, 0);
  if (*ptr != '\0')
    ++ptr;
  attr->f_size = simple_strtoul (ptr, &ptr, 0);
  if (*ptr != '\0')
    ++ptr;
  attr->f_atime = simple_strtoul (ptr, &ptr, 0);
  if (*ptr != '\0')
    ++ptr;
  attr->f_mtime = simple_strtoul (ptr, &ptr, 0);
  if (*ptr != '\0')
    ++ptr;
  attr->f_ctime = simple_strtoul (ptr, &ptr, 0);
  if (*ptr != '\0')
    ++ptr;
  attr->f_version = strdup (ptr);

  attr->f_uid = info->mount.uid;
  attr->f_gid = info->mount.gid;

  /* mask the attributes with the given umask */
  if ((attr->f_mode & S_IFDIR) != 0)
    attr->f_mode &= (info->mount.dir_mode | ~S_IRWXUGO);
  else
    if ((attr->f_mode & S_IFREG) != 0)
      attr->f_mode &= (info->mount.file_mode | ~S_IRWXUGO);

  /* calculate the number of blocks */
  attr->f_blocks = (attr->f_size + attr->f_blksize - 1) / attr->f_blksize;

  kfree (response);

  return 0;
}



/* this function reads a file data block                  */
/* the request sent to the daemon looks like this:        */
/*   get <offset> <size> <file name>                      */
/* as result the data is returned.                        */
int
cvsfs_read (struct cvsfs_sb_info * info, char * name, char * version,
	    unsigned long offset, unsigned long count, char * buffer)
{
  char * cmd;
  char * response;
  char number1[32];
  char number2[32];
  int size;
  int ret;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: read - file=%s, offset=%i, count=%i\n", name, (int) offset, (int) count);
#endif
  sprintf (number1, "%lu", offset);
  sprintf (number2, "%lu", count);

  size = 7 + strlen (number1) + strlen (number2) + strlen (name);
  if (version)
    size += strlen (version) + 1;
  cmd = kmalloc (size, GFP_KERNEL);
  if (!cmd)
    return -1;

  sprintf (cmd, "get %s %s %s", number1, number2, name);
  if (version)
  {
    strcat (cmd, " ");
    strcat (cmd, version);
  }
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: read - send request -->%s\n", cmd);
#endif    
  ret = cvsfs_serialize_request (info, cmd, size, &response);
  kfree (cmd);
  if (ret < 0)
    ret = -1;
  else
    if (ret > 0)
      memcpy (buffer, response, ret);

  kfree (response);

  return ret;
}



int
cvsfs_get_view (struct cvsfs_sb_info * info, char ** data)
{
  if (info->proc.view)
  {
    *data = info->proc.view;

    return strlen (info->proc.view);
  }

  return 0;
}



void
cvsfs_reset_viewrule (struct cvsfs_sb_info * info)
{
  printk (KERN_DEBUG "cvsfs: cvsfs_reset_viewrule'\n");
  printk (KERN_DEBUG "cvsfs: cvsfs_reset_viewrule - not implemented'\n");
}



int
cvsfs_append_viewrule (struct cvsfs_sb_info * info, char * filemask, char * rule)
{
  printk (KERN_DEBUG "cvsfs: cvsfs_append_viewrule - filemask: '%s', rule: '%s'\n", filemask, rule);
  printk (KERN_DEBUG "cvsfs: cvsfs_append_viewrule - not implemented\n");

  return 0;
}


int
cvsfs_control_command (struct cvsfs_sb_info * info, char * command, char * parameter)
{
  if (parameter)
    printk (KERN_DEBUG "cvsfs: cvsfs_control_command - command: '%s', parameter '%s'\n", command, parameter);
  else
    printk (KERN_DEBUG "cvsfs: cvsfs_control_command - command: '%s'\n", command);
  printk (KERN_DEBUG "cvsfs: cvsfs_control_command - not implemented\n");

  return 0;
}
