/***************************************************************************
                          main.c  -  description
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
#include <pwd.h>
#include <grp.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <linux/config.h>



void
program_version ()
{
  printf ("cvsmnt %s\n", VERSION);
}



void
help ()
{
  program_version ();
  printf ("\n");
  printf ("usage: cvsmnt mount-point [options]\n");
  printf ("  --server server\n");
  printf ("  -s server         cvs-pserver\n");
  printf ("  --root root\n");
  printf ("  -r root           cvs root directory\n");
  printf ("  --project project\n");
  printf ("  -m project        module name\n");
  printf ("  --user user\n");
  printf ("  -u user           user account at cvs server (anonymous if not given)\n");
  printf ("  --password password\n");
  printf ("  -p password       password at cvs server (default is <none>)\n");
  printf ("  --uid uid\n");
  printf ("  -i uid            user id of the remote files (default is current uid)\n");
  printf ("  --gid gid\n");
  printf ("  -g gid            group id of the remote files (default is current uid)\n");
  printf ("  --filemask mask\n");
  printf ("  -f mask           file mask of the remote files (default is current umask)\n");
  printf ("  --dirmask mask\n");
  printf ("  -d mask           file mask of the remote directories (default is current umask)\n");
  printf ("  --help\n");
  printf ("  -h                print this help screen\n");
  printf ("  --version         print the program version\n\n");
}



void
add_option (char **data, const char *option, const char *argument)
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



char *
evaluate_ip (const char *host)
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



char *
current_uid ()
{
  static char buffer[32];
  
  sprintf (buffer, "%lu", (unsigned long) getuid ());
  
  return buffer;
}



char *
current_gid ()
{
  static char buffer[32];
  
  sprintf (buffer, "%lu", (unsigned long) getgid ());
  
  return buffer;
}



char *
current_umask ()
{
  static char buffer[32];
  unsigned long mask;
  
  mask = umask (0);
  umask (mask);               // stupid - there is no getter-only for umask
  
  sprintf (buffer, "%lu", (S_IRUSR | S_IWUSR |
                           S_IRGRP | S_IWGRP |
			   S_IROTH | S_IWOTH) & (~mask));
  
  return buffer;
}



char *
current_dmask ()
{
  static char buffer[32];
  unsigned long mask;
  unsigned long attr;
  
  mask = umask (0);
  umask (mask);               // stupid - there is not getter-only for umask

  attr = (S_IRWXU | S_IRWXG | S_IRWXO) & (~mask);

  if ((attr & S_IRUSR) != 0)
    attr |= S_IXUSR;
  if ((attr & S_IRGRP) != 0)
    attr |= S_IXGRP;
  if ((attr & S_IROTH) != 0)
    attr |= S_IXOTH;
  
  sprintf (buffer, "%lu", (unsigned long) 493 /* getumask () */);
  
  return buffer;
}



char *
convert_uid (char * uid)
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



char *
convert_gid (char * gid)
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



char *
convert_umask (char * umask)
{
  static char buffer[32];
  unsigned long mask;
  
  mask = strtoul (umask, NULL, 8);
  
  sprintf (buffer, "%lu", (S_IRWXU | S_IRWXG | S_IRWXO) & mask);
  
  return buffer;
}



