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

#include "cvsfs_config.h"
#include "proc.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/ctype.h>

#include <net/ip.h>

#include "inode.h"
#include "cache.h"
#include "socket.h"
#include "commands.h"



const char *revision_delimiter = "@@";



static unsigned char shifts[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  114,120, 53, 79, 96,109, 72,108, 70, 64, 76, 67,116, 74, 68, 87,
  111, 52, 75,119, 49, 34, 82, 81, 95, 65,112, 86,118,110,122,105,
   41, 57, 83, 43, 46,102, 40, 89, 38,103, 45, 50, 42,123, 91, 35,
  125, 55, 54, 66,124,126, 59, 47, 92, 71,115, 78, 88,107,106, 56,
   36,121,117,104,101,100, 69, 73, 99, 63, 94, 93, 39, 37, 61, 48,
   58,113, 32, 90, 44, 98, 60, 51, 33, 97, 62, 77, 84, 80, 85,223,
  225,216,187,166,229,189,222,188,141,249,148,200,184,136,248,190,
  199,170,181,204,138,232,218,183,255,234,220,247,213,203,226,193,
  174,172,228,252,217,201,131,230,197,211,145,238,161,179,160,212,
  207,221,254,173,202,146,224,151,140,196,205,130,135,133,143,246,
  192,159,244,239,185,168,215,144,139,165,180,157,147,186,214,176,
  227,231,219,169,175,156,206,198,129,164,150,210,154,177,134,127,
  182,128,158,208,162,132,167,209,149,241,153,251,237,236,171,195,
  243,233,253,240,194,250,191,155,142,137,245,235,163,242,178,152
};



/* scambles a password according cvs pserver rules */
void
cvsfs_password_scramble (char * data, char * result)
{
  char *src = data;
  char *dest = result;

  *dest = 'A';

  for (++dest; *src != '\0'; ++dest, ++src)
    *dest = shifts[(unsigned char) (*src)];

  *dest = '\0';
}



void
cvsfs_init_root_dirent (struct cvsfs_sb_info * server, struct cvsfs_fattr * fattr)
{
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: cvsfs_init_root_dirent\n");
#endif
  memset (fattr, 0, sizeof (struct cvsfs_fattr));

  fattr->f_nlink = 1;
  fattr->f_uid = server->mnt.uid;
  fattr->f_gid = server->mnt.gid;
  fattr->f_blksize = 512;
  fattr->f_ino = 2;
  fattr->f_mtime = CURRENT_TIME;
  fattr->f_mode = server->mnt.dir_mode;
  fattr->f_size = 512;
  fattr->f_blocks = 0;
}



/* converts a ipv4 IP address to an unsigned long */
unsigned long
cvsfs_inet_addr (char * ip)
{
  unsigned long res = 0;
  int i;
  int no = 0;
  int np = 0;

  for (i = 0; i < strlen (ip); ++i)
  {
    if (((ip[i] < '0') || (ip[i] > '9')) && (ip[i] != '.'))
      return -1;

    if (ip[i] == '.')
    {
      if (++np > 3)
        return -1;

      if ((no < 0) || (no > 255))
        return -1;

      res = (res >> 8) + (no << 24);
      no = 0;
    }
    else
      no = no * 10 + ip[i] - '0';
  }

  if ((no < 0) || (no > 255))
    return -1;

  res = (res >> 8) + (no << 24);

  if (np != 3)
    return -1;

  return res;
}



