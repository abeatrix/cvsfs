/***************************************************************************
                          main.c  -  description
                             -------------------
    begin                : Sun Nov 18 2001
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mntent.h>
#include <fcntl.h>
#include <sys/param.h>           /* defines MAXPATHLEN */
#include <sys/mount.h>
#include <sys/stat.h>



void
help ()
{
  printf ("\n");
  printf ("usage: cvsumount mount-point\n");
}



char *
fullpath (const char *p)
{
  char path[MAXPATHLEN];
  
  if (strlen (p) > (MAXPATHLEN - 1))
  {
    fprintf (stderr, "Path '%s' is too long\n", p);
    exit (1);
  }
  
  if (realpath (p, path) == NULL)
  {
    fprintf (stderr, "Failed to find real path for '%s'\n", p);
    exit (1);
  }
  
  return strdup (path);
}



int
main(int argc, char *argv[])
{
  struct mntent *ment;
  FILE *mtab;
  FILE *new_mtab;
  int fd;
  char *mount_point;
  
  if (argc != 2)
  {
    help ();
    exit (1);
  }
  
  if (geteuid () != 0)
  {
    fprintf (stderr, "cvsumount must be installed suid root for direct user unmounts\n");
    exit (1);
  }

  mount_point = fullpath (argv[1]);
  
  if (umount (mount_point) != 0)
  {
    fprintf (stderr, "Could not umount %s: %s\n", mount_point, strerror (errno));
    exit (1);
  }
  
  if ((fd = open (MOUNTED"~", O_RDWR | O_CREAT | O_EXCL, 0600)) == -1)
  {
    fprintf (stderr, "Can't get "MOUNTED"~ lock file\n");
    return 1;
  }
  close (fd);
  
  if ((mtab = setmntent (MOUNTED, "r")) == NULL)
  {
    fprintf (stderr, "Can't open " MOUNTED);
    return 1;
  }

#define MOUNTED_TMP MOUNTED".tmp"

  if ((new_mtab = setmntent (MOUNTED_TMP, "w")) == NULL)
  {
    fprintf (stderr, "Can't open %s: %s\n", MOUNTED_TMP, strerror (errno));
    endmntent (mtab);
    return 1;
  }
  
  while ((ment = getmntent (mtab)) != NULL)
  {
    if (strcmp (ment->mnt_dir, mount_point) != 0)
      addmntent (new_mtab, ment);
  }
  
  endmntent (mtab);

  if (fchmod (fileno (new_mtab), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0)
  {
    fprintf (stderr, "Error changing mode of %s: %s\n", MOUNTED_TMP, strerror (errno));
    return 1;
  }

  endmntent (new_mtab);
  
  if (rename (MOUNTED_TMP, MOUNTED) < 0)
  {
    fprintf (stderr, "Cannot rename %s to %s: %s\n", MOUNTED_TMP, MOUNTED, strerror (errno));
    return 1;
  }
  
  if (unlink (MOUNTED"~") == -1)
  {
    fprintf (stderr, "Can't remove "MOUNTED"~");
    return 1;
  }

  /* cleanup allocated space */
  free (mount_point);
  
  return EXIT_SUCCESS;
}