int
parse_args (int argc, char *argv[], char **mount_point,
            char **data, char **share, int id_change_allowed)
{
  static struct option options[] = {
                                     { "server",   required_argument, 0, 's'},
                                     { "root",     required_argument, 0, 'r'},
				     { "project",  required_argument, 0, 'm'},
				     { "user",     required_argument, 0, 'u'},
				     { "password", required_argument, 0, 'p'},
				     { "uid",      required_argument, 0, 'i'},
				     { "gid",      required_argument, 0, 'g'},
				     { "filemask", required_argument, 0, 'f'},
				     { "dirmask",  required_argument, 0, 'd'},
				     { "help",     no_argument,	      0, 'h'},
				     { "version",  no_argument,	      0, 'v'},
				     { 0,          no_argument,       0, 0 }
				   };
  int opt;
  int index = 0;
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

  while ((opt = getopt_long (argc, argv,
                             "-s:r:m:u:p:i:g:f:d:h",
			     options, &index)) != -1)
  {
    switch (opt)
    {
      case 1:
        if (*mount_point != NULL)
	{
          fprintf (stderr, "mount point ('%s') is doubly defined\n", optarg);
	  
	  return -1;
	}
	
	*mount_point = fullpath (optarg);
        add_option (data, "mount", *mount_point);
        break;
	
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
        if (id_change_allowed == 0)
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
	else
          fprintf (stderr, "ownership modification (user id) is only allowed as root\n");
        break;
	
      case 'g':
        if (id_change_allowed == 0)
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
	else
          fprintf (stderr, "ownership modification (group id) is only allowed as root\n");
        break;
	
      case 'f':
        add_option (data, "fmask", convert_umask (optarg));
	fmask_found = 1;
        break;
	
      case 'd':
        add_option (data, "dmask", convert_umask (optarg));
	dmask_found = 1;
        break;
	
      case 'h':
	return -1;
	
      case 'v':
        program_version ();
	exit (0);

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



void
add_cache_option (char **data)
{
  struct passwd *uidentry;
  char *configdir;
    
  if ((uidentry = getpwuid (getuid ())) != NULL)
  {
    configdir = (char *) malloc (strlen (uidentry->pw_dir) + 9);
    
    strcpy (configdir, uidentry->pw_dir);
    strcat (configdir, "/.cvsfs/");
  }
  else
  {
    char *dir;
    
    dir = tempnam (NULL, NULL);
    configdir = (char *) malloc (strlen (dir) + 2);
    
    strcpy (configdir, uidentry->pw_dir);
    strcat (configdir, "/");
    
    free (dir); 
  }
  
  add_option (data, "cache", configdir);
    
  free (configdir);
}


#ifndef CONFIG_DEVFS_FS
int
allocate_devfs_node (char * mount_point)
{
  FILE *procfs;
  char *buffer;

  // first read /proc/cvsfs/<mount point>/device to get the major/minor devnbr
  buffer = malloc (strlen (mount_point) + 20);
  strcpy (buffer, "/proc/cvsfs");
  strcat (buffer, mount_point);
  strcat (buffer, "/device");
  
  if ((procfs = fopen (buffer, "r")) != NULL)
  {
    char line[64];
    char *next;
    int major;
    int minor;

    // obtain the major/minor device number assigned in the driver    
    fgets (line, sizeof (line), procfs);
    fclose (procfs);
    
    major = strtol (line, &next, 10);
    ++next;
    minor = strtol (next, NULL, 10);

    if ((major != -1) && (minor != -1))
    {
      // create the directory in /dev
      mkdir ("/dev/cvsfs", S_IRUSR | S_IRGRP | S_IROTH |
                           S_IXUSR | S_IXGRP | S_IXOTH | S_IWUSR);

      // create the device node
      sprintf (line, "/dev/cvsfs/cvsfs%i", minor);	 
      unlink (line);				// remove node if existing
      if (mknod (line, S_IFCHR | S_IRUSR | S_IWUSR, makedev (major, minor)))
        fprintf (stderr, "cvsmnt: can not allocate device node '%s' - error %s\n",
                 line, strerror (errno));
    }
  }
  else
    fprintf (stderr, "cvsmnt: can not open procfs file '%s'\n", buffer);

  free (buffer);
}
#endif


int
main(int argc, char *argv[])
{
  char *data;
  char *share;
  unsigned int flags;
  struct mntent ment;
  int fd;
  FILE *mtab;
  char *mount_point;
  int ownership_setting_allowed;
  char *cache_dir;
  
  if ((argc < 2) /* || (argv[1][0] == '-') */ )
  {
    help ();
    exit (1);
  }
  
  if (geteuid () != 0)
  {
    fprintf (stderr, "cvsmnt must be installed suid root for direct user mounts\n");
    exit (1);
  }

  if (getuid () != 0)
    ownership_setting_allowed = 1;
  else
    ownership_setting_allowed = 0;
  
  mount_point = NULL;
  if (parse_args (argc, argv, &mount_point,
                  &data, &share, ownership_setting_allowed) != 0)
  {
    help ();
    return -1;
  }

  add_cache_option (&data);

  add_option (&data, "mount_user", current_uid ());
  add_option (&data, "mount_group", current_gid ());
  
  flags = 0xC0ED0000; /* MS_MGC_VAL */
  
  if (mount (share, mount_point, "cvsfs", flags, (char *) data) < 0)
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
  free (data);

#ifndef CONFIG_DEVFS_FS
  /* allocate the node file in /dev if not already allocated */
  allocate_devfs_node (mount_point);
#endif
  
  /* cleanup allocated space */
  free (mount_point);

  return EXIT_SUCCESS;
}