/* handles the options passed from the mount syscall */
int
cvsfs_parse_options (struct cvsfs_sb_info * info, void * opts)
{
  char *p;
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: cvsfs_parse_options '%s'\n", (char *) opts);
#endif
  info->address.sin_addr.s_addr = cvsfs_inet_addr ("127.0.0.1");
  info->address.sin_port = htons (2401);
  strcpy (info->user, "anonymous");
  strcpy (info->pass, "A");
  strcpy (info->mnt.root, "/cvsroot");
  *(info->mnt.project) = '\0';

  if (!opts)
    return -1;
    
  for (p = strtok (opts, ","); p; p = strtok (NULL, ","))
  {
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - key = '%s'\n", p);
#endif
    if (strncmp (p, "server=", 7) == 0)
    {
      info->address.sin_addr.s_addr = cvsfs_inet_addr (&p[7]);
      sprintf (info->mnt.server, "%u.%u.%u.%u",
               info->address.sin_addr.s_addr &0xff,
               (info->address.sin_addr.s_addr >> 8) &0xff,
	       (info->address.sin_addr.s_addr >> 16) &0xff,
	       (info->address.sin_addr.s_addr >> 24) &0xff);
    }
    else
      if (strncmp (p, "module=", 7) == 0)
        strncpy (info->mnt.project, &p[7], sizeof (info->mnt.project) - 1);
      else
        if (strncmp (p, "user=", 5) == 0)
          strncpy (info->user, &p[5], sizeof (info->user) - 1);
        else
          if (strncmp (p, "password=", 9) == 0)
	  {
	    char password[CVSFS_MAX_PASS + 1];
	    
            strncpy (password, &p[9], sizeof (info->pass) - 1);
            cvsfs_password_scramble (password, info->pass);
	  }
          else
            if (strncmp (p, "cvsroot=", 8) == 0)
              strncpy (info->mnt.root, &p[8], sizeof (info->mnt.root) - 1);
            else
	      if (strncmp (p, "mount=", 6) == 0)
	      {
	        strncpy (info->mnt.mountpoint, &p[6], sizeof (info->mnt.mountpoint));
#ifdef __DEBUG__
	        printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - mounting at %s\n", info->mnt.mountpoint);
#endif
	      }
	      else
                if (strncmp (p, "uid=", 4) == 0)
	        {
	          info->mnt.uid = simple_strtoul (&p[4], NULL, 0);
#ifdef __DEBUG__
	          printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - uid = %d\n", info->mnt.uid);
#endif
  	        }
	        else
	          if (strncmp (p, "gid=", 4) == 0)
	          {
	            info->mnt.gid = simple_strtoul (&p[4], NULL, 0);
#ifdef __DEBUG__
	            printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - gid = %d\n", info->mnt.gid);
#endif
	          }
	          else
	            if (strncmp (p, "fmask=", 6) == 0)
	            {
	              info->mnt.file_mode = simple_strtoul (&p[6], NULL, 0) | S_IFREG;
#ifdef __DEBUG__
	              printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - fmask = %d\n", info->mnt.file_mode);
#endif
	            }
	            else
	              if (strncmp (p, "dmask=", 6) == 0)
	              {
	                info->mnt.dir_mode = simple_strtoul (&p[6], NULL, 0) | S_IFDIR;
#ifdef __DEBUG__
	                printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - dmask = %d\n", info->mnt.dir_mode);
#endif
	              }
	              else
	                if (strncmp (p, "cache=", 6) == 0)
	                {
	                  strncpy (info->cachedir, &p[6], sizeof (info->cachedir));
#ifdef __DEBUG__
	                  printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - cache dir = %s\n", info->cachedir);
#endif
	                }
	                else
                          if (strncmp (p, "mount_user=", 11) == 0)
	    		  {
	    		    info->mount_uid = simple_strtoul (&p[11], NULL, 0);
#ifdef __DEBUG__
		            printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - mount_uid = %d\n", info->mount_uid);
#endif
  			  }
	    		  else
            		    if (strncmp (p, "mount_group=", 12) == 0)
	    		    {
	    		      info->mount_gid = simple_strtoul (&p[12], NULL, 0);
#ifdef __DEBUG__
	    		      printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - mount_gid = %d\n", info->mount_gid);
#endif
  	    		    }
	    		    else
	            	      printk (KERN_ERR "Invalid option '%s' passed\n", p);
  }

  return 0;
}



/* analyzes one rdiff result line and pick the relavant values */
int
cvsfs_get_fname (char * s, char * d, char * ver)
{
  static char *prefix = "M File ";
  static char *revision = " is new; current revision ";
  static char *changed1 = " changed from revision ";
  static char *changed2 = " to ";
  static char *dir = "E cvs server: Diffing ";
  int ret = -1;

  if (strncmp (s, prefix, strlen (prefix)) == 0)
  {
    char *name;
    char *ptr;
    
    name = &(s[strlen (prefix)]);
#ifdef __DEBUG__
    printk (KERN_DEBUG " -- is a file: (%s)\n", name);
#endif
    if ((ptr = strstr (name, revision)) == NULL)
    {
      if ((ptr = strstr (name, changed1)) != NULL)
      {
        char *version;
#ifdef __DEBUG__
        printk (KERN_DEBUG " -- changed version '%s'\n", name);
#endif
	version = &(ptr[strlen (changed1)]);
	
	if ((version = strstr (version, changed2)) != NULL)
	{
	  strcpy (ver, &(version[strlen (changed2)]));
#ifdef __DEBUG__
          printk (KERN_DEBUG " -- version '%s'\n", ver);
#endif
	}
	else
	  ptr = NULL;
      }
    }
    else
    {
      strcpy (ver, &(ptr[strlen (revision)]));
#ifdef __DEBUG__
      printk (KERN_DEBUG " -- version '%s'\n", ver);
#endif
    }
      
    if (ptr != NULL)
    {
      *ptr = '\0';
      strcpy (d, name);
      ret = 1;
    }
  }
  else
    if (strncmp (s, dir, strlen (dir)) == 0)
    {
      strcpy (d, &(s[strlen (dir)]));
      *ver = '\0';

      ret = 0;
    }

  return ret;
}



