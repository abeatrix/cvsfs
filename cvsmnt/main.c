/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Mit Jun 14 18:32:49 CEST 2001
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
#include <ctype.h>
#include <errno.h>
#include <mntent.h>
#include <fcntl.h>
#include <sys/param.h>           /* defines MAXPATHLEN */
#include <netdb.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <arpa/inet.h>
#include <sys/mount.h>
#include <sys/stat.h>



void help ()
{
  printf ("\n");
  printf ("usage: cvsmnt mount-point [options]\n");
  printf ("  -s server    cvs-pserver\n");
  printf ("  -r root      cvs root directory\n");
  printf ("  -m project   module name\n");
  printf ("  -u user      user account at cvs server (anonymous if not given)\n");
  printf ("  -p password  password at cvs server (default is <none>)\n");
  printf ("  -i uid       user id of the remote files (default is current uid)\n");
  printf ("  -g gid       group id of the remote files (default is current uid)\n");
  printf ("  -f mask      file mask of the remote files (default is current umask)\n");
  printf ("  -d mask      file mask of the remote directories (default is current umask)\n");
  printf ("  -h           print this help screen\n");
}



void add_option (char **data, const char *option, const char *argument)
{
  if (**data != '\0')
  {
    *data = realloc (*data, strlen (*data) + strlen (option) + strlen (argument) + 3);
    strcat (*data, ",");
  }
  else
    *data = realloc (*data, strlen (option) + strlen (argument) + 2);
    
  strcat (*data, option);
  strcat (*data, "=");
  strcat (*data, argument);
}



char *evaluate_ip (const char *host)
{
  struct hostent *hptr;
  char ip_name[32];

  hptr = gethostbyname2 (optarg, AF_INET);
  if (hptr == NULL)
  {
    fprintf (stderr, "server '%s' is not resolvable. Error %s\n",
             optarg, hstrerror (h_errno));
    return NULL;
  }

  *ip_name = '\0';
  inet_ntop (hptr->h_addrtype, *(hptr->h_addr_list), ip_name, sizeof (ip_name));

  return strdup (ip_name);
}



char *current_uid ()
{
  static char buffer[32];
  
  sprintf (buffer, "%lu", (unsigned long) getuid ());
  
  return buffer;
}



char *current_gid ()
{
  static char buffer[32];
  
  sprintf (buffer, "%lu", (unsigned long) getgid ());
  
  return buffer;
}



char *current_umask ()
{
  static char buffer[32];
  
  sprintf (buffer, "%lu", (unsigned long) 420 /* getumask () */);
  
  return buffer;
}



char *current_dmask ()
{
  static char buffer[32];
  
  sprintf (buffer, "%lu", (unsigned long) 493 /* getumask () */);
  
  return buffer;
}



char *convert_uid (char * uid)
{
  static char buffer[32];

  if (isdigit (*uid) != 0)
  {
    sprintf (buffer, "%lu", strtoul (uid, NULL, 0));
  }
  else
  {
    struct passwd *uidentry;
    
    if ((uidentry = getpwnam (uid)) != NULL)    
      sprintf (buffer, "%lu", (unsigned long) uidentry->pw_uid);
    else
      return NULL;
  }
  
  return buffer;
}



char *convert_gid (char * gid)
{
  static char buffer[32];

  if (isdigit (*gid) != 0)
  {
    sprintf (buffer, "%lu", strtoul (gid, NULL, 0));
  }
  else
  {
    struct group *gidentry;
    
    if ((gidentry = getgrnam (gid)) != NULL)
      sprintf (buffer, "%lu", (unsigned long) gidentry->gr_gid);
    else
      return NULL;
  }
  
  return buffer;
}



char *convert_umask (char * umask)
{
  return "420";
}



