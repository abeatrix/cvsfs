/***************************************************************************
                          superblock.c  -  description
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
#include "superblock.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>

#include "file.h"
#include "inode.h"
#include "util.h"



//#define __DEBUG__

#define CVSFS_SUPER_MAGIC	0xED79ED79
#define CVSFS_ROOT_INODE	1
#define CVSFS_MAXPATHLEN	1024


/* options transfered from mount call */
#define INVALID_OPTION		-1
#define OPTION_SERVER		0
#define OPTION_MODULE		1
#define	OPTION_USER		2
#define	OPTION_PASSWORD		3
#define	OPTION_CVSROOT		4
#define	OPTION_UID		5
#define	OPTION_GID		6
#define	OPTION_FMASK		7
#define	OPTION_DMASK		8
#define	OPTION_MOUNT_USER	9
#define	OPTION_MOUNT_GROUP	10
#define	OPTION_MOUNT_POINT	11
#define	LAST_OPTION		OPTION_MOUNT_POINT

typedef char *pchar;
static pchar cvsfs_option[LAST_OPTION + 2]	= { "server=",
                        			    "module=",
						    "user=",
    						    "password=",
                    				    "cvsroot=",
    	        				    "uid=",
		    				    "gid=",
	    					    "fmask=",
	            		    		    "dmask=",
            		    			    "mount_user=",
        					    "mount_group=",
				    		    "mount=",
						    NULL };



/* data to scramble the cleartext password */
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



/* list of super blocks of cvsfs */
struct cvsfs_sb_info	*cvsfs_root	= NULL;
rwlock_t		root_lock	= RW_LOCK_UNLOCKED;


/* forward references - super operations */
static void cvsfs_put_super (struct super_block *);
static int cvsfs_statfs (struct super_block *, struct statfs *);

struct super_operations cvsfs_sops = {
					put_inode:	force_delete,
					delete_inode:	cvsfs_delete_inode,
					put_super:	cvsfs_put_super,
					statfs:		cvsfs_statfs,
					clear_inode:	cvsfs_clear_inode,
				     };

/* forward refernces - local routines */
static int cvsfs_parse_options (struct cvsfs_sb_info *, void *);



/* lock and obtain managed list for read */
struct cvsfs_sb_info *
cvsfs_read_lock_superblock_list (void)
{
  read_lock (&root_lock);
  
  return cvsfs_root;
}



/* unlock managed list from read access */
void
cvsfs_read_unlock_superblock_list (void)
{
  read_unlock (&root_lock);
}



/* add the current superblock to the managed list */
static void
cvsfs_add_superblock (struct cvsfs_sb_info *sb)
{
  const int max_mounts = 32000;
  struct cvsfs_sb_info *scan;

  write_lock (&root_lock);
  
  /* search for a unique id for this mount operation */
#ifdef __DEBUG__
  printk (KERN_ERR "cvsfs add_superblock: scan for valid id\n");
#endif
  sb->id = 1;			/* id 0 is reserved for special use */
  scan = cvsfs_root;
  while (1 == 1)
  {
#ifdef __DEBUG__
    printk (KERN_ERR "cvsfs add_superblock: is id %d free ?\n", sb->id);
#endif
    for (scan = cvsfs_root; scan != NULL; scan = scan->next)
      if (scan->id == sb->id)
        break;
    
    if (scan == NULL)		/* free id found */
      break;

    if (sb->id > max_mounts)	/* number of ids exhausted */
      break;

    ++(sb->id);
  }
  if (scan != NULL)
#ifdef __DEBUG__
  {
    printk (KERN_ERR "cvsfs add_superblock: no id free !\n");
#endif
    sb->id = 0;			/* indication: no id assigned */
#ifdef __DEBUG__
  }
  else
    printk (KERN_ERR "cvsfs add_superblock: id %d assigned\n", sb->id);
#endif
        
  sb->next = cvsfs_root;
  sb->prev = NULL;
  
  if (cvsfs_root != NULL)
    cvsfs_root->prev = sb;
    
  cvsfs_root = sb;

  write_unlock (&root_lock);
  
  sprintf (sb->idstring, "%d", sb->id);
}



