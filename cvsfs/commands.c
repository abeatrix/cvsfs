/***************************************************************************
                          commands.c  -  description
                             -------------------
    begin                : Thu Nov 8 2001
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
#include "commands.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/ctype.h>

#include <net/ip.h>

#include "inode.h"
#include "cache.h"
#include "socket.h"



int cvsfs_command_sequence_co (struct socket * sock, struct cvsfs_sb_info * info, char * dir, char * name)
{
  if (cvsfs_execute (sock, "Argument -N") < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_command_sequence_co - 'Argument -N' failed !\n");

    return -1;
  }

  if (cvsfs_execute_command (sock, "Argument ", dir, "/", name, NULL) < 0)
    return -1;

  if (cvsfs_execute (sock, "Directory .") < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_command_sequence_co - 'Directory .' failed !\n");

    return -1;
  }

  if (cvsfs_execute (sock, info->mnt.root) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_command_sequence_co - '%s' failed !\n", info->mnt.root);

    return -1;
  }

  if (cvsfs_execute (sock, "co") < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_command_sequence_co - 'co' failed !\n");

    return -1;
  }
  
  return 0;
}



int cvsfs_command_sequence_rdiff (struct socket * sock, struct cvsfs_sb_info * info, char * basedir)
{
//  if (cvsfs_execute (info, "UseUnchanged") < 0)
//  {
//    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - 'UseUnchanged' failed !\n");

//    return -1;
//  }

  if (cvsfs_execute (sock, "Argument -s") < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_command_sequence_rdiff - 'Argument -s' failed !\n");

    return -1;
  }

  if (cvsfs_execute (sock, "Argument -r") < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_command_sequence_rdiff - 'Argument -r' failed !\n");

    return -1;
  }

  if (cvsfs_execute (sock, "Argument 0") < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_command_sequence_rdiff - 'Argument 0' failed !\n");

    return -1;
  }
  
  if (cvsfs_execute_command (sock, "Argument ", basedir, NULL) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_command_sequence_rdiff - 'Argument %s' failed !\n", basedir);

    return -1;
  }

  if (cvsfs_execute (sock, "rdiff") < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_command_sequence_rdiff - 'rdiff' failed !\n");

    return -1;
  }

  return 0;
}