int parse_args (int argc, char *argv[], char **data, char **share)
{
  int opt;
  int server_found = 0;
  int module_found = 0;
  int uid_found = 0;
  int gid_found = 0;
  int fmask_found = 0;
  int dmask_found = 0;
  char *server;
  char *module;
  char *help;

  *data = malloc (1);
  **data = '\0';
  
  while ((opt = getopt (argc, argv, "s:r:m:u:p:i:g:f:d:")) != EOF)
  {
    switch (opt)
    {
      case 's':
	if (isdigit (*optarg) == 0)
	{
	  char *ip = evaluate_ip (optarg);
	  if (ip == NULL)
	    return -1;
	    
          add_option (data, "server", ip);
	  free (ip);
	}
	else
          add_option (data, "server", optarg);
	  
        server = strdup (optarg);
	
	server_found = 1;
        break;
	
      case 'r':
        add_option (data, "cvsroot", optarg);
	break;
	
      case 'm':
        add_option (data, "module", optarg);
	module = strdup (optarg);
	module_found = 1;
        break;
	
      case 'u':
        add_option (data, "user", optarg);
        break;
	
      case 'p':
        add_option (data, "password", optarg);
        break;
	
      case 'i':
        if ((help = convert_uid (optarg)) != NULL)
	{
          add_option (data, "uid", help);
	  uid_found = 1;
	}
	else
	{
          fprintf (stderr, "user id '%s' does not exist\n", optarg);
	  
	  return -1;
	}
        break;
	
      case 'g':
        if ((help = convert_gid (optarg)) != NULL)
	{
          add_option (data, "gid", help);
	  gid_found = 1;
	}
	else
	{
          fprintf (stderr, "group id '%s' does not exist\n", optarg);
	  
	  return -1;
	}
        break;
	
      case 'f':
        add_option (data, "fmask", convert_umask (optarg));
	fmask_found = 1;
        break;
	
      case 'd':
        add_option (data, "dmask", convert_umask (optarg));
	dmask_found = 1;
        break;
	
      default:
        return -1;
    }
  }
  
  if (server_found == 0)
  {
    fprintf (stderr, "No cvs pserver given\n");
    return -1;
  }
  
  if (module_found == 0)
  {
    fprintf (stderr, "No cvs module given\n");
    return -1;
  }

  if (uid_found == 0)
    add_option (data, "uid", current_uid ());

  if (gid_found == 0)
    add_option (data, "gid", current_gid ());

  if (fmask_found == 0)
    add_option (data, "fmask", current_umask ());

  if (dmask_found == 0)
    add_option (data, "dmask", current_dmask ());

  *share = malloc (strlen (server) + strlen (module) + 4);

  strcpy (*share, "//");
  strcat (*share, server);
  strcat (*share, "/");
  strcat (*share, module);

  free (server);
  free (module);
  
  return 0;
}



char *fullpath (const char *p)
{
  char path[MAXPATHLEN];
  
  if (strlen (p) > (MAXPATHLEN - 1))
  {
    fprintf (stderr, "Mount point is too long\n");
    exit (1);
  }
  
  if (realpath (p, path) == NULL)
  {
    fprintf (stderr, "Failed to find real path for mount point\n");
    exit (1);
  }
  
  return strdup (path);
}



int main(int argc, char *argv[])
{
  char *data;
  char *share;
  unsigned int flags;
  struct mntent ment;
  int fd;
  FILE *mtab;
  char *mount_point;
  
  if ((argc < 2) || (argv[1][0] == '-'))
  {
    help ();
    exit (1);
  }
  
  if (geteuid () != 0)
  {
    fprintf (stderr, "cvsmnt must be installed suid root for direct user mounts\n");
    exit (1);
  }
  
  mount_point = fullpath (argv[1]);
  
  ++argv;
  --argc;
  
  if (parse_args (argc, argv, &data, &share) != 0)
  {
    help ();
    return -1;
  }

  add_option (&data, "mount", mount_point);
  
  flags = 0xC0ED0000; /* MS_MGC_VAL */
  
  if (mount (share, argv[0], "cvsfs", flags, (char *) data) < 0)
  {
    switch (errno)
    {
      case ENODEV:
        fprintf (stderr, "ERROR: cvsfs filesystem not supported by kernel\n");
	break;
	
      default:
        perror ("mount error");
    }
    fprintf (stderr, "Please refer to the cvsmnt(8) manual page\n");
    return -1;
  }
  
  ment.mnt_fsname = share;
  ment.mnt_dir = mount_point;
  ment.mnt_type = "cvsfs";
  ment.mnt_opts = "";
  ment.mnt_freq = 0;
  ment.mnt_passno = 0;
  
  if ((fd = open (MOUNTED"~", O_RDWR | O_CREAT | O_EXCL, 0600)) == -1)
  {
    fprintf (stderr, "Can't get "MOUNTED"~ lock file\n");
    return 1;
  }
  close (fd);
  
  if ((mtab = setmntent (MOUNTED, "a+")) == NULL)
  {
    fprintf (stderr, "Can't open " MOUNTED);
    return 1;
  }
  
  if (addmntent (mtab, &ment) == -1)
  {
    fprintf (stderr, "Can't write mount entry\n");
    return 1;
  }
  
  if (fchmod (fileno (mtab), 0644) == -1)
  {
    fprintf (stderr, "Can't set perms on "MOUNTED);
    return 1;
  }
  endmntent (mtab);
  
  if (unlink (MOUNTED"~") == -1)
  {
    fprintf (stderr, "Can't remove "MOUNTED"~");
    return 1;
  }

  /* cleanup allocated space */
  free (mount_point);
  free (data);
  
  return EXIT_SUCCESS;
}