/* remove the current superblock from the managed list */
static void
cvsfs_remove_superblock (struct cvsfs_sb_info *sb)
{
  write_lock (&root_lock);

  /* unchain it from list */
  if (sb->prev == NULL)
  {
    cvsfs_root = sb->next;
    cvsfs_root->prev = NULL;	/* remove dangling pointer to old first sb */
  }
  else
    sb->prev = sb->next;

  /* for sanity */
  sb->next = NULL;
  sb->prev = NULL;

  write_unlock (&root_lock);
}



/* called when a mount is issued - initializes super block */
struct super_block *
cvsfs_read_super (struct super_block *sb, void *data, int silent)
{
  struct inode		*inode;
  struct cvsfs_sb_info	*info;
  struct cvsfs_fattr	root;
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: cvsfs_read_super\n");
#endif
  info = kmalloc (sizeof (struct cvsfs_sb_info), GFP_KERNEL);
  if (!info)
  {
    printk (KERN_ERR "cvsfs read_super: can not allocate info block - memory squeeze\n");

    return NULL;
  }

  memset (info, 0, sizeof (struct cvsfs_sb_info));

  sb->u.generic_sbp = info;		/* store network info */

  sb->s_blocksize = 1024; /* PAGE_CACHE_SIZE; */
  sb->s_blocksize_bits = 10; /* PAGE_CHACHE_SHIFT; */
  sb->s_magic = CVSFS_SUPER_MAGIC;
/*  sb->s_flags = 0;  */
/*  sb->s_flags |= MS_RDONLY; */
  sb->s_op = &cvsfs_sops;

  info->blocksize = sb->s_blocksize;
  info->blocksize_bits = sb->s_blocksize_bits;
  info->mount.file_mode = S_IRWXU | S_IRGRP | S_IROTH | S_IFREG;
  info->mount.dir_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IFDIR;
  info->mount.uid = current->uid;
  info->mount.gid = current->gid;
  init_MUTEX (&info->sem);
  init_MUTEX (&info->proc.lock);	/* Mutex required in proc.c */
  info->proc.view = strdup ("* /CHECKEDOUT\n* /HEAD\n");
  if (info->proc.view)
  {
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: read_super - view set.\n");
#endif

    info->parm_string = strdup ((char *) data);
    if (info->parm_string)
    {
#ifdef __DEBUG__
      printk (KERN_DEBUG "cvsfs: read_super - parameters saved -->%s\n", info->parm_string);
#endif
      if (cvsfs_parse_options (info, data) < 0)
        printk (KERN_ERR "cvsfs: read_super - invalid options\n");
      else
      {
        struct cvsfs_sb_info	*check;

#ifdef __DEBUG__
        printk (KERN_DEBUG "cvsfs: read_super - parameters parsed successfully.\n");
#endif
        check = cvsfs_read_lock_superblock_list ();
	while ((check != NULL) && (strcmp (check->mount.mountpoint, info->mount.mountpoint) != 0))
	  check = check->next;
	  
	cvsfs_read_unlock_superblock_list ();
	
	if (check == NULL)
	{
#ifdef __DEBUG__
          printk (KERN_DEBUG "cvsfs: read_super - mountpoint detected unused.\n");
#endif
          cvsfs_init_root_dirent (info, &root);

#ifdef __DEBUG__
          printk (KERN_DEBUG "cvsfs: read_super - root entry initialized.\n");
#endif
          inode = cvsfs_iget (sb, &root);
          if (inode)
          {
#ifdef __DEBUG__
            printk (KERN_DEBUG "cvsfs: read_super - root inode prepared.\n");
#endif
            sb->s_root = d_alloc_root (inode);

            if (sb->s_root)
            {
              cvsfs_add_superblock (info);	/* add to managed list of superblocks */
	    
#ifdef __DEBUG__
              printk (KERN_DEBUG "cvsfs(%d): read_super - root dentry allocated.\n", info->id);
#endif
	      if (cvsfs_procfs_user_init (info) == 0)
	      {
#ifdef __DEBUG__
                printk (KERN_DEBUG "cvsfs(%d): read_super - init user procfs completed.\n", info->id);
#endif
	        cvsfs_devfs_user_init (info);

	        printk (KERN_INFO "cvsfs(%d): project '//%s%s/%s' mounted\n", info->id,
	 	        info->connection.server, info->connection.root, info->connection.project);

                return sb;
	      }
	    
	      cvsfs_remove_superblock (info);
            }
          }
          iput (inode);
        }
      }
      kfree (info->connection.user);
      kfree (info->connection.pass);
      kfree (info->connection.root);
      kfree (info->connection.server);
      kfree (info->connection.project);
      kfree (info->mount.mountpoint);

      kfree (info->parm_string);
    }
    kfree (info->proc.view);
  }

  printk (KERN_ERR "cvsfs read_super: failed\n");

  kfree (info);

  return NULL;
}