/* converts a cleartext month name (short) to a number (1..12) */
int
cvsfs_convert_month (char *ptr, char **newptr)
{
  static char *monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  int loop;
  
  *newptr = &ptr[3];

  for (loop = 12; loop > 0; loop--)
    if (strncmp (ptr, monthNames[loop - 1], 3) == 0)
      return loop;
  
  printk (KERN_ERR "cvsfs: cvsfs_convert_month - invalid month name %s - using 'January'\n", ptr);
  
  return 1;  // no valid month string - return dummy: January
}



/* converts a tinestamp to a unix time */
unsigned long
cvsfs_convert_time (char *ptr, char **newptr)
{
  int year;
  int month;
  int day;
  int hour;
  int minutes;
  int seconds;
  
  day = simple_strtoul (ptr, &ptr, 0);
  ++ptr;
  month = cvsfs_convert_month (ptr, &ptr);
  ++ptr;
  year = simple_strtoul (ptr, &ptr, 0);
  ++ptr;
  if (*ptr == '0')
    ++ptr;
  hour = simple_strtoul (ptr, &ptr, 0);
  ++ptr;
  if (*ptr == '0')
    ++ptr;
  minutes = simple_strtoul (ptr, &ptr, 0);
  ++ptr;
  if (*ptr == '0')
    ++ptr;
  seconds = simple_strtoul (ptr, newptr, 0);

  return mktime (year, month, day, hour, minutes, seconds);
}



/* converts a single 'rwx' block to the appropriate bit settings */
umode_t
cvsfs_convert_attr_single (char *ptr, umode_t actual, umode_t reset,
			   umode_t read, umode_t write, umode_t execute)
{
  char *loop;
  
  actual &= ~reset;

  for (loop = ptr; (*loop != '\0') &&
                   ((*loop == 'r') || (*loop == 'w') || (*loop == 'x')); ++loop)
  {
    switch (*loop)
    {
      case 'r': actual |= read;		break;
      case 'w': actual |= write;	break;
      case 'x': actual |= execute;	break;
    }
  }
  
  return actual;
}



/* converts the attribute information to the appropriate bit settings */
umode_t
cvsfs_convert_attr (char *ptr, umode_t actual, char **newptr)
{
  if (strncmp (ptr, "u=", 2) == 0)
    actual = cvsfs_convert_attr_single (&(ptr[2]), actual,
                                        S_IRWXU, S_IRUSR, S_IWUSR, S_IXUSR);
  
  if (strncmp (ptr, "g=", 2) == 0)
    actual = cvsfs_convert_attr_single (&(ptr[2]), actual,
                                        S_IRWXG, S_IRGRP, S_IWGRP, S_IXGRP);
  
  if (strncmp (ptr, "o=", 2) == 0)
    actual = cvsfs_convert_attr_single (&(ptr[2]), actual,
                                        S_IRWXO, S_IROTH, S_IWOTH, S_IXOTH);

  *newptr = strchr (ptr, ',');

  return actual;
}



