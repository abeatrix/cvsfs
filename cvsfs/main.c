/***************************************************************************
                          main.c  -  entry point
                             -------------------
    begin                : Mon May 27 2002
    copyright            : (C) 2002 by Petric Frank
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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#include "procfs.h"
#include "devfs.h"
#include "superblock.h"

MODULE_AUTHOR ("Petric Frank <pfrank@gmx.de>");
MODULE_DESCRIPTION ("CVS virtual file system");


static DECLARE_FSTYPE (cvsfs_fs_type, "cvsfs", cvsfs_read_super, 0);



/* initialize the file systen driver */
static int
__init init_cvsfs_fs ()
{
  printk (KERN_DEBUG "cvsfs: init_cvs_vfs\n");

  cvsfs_procfs_init ();

  cvsfs_devfs_init ();

  return register_filesystem (&cvsfs_fs_type);
}



/* cleanup the file system driver before unloading */
static void
__exit exit_cvsfs_fs ()
{
  printk (KERN_DEBUG "cvsfs: exit_cvs_vfs\n");

  cvsfs_procfs_cleanup ();

  cvsfs_devfs_cleanup ();

  unregister_filesystem (&cvsfs_fs_type);
}



EXPORT_NO_SYMBOLS;

module_init (init_cvsfs_fs);
module_exit (exit_cvsfs_fs);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,10)
MODULE_LICENSE("GPL");
#endif