/* called when a umount is issued - shuts down the super block */
static void
cvsfs_put_super (struct super_block * sb)
{
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) sb->u.generic_sbp;
  int id = info->id;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): put_super - '%s' unmounted\n", id, info->mount.mountpoint);
#endif

  cvsfs_devfs_user_cleanup (info);

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): put_super - devfs shut down\n", id);
#endif

  cvsfs_procfs_user_cleanup (info);

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs(%d): put_super - procfs shut down\n", id);
#endif

  cvsfs_remove_superblock (info);

  printk (KERN_INFO "cvsfs(%d): project '//%s%s/%s' unmounted\n", id,
	  info->connection.server, info->connection.root, info->connection.project);

  kfree (info->proc.view);
  kfree (info->parm_string);
  kfree (info->connection.user);
  kfree (info->connection.pass);
  kfree (info->connection.root);
  kfree (info->connection.server);
  kfree (info->connection.project);
  kfree (info->mount.mountpoint);

  kfree (info);
}



static int
cvsfs_statfs (struct super_block * sb, struct statfs * attr)
{
  attr->f_type = CVSFS_SUPER_MAGIC;
  attr->f_bsize = 1024;
  attr->f_blocks = 0;
  attr->f_namelen = CVSFS_MAXPATHLEN;
  attr->f_files = -1;
  attr->f_bavail = -1;

  return 0;
}