/* obtains the attributes (size, date, access rights) from cvs server */
int
cvsfs_get_fattr (struct cvsfs_sb_info * info,
                 char * dir, struct cvsfs_dir_entry * entry)
{
  struct socket *sock;
  char line[CVSFS_MAX_LINE];
  char *ptr;
  int i;

  if ((entry->mode & S_IFDIR) != 0)
    return 0;				// is directory - cvs does not version it
  
  strcpy (line, info->mnt.project);
  strcat (line, dir);
  i = strlen (line);
  if (line[i - 1] == '/')
    line[i - 1] = '\0';         /* purge trailing slashes */

  sock = NULL;
#ifdef __DEBUG__  
  if (entry->version == NULL)
    printk (KERN_DEBUG "cvsfs: cvsfs_get_fattr '%s - %s'\n", line, entry->name);
  else
    printk (KERN_DEBUG "cvsfs: cvsfs_get_fattr '%s - %s (version %s)'\n", line, entry->name, entry->version);
#endif  
  if (cvsfs_connect (&sock, info->user, info->pass,
                     info->mnt.root, info->address, 0) < 0)
  {
    printk (KERN_ERR "cvsfs: cvsfs_get_fattr - connect failed !\n");

    return -1;
  }

  if (cvsfs_command_sequence_co (sock, info, line,
                                 entry->name, entry->version) < 0)
  {
    cvsfs_disconnect (&sock);

    return -1;
  }

  i = cvsfs_long_readline (sock, line, CVSFS_MAX_LINE);
  if (i < 0)
  {
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: cvsfs_get_fattr - line read : %s\n", line);
#endif

    cvsfs_disconnect (&sock);

    return -1;
  }
  
  while (strcmp (line, "ok") != 0)
  {
    if (strncmp (line, "error", 5) == 0)
    {
      printk (KERN_ERR "cvsfs: cvsfs_get_fattr - error while reading file\n");

      cvsfs_disconnect (&sock);

      return -1;
    }

    if (i > 0)
    {
      if (strncmp (line, "Mod-time ", 9) == 0)
      {
        entry->date = cvsfs_convert_time (&line[9], &ptr);
      }

      if (strncmp (line, "u=", 2) == 0)
      {
#ifdef __DEBUG__
        printk (KERN_DEBUG "cvsfs: cvsfs_get_fattr - attribs : %s\n", line);
#endif
        entry->mode = cvsfs_convert_attr (line, entry->mode, &ptr);
	if (ptr != NULL)
	{
	  ++ptr;
          entry->mode = cvsfs_convert_attr (ptr, entry->mode, &ptr);
	}
	if (ptr != NULL)
	{
	  ++ptr;
          entry->mode = cvsfs_convert_attr (ptr, entry->mode, &ptr);
	}
      }
      
      if (isdigit (*line) != 0)
      {
        entry->size = simple_strtoul (line, &ptr, 0);
        entry->blocks = (entry->size + 1023) >> 9;
	
	break;           // exit the reading -- kill connection
      }
    }
    
    i = cvsfs_long_readline (sock, line, CVSFS_MAX_LINE);
    if (i <= 0)
    {
      printk (KERN_ERR "cvsfs: cvsfs_get_fattr - no response !\n");

      cvsfs_disconnect (&sock);

      return -1;
    }
  }
  
  cvsfs_disconnect (&sock);

  return 0;
}



/* loads a complete directory info from cvs server */
int
cvsfs_loaddir (struct cvsfs_sb_info * info, char * name,
               struct cvsfs_directory * dir, char * ver)
{
  char res[CVSFS_MAX_LINE];
  char line[CVSFS_MAX_LINE];
  char basedir[CVSFS_MAX_LINE];
  char version[CVSFS_MAX_LINE];
  char *ptr;
  int len;
  int i;
  struct socket *sock;

  sock = NULL;
#ifdef __DEBUG__
  if (ver == NULL)
    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - directory '%s'\n", name);
  else
    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - directory '%s' (version %s)\n", name, ver);
#endif
  strcpy (basedir, info->mnt.project);
  strcat (basedir, name);
  len = strlen (basedir);
  if (basedir[len - 1] == '/')
  {
    basedir[len - 1] = '\0';         /* purge trailing slashes */
    --len;
  }

  if (cvsfs_connect (&sock, info->user, info->pass,
                     info->mnt.root, info->address, 0) < 0)
  {
    printk (KERN_ERR "cvsfs: cvsfs_loaddir - connect failed !\n");

    return -1;
  }

  if (cvsfs_command_sequence_rdiff (sock, info, basedir, ver) < 0)
  {
    printk (KERN_ERR "cvsfs: cvsfs_loaddir - rdiff command failed !\n");

    cvsfs_disconnect (&sock);

    return -1;
  }

  i = cvsfs_long_readline (sock, line, CVSFS_MAX_LINE);
  if (i < 0)
  {
    printk (KERN_ERR "cvsfs: cvsfs_loaddir - no response !\n");

    cvsfs_disconnect (&sock);

    return -1;
  }