/* scambles a password according cvs pserver rules */
static void
cvsfs_password_scramble (char * data, char * result)
{
  char *src = data;
  char *dest = result;

  *dest = 'A';

  for (++dest; *src != '\0'; ++dest, ++src)
    *dest = shifts[(unsigned char) (*src)];

  *dest = '\0';
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

#undef __DEBUG__

/* identify the option to handle */
static int
cvsfs_identify_option (char *p)
{
  int loop;

#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: identify_option '%s'\n", p);
#endif
  
  for (loop = 0; (loop <= LAST_OPTION) && (cvsfs_option[loop] != NULL); ++loop)
  {
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: identify_option - is it option '%s' ?\n", cvsfs_option[loop]);
#endif
    if (strncmp (cvsfs_option[loop], p, strlen (cvsfs_option[loop])) == 0)
      return loop;
  }
  
  return INVALID_OPTION;
}



/* handles the options passed from the mount syscall */
static int
cvsfs_parse_options (struct cvsfs_sb_info * info, void * opts)
{
  char *p;
  char buffer[256];
  int  option;
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: cvsfs_parse_options '%s'\n", (char *) opts);
#endif
  info->connection.address.sin_addr.s_addr = cvsfs_inet_addr ("127.0.0.1");
  info->connection.address.sin_port = htons (2401);

  if (!opts)
    return -1;
    
  for (p = strtok (opts, ","); p; p = strtok (NULL, ","))
  {
#ifdef __DEBUG__
    printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - key = '%s'\n", p);
#endif

    if ((option = cvsfs_identify_option (p)) != INVALID_OPTION)
    {
      switch (option)
      {
        case OPTION_SERVER:
          info->connection.address.sin_addr.s_addr = cvsfs_inet_addr (&p[7]);
          sprintf (buffer, "%u.%u.%u.%u",
                   info->connection.address.sin_addr.s_addr &0xff,
    	           (info->connection.address.sin_addr.s_addr >> 8) &0xff,
	           (info->connection.address.sin_addr.s_addr >> 16) &0xff,
	           (info->connection.address.sin_addr.s_addr >> 24) &0xff);
          if ((info->connection.server = strdup (buffer)) == NULL)
            return -1;

          break;

        case OPTION_MODULE:	
          info->connection.project = strdup (&p[7]);
	  break;
	  
	case OPTION_USER:
          info->connection.user = strdup (&p[5]);
	  break;

        case OPTION_PASSWORD:
	  if (strlen (&p[9]) >= sizeof (buffer))
	    p[7 + sizeof (buffer)] = '\0';
	    
          cvsfs_password_scramble (&p[9], buffer);
	  info->connection.pass = strdup (buffer);
	  break;

        case OPTION_CVSROOT:
	  info->connection.root = strdup (&p[8]);
	  break;

        case OPTION_UID:
	  info->mount.uid = simple_strtoul (&p[4], NULL, 0);
#ifdef __DEBUG__
	  printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - uid = %d\n", info->mount.uid);
#endif
	  break;

	case OPTION_GID:
	  info->mount.gid = simple_strtoul (&p[4], NULL, 0);
#ifdef __DEBUG__
	  printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - gid = %d\n", info->mount.gid);
#endif
          break;

        case OPTION_FMASK:
	  info->mount.file_mode = simple_strtoul (&p[6], NULL, 0) | S_IFREG;
#ifdef __DEBUG__
	  printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - fmask = %d\n", info->mount.file_mode);
#endif
          break;

	case OPTION_DMASK:
	  info->mount.dir_mode = simple_strtoul (&p[6], NULL, 0) | S_IFDIR;
#ifdef __DEBUG__
	  printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - dmask = %d\n", info->mount.dir_mode);
#endif
          break;

        case OPTION_MOUNT_USER:
	  info->mount_uid = simple_strtoul (&p[11], NULL, 0);
#ifdef __DEBUG__
	  printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - mount_uid = %d\n", info->mount_uid);
#endif
          break;

        case OPTION_MOUNT_GROUP:
	  info->mount_gid = simple_strtoul (&p[12], NULL, 0);
#ifdef __DEBUG__
	  printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - mount_gid = %d\n", info->mount_gid);
#endif
          break;

        case OPTION_MOUNT_POINT:
	  info->mount.mountpoint = strdup (&p[6]);
#ifdef __DEBUG__
	  printk (KERN_DEBUG "cvsfs: cvsfs_parse_options - mount point = %s\n", info->mount.mountpoint);
#endif
	  break;

        default:
	  printk (KERN_ERR "Invalid option id '%d' passed\n", option);
      }
    }
    else
    {
      printk (KERN_ERR "Invalid option '%s' passed\n", p);
    }
  }

  /* apply default values if notgiven by the user */
  if (info->connection.server == NULL)
    info->connection.server = strdup ("127.0.0.1");

  if (info->connection.user == NULL)
    info->connection.user = strdup ("anonymous");

  if (info->connection.pass == NULL)
    info->connection.pass = strdup ("A");	/* encrypted version of empty string */

  if (info->connection.root == NULL)
    info->connection.root = strdup ("/cvsroot");

  if (info->connection.project == NULL)
    info->connection.project = strdup ("\0");

  if (info->mount.mountpoint == NULL)
    info->mount.mountpoint = strdup ("\0");

  return 0;
}