  while (strcmp (line, "ok") != 0)
  {
    if (strncmp (line, "error", 5) == 0)
    {
      printk (KERN_ERR "cvsfs: cvsfs_loaddir - error while reading directory\n");

      cvsfs_disconnect (&sock);

      return -1;
    }

    i = cvsfs_get_fname (line, res, version);
    if (i != -1)
    {
      if ((strncmp (res, basedir, len) == 0) && (res[len] == '/'))
        ptr = &res[len + 1];
      else
        ptr = strchr (res, '\0');
      
      if (strchr (ptr, '/') != NULL)
        i = -1;

      if ((i != -1) && (*ptr != '\0'))
      {
        switch (i)
        {
          case 0:			// is a directory
#ifdef __DEBUG__
            printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - add subdirectory '%s'\n", ptr);
#endif
            cvsfs_cache_add_file (dir, ptr, version, info->mnt.dir_mode);
	    
	    break;
	    
          case 1:			// is a file
#ifdef __DEBUG__
	    if (*version != '\0')
              printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - add file '%s' - version %s\n", ptr, version);
	    else
              printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - add file '%s'\n", ptr);
#endif
            cvsfs_cache_add_file (dir, ptr, version, info->mnt.file_mode);
	    
	    break;
        }
      }
    }
    
    i = cvsfs_long_readline (sock, line, CVSFS_MAX_LINE);
    if (i <= 0)
    {
      printk (KERN_ERR "cvsfs: cvsfs_loaddir - no response !\n");

      cvsfs_disconnect (&sock);

      return -1;
    }
  }

  cvsfs_disconnect (&sock);

  return 0;
}



/* evaluate full path starting with given dentry */
int
cvsfs_get_name (struct dentry * d, char * name)
{
  int len = 0;
  struct dentry *p;

  for (p = d; p != p->d_parent; p = p->d_parent)
    len += p->d_name.len + 1;

  if (len > CVSFS_MAXPATHLEN)
    return -1;

  if (len == 0)
  {
    name[0] = '/';
    name[1] = '\0';

    return 0;
  }

  name[len] = '\0';
  for (p = d; p != p->d_parent; p = p->d_parent)
  {
    len -= p->d_name.len;
    strncpy (&(name[len]), p->d_name.name, p->d_name.len);
    --len;
    name[len] = '/';
  }

  return 0;
}



inline void
cvsfs_lock (struct cvsfs_sb_info * info)
{
  down (&(info->sem));
}



inline void
cvsfs_unlock (struct cvsfs_sb_info * info)
{
  up (&(info->sem));
}



int
cvsfs_get_attr (struct dentry * dentry,
                struct cvsfs_fattr * fattr, struct cvsfs_sb_info * info)
{
  struct cvsfs_directory *dir;
  char buf[CVSFS_MAX_LINE];
  struct cvsfs_dir_entry *file;
  char name[CVSFS_MAX_LINE];
  char *version;
  int size;

  cvsfs_get_name (dentry->d_parent, buf);	// parent directory

  size = dentry->d_name.len < (sizeof (name) - 1) ? dentry->d_name.len
                                                  : (sizeof (name) - 1);
  strncpy (name, dentry->d_name.name, size);
  name[size] = '\0';

  if ((version = strstr (name, revision_delimiter)) != NULL)
  {
    *version = '\0';
    ++version;
    ++version;
  }
  
  // Actual:
  // name    = file name
  // version = version number (if any)
  
#ifdef __DEBUG__
  if (version == NULL)
    printk (KERN_DEBUG "cvsfs: cvsfs_get_attr - file %s\n", name);
  else
    printk (KERN_DEBUG "cvsfs: cvsfs_get_attr - file %s (version %s)\n", name, version);
#endif
  dir = cvsfs_cache_get_dir (info, buf, version);

  if (!dir)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_get_attr - cvsfs_cache_get_dir failed !\n");

    return -1;
  }
#ifdef __DEBUG__
  if (version == NULL)
    printk (KERN_DEBUG "cvsfs: cvsfs_get_attr - search file %s\n", name);
  else
    printk (KERN_DEBUG "cvsfs: cvsfs_get_attr - search file %s (version %s)\n", name, version);
#endif

  file = cvsfs_cache_get_file (info, dir, name, version);

  if (!file)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_get_attr - file not found in parent dir cache !\n");

    return -1;
  }

  fattr->f_mode = file->mode;
  fattr->f_size = file->size;
  fattr->f_blksize = file->blocksize;
  fattr->f_blocks = file->blocks;
  fattr->f_nlink = file->nlink;
  fattr->f_mtime = file->date;     // was: CURRENT_TIME;
  fattr->f_uid = info->mnt.uid;
  fattr->f_gid = info->mnt.gid;
  fattr->f_info.version = NULL;
  if (file->version != NULL)
  {
    fattr->f_info.version = kmalloc (strlen (file->version) + 1, GFP_KERNEL);
    if (fattr->f_info.version == NULL)
    {
      printk (KERN_ERR "cvsfs: cvsfs_get_attr - memory squeeze !\n");
      
      return -1;
    }
    strcpy (fattr->f_info.version, file->version);
  }
#ifdef __DEBUG__
  if (version == NULL)
    printk (KERN_DEBUG "cvsfs: cvsfs_get_attr - file %s found, attrib = %04o\n", name, fattr->f_mode);
  else
    printk (KERN_DEBUG "cvsfs: cvsfs_get_attr - file %s (version %s), attrib = %04o\n", name, version, fattr->f_mode);
#endif
  return 0;
}



/* read the file contents of a file - called by the kernel */
int
cvsfs_read (struct dentry * dentry, unsigned long offset,
            unsigned long count, char * buffer)
{
  struct inode *inode = dentry->d_inode;
  struct super_block *sb = inode->i_sb;
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) sb->u.generic_sbp;
  struct socket *sock;
  char line[CVSFS_MAX_LINE];
  char *version;
  int res = 0;
  int i;
  
  cvsfs_lock (info);

  cvsfs_get_name (dentry, line);

  if ((version = strstr (line, revision_delimiter)) != NULL) // strip version info
    *version = '\0';
  
  sock = NULL;

  if ((inode->u.generic_ip != NULL) &&
      (((struct cvsfs_versioninfo *) (inode->u.generic_ip))->version != NULL))
  {
    version = ((struct cvsfs_versioninfo *) (inode->u.generic_ip))->version;
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: cvsfs_read '%s %s version %s'\n", info->mnt.project, line, version);
#endif
  }
  else
  {
    version = NULL;
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: cvsfs_read '%s %s'\n", info->mnt.project, line);
#endif
  }
  
  if (cvsfs_connect (&sock, info->user, info->pass,
                     info->mnt.root, info->address, 0) < 0)
  {
    printk (KERN_ERR "cvsfs: cvsfs_read - connect failed !\n");

    return 0;
  }

  if (cvsfs_command_sequence_co (sock, info,
                                 info->mnt.project, &line[1], version) < 0)
  {
    cvsfs_disconnect (&sock);

    return 0;
  }

  i = cvsfs_long_readline (sock, line, CVSFS_MAX_LINE);
  if (i < 0)
  {
    printk (KERN_ERR "cvsfs: cvsfs_read - read timeout\n");

    cvsfs_disconnect (&sock);

    return 0;
  }
  
  while (strcmp (line, "ok") != 0)
  {
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: cvsfs_read - handle string '%s'\n", line);
#endif
    if (strncmp (line, "error", 5) == 0)
    {
      printk (KERN_ERR "cvsfs: cvsfs_read - 'error' returned from server\n");

      cvsfs_disconnect (&sock);

      return 0;
    }

    if (i > 0)
    {
      if (isdigit (*line) != 0)
      {
        unsigned long size = simple_strtoul (line, NULL, 0);

        if (size < offset)
	  return 0;
	  
        if (cvsfs_read_raw_data (sock, offset, NULL) < 0)  // skip bytes
        {
          printk (KERN_ERR "cvsfs: cvsfs_read - low read !\n");

          cvsfs_disconnect (&sock);

          return 0;
        }
	
	size -= offset;
	  
        if ((i = cvsfs_read_raw_data (sock, size < count ? size
	                                                 : count, buffer)) >= 0)
	  res = i;

	break;           // exit the reading -- kill connection
      }
    }
    
    i = cvsfs_long_readline (sock, line, CVSFS_MAX_LINE);
    if (i <= 0)
    {
      printk (KERN_ERR "cvsfs: cvsfs_read - no response !\n");

      cvsfs_disconnect (&sock);

      return 0;
    }
  }
  
  cvsfs_disconnect (&sock);

  cvsfs_unlock (info);

  return res;
}
