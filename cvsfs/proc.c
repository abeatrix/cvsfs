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

#include "proc.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/net.h>
#include <linux/string.h>

#include <net/scm.h>
#include <net/ip.h>

#include <asm/uaccess.h>

#include "inode.h"
#include "cache.h"

#define CVSFS_MAX_LINE	512


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



void cvsfs_password_scramble (char * data, char * result)
{
  char *src = data;
  char *dest = result;

  *dest = 'A';

  for (++dest; *src != '\0'; ++dest, ++src)
    *dest = shifts[(unsigned char) (*src)];

  *dest = '\0';
}



void cvsfs_init_root_dirent (struct cvsfs_sb_info * server, struct cvsfs_fattr * fattr)
{
  printk (KERN_DEBUG "cvsfs: cvsfs_init_root_dirent\n");

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



unsigned long cvsfs_inet_addr (char * ip)
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



int cvsfs_parse_options (struct cvsfs_sb_info * info, void * opts)
{
  char *p;

  printk (KERN_DEBUG "cvsfs: cvsfs_parse_options '%s'\n", (char *) opts);

  info->address.sin_addr.s_addr = cvsfs_inet_addr ("127.0.0.1");
  info->address.sin_port = htons (2401);
  strcpy (info->user, "anonymous");
  strcpy (info->pass, "");
  strcpy (info->mnt.root, "/cvsroot");
  *(info->mnt.project) = '\0';

  if (!opts)
    return -1;
    
  for (p = strtok (opts, ","); p; p = strtok (NULL, ","))
  {
    if (strncmp (p, "server=", 7) == 0)
      info->address.sin_addr.s_addr = cvsfs_inet_addr (&p[7]);
    else
      if (strncmp (p, "module=", 7) == 0)
        strncpy (info->mnt.project, &p[7], sizeof (info->mnt.project) - 1);
      else
        if (strncmp (p, "user=", 5) == 0)
          strncpy (info->user, &p[5], sizeof (info->user) - 1);
        else
          if (strncmp (p, "password=", 9) == 0)
            strncpy (info->pass, &p[9], sizeof (info->pass) - 1);
          else
            if (strncmp (p, "cvsroot=", 8) == 0)
              strncpy (info->mnt.root, &p[8], sizeof (info->mnt.root) - 1);
            else
	      printk (KERN_DEBUG "Invalid option '%s' passed\n", p);
  }

  return 0;
}



int sock_send (struct socket * sock, const void * buf, int len)
{
  struct iovec iov;
  struct msghdr msg;
  struct scm_cookie scm;
  int err;
  mm_segment_t fs;

  fs = get_fs ();
  set_fs (get_ds ());

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;

  iov.iov_base = (void *) buf;
  iov.iov_len = len;

  err = scm_send (sock, &msg, &scm);

  if (err >= 0)
  {
    err = sock->ops->sendmsg (sock, &msg, len, &scm);

    scm_destroy (&scm);
  }

  set_fs (fs);

  return err;
}



int sock_recv (struct socket * sock, unsigned char * buf, int size, unsigned flags)
{
  struct iovec iov;
  struct msghdr msg;
  struct scm_cookie scm;
  mm_segment_t fs;

  fs = get_fs ();
  set_fs (get_ds ());

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;

  iov.iov_base = buf;
  iov.iov_len = size;

  memset (&scm, 0, sizeof (scm));

  size = sock->ops->recvmsg (sock, &msg, size, flags, &scm);

  if (size >= 0)
    scm_recv (sock, &msg, &scm, flags);

  set_fs (fs);

  return size == -EAGAIN ? 0 : size;
}


/*
int cvsfs_readline (struct socket * sock, char * buf, int len)
{
  int i = 0;
  int out = 0;
  int size;

  do
  {
    size = sock_recv (sock, &(buf[i]), 1, 0);

    if (size >= 0)
    {
      if (size > 0)
      {
        if ((buf[i] == '\012') || (buf[i] == '\015'))
          out = 1;

        ++i;
      }
    }
    else
    {
      buf[i] = 0;

      return -1;
    }
  } while ((i < len) && (out == 0));

  buf[i] = 0;

  printk (KERN_DEBUG "cvsfs: cvsfs_readline - returns '%s'\n", buf);

  return i;
}
*/


int cvsfs_readline (struct socket * sock, char * buf, int len)
{
  int i = 0;
  int out = 0;
  int count = 10;             /* up to 10 retries */
  int size;
  unsigned char *ptr;

  ptr = (unsigned char *) buf;
  do
  {
    size = sock_recv (sock, ptr, 1, MSG_PEEK | MSG_DONTWAIT);

    if (size >= 0)
    {
      if (size > 0)
      {
        sock_recv (sock, ptr, 1, 0);

        if ((*ptr == '\012') || (*ptr == '\015'))
	{
	  *ptr = '\0';
          out = 1;
        }
        ++i;
        ++ptr;
        count = 10;
      }
      else
      {
        long wait = jiffies + (HZ/20) * (11 - count);
        for (;time_before (jiffies, wait);)
          schedule ();
      }
    }
    else
    {
      *ptr = 0;

      return -1;
    }

    --count;
  } while ((i < len) && (out == 0) && (count > 0));

  *ptr = 0;

  return i;
}



int cvsfs_execute (struct cvsfs_sb_info * info, char * cmd)
{
  printk (KERN_DEBUG "cvsfs: cvsfs_execute - command: '%s'\n", cmd);

  if (sock_send (info->sock, cmd, strlen (cmd)) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_execute - send error !\n");

    return -1;
  }

  if (sock_send (info->sock, "\012", 1) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_execute - send error (newline) !\n");

    return -1;
  }

  return 0;
}



int cvsfs_login (struct cvsfs_sb_info * info, int test)
{
  char buf[CVSFS_MAX_LINE];
  char scrambled[CVSFS_MAX_PASS + 1];
  int i;

  printk (KERN_DEBUG "cvsfs: cvsfs_login\n");

  if (test == 0)
  {
    if (cvsfs_execute (info, "BEGIN AUTH REQUEST") < 0)
    {
      printk (KERN_DEBUG "cvsfs: cvsfs_login - 'BEGIN AUTH REQUEST' failed !\n");

      return -1;
    }
  }
  else
  {
    if (cvsfs_execute (info, "BEGIN VERIFICATION REQUEST") < 0)
    {
      printk (KERN_DEBUG "cvsfs: cvsfs_login - 'BEGIN VERIFICATION REQUEST' failed !\n");

      return -1;
    }
  }

  if (cvsfs_execute (info, info->mnt.root) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_login - '%s' failed !\n", info->mnt.root);

    return -1;
  }

  if (cvsfs_execute (info, info->user) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_login - userid '%s' failed !\n", info->user);

    return -1;
  }

  cvsfs_password_scramble (info->pass, scrambled);
  if (cvsfs_execute (info, scrambled) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_login - password failed !\n");

    return -1;
  }

  if (test == 0)
  {
    if (cvsfs_execute (info, "END AUTH REQUEST") < 0)
    {
      printk (KERN_DEBUG "cvsfs: cvsfs_login - 'END AUTH REQUEST' failed !\n");

      return -1;
    }
  }
  else
  {
    if (cvsfs_execute (info, "END VERIFICATION REQUEST") < 0)
    {
      printk (KERN_DEBUG "cvsfs: cvsfs_login - 'END VERIFICATION REQUEST' failed !\n");

      return -1;
    }
  }

  i = cvsfs_readline (info->sock, buf, CVSFS_MAX_LINE);
  if (i < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_login - no response !\n");

    return -1;
  }

  if (strncmp (buf, "I LOVE YOU", 10) != 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_login - response '%s' (failure) !\n", buf);

    return -1;
  }

  return 0;
}



int cvsfs_connect (struct cvsfs_sb_info * info, int test)
{
  struct sockaddr_in address;
/*  struct timeval tv; */
/*  mm_segment_t fs; */

  info->address.sin_family = AF_INET;

  printk (KERN_DEBUG "cvsfs: cvsfs_connect\n");

  if (info->sock)
    cvsfs_disconnect (info);

  if (sock_create (AF_INET, SOCK_STREAM, 0, &info->sock) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_connect - create socket failed !\n");

    return -1;
  }

  address = info->address;
  if (info->sock->ops->connect (info->sock, (struct sockaddr *) &address, sizeof (address), 0) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_connect - connect failed !\n");

    return -1;
  }

  /* Setting of timeout is not working - i don't know why */
  /* so i have to use a busy-wait when reading data - bad */
/*  fs = get_fs (); */
/*  set_fs (get_ds ()); */

  /* set a read timeot of 5 seconds */
/*  tv.tv_sec = 5; */
/*  tv.tv_usec = 0; */
  
/*  info->sock->ops->setsockopt (info->sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof (tv)); */

/*  set_fs (fs); */

  if (cvsfs_login (info, test) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_connect - login failed !\n");

    return -1;
  }

//  if (cvsfs_execute (info, "Valid-responses ok error Valid-requests Checked-in New-entry Checksum Copy-file Updated Created Update-existing Merged Patched Rcs-diff Mode Mod-time Removed Remove-entry Set-static-directory Clear-static-directory Set-sticky Clear-sticky Template Set-checkin-prog Set-update-prog Notified Module-expansion Wrapper-rcsOption M Mbinary E F MT") < 0)
//  {
//    printk (KERN_DEBUG "cvsfs: cvsfs_login - 'Valid-responses' failed !\n");

//    return -1;
//  }

  if (test != 0)
  {
    sock_release (info->sock);

    info->sock = NULL;
  }

  return 0;
}



void cvsfs_disconnect (struct cvsfs_sb_info * info)
{
  printk (KERN_DEBUG "cvsfs: cvsfs_disconnect\n");

  if (info->sock)
    sock_release (info->sock);

  info->sock = NULL;
}



int cvsfs_getIP (char * buf, unsigned * ip, unsigned short * port)
{
  char *b;
  int i;

  *ip = *port = 0;

  buf = strchr (buf, '(') + 1;
  if (!buf)
    return -1;

  for (i = 0; i < 4; ++i)
  {
    b = buf;
    *ip = (*ip >> 8) + (simple_strtoul (b, &b, 0) << 24);
    buf = strchr (buf, ',') + 1;
  }

  b = buf;
  *port = simple_strtoul (b, &b, 0) << 8;
  buf = strchr (buf, ',') + 1;
  b = buf;
  *port = htons (*port + simple_strtoul (b, &b, 0));

  return 0;
}



int cvsfs_get_fname (char * s, char * d)
{
  static char *prefix = "M File ";
  static char *revision = " is new; current revision ";
  static char *dir = "E cvs server: Diffing ";
  int ret = -1;

  printk (KERN_DEBUG "cvsfs: cvsfs_get_fname - line: '%s'\n", s);


  if (strncmp (s, prefix, strlen (prefix)) == 0)
  {
    char *ptr;
    int tocheck;
    int stop = 1;
    int i;

    printk (KERN_DEBUG " -- is a file\n");

    ptr = &(s[strlen (prefix)]);
    tocheck = strlen (ptr) - strlen (revision);
    for (tocheck = strlen (ptr) - strlen (revision); tocheck > 0; --tocheck)
    {
      stop = 0;

      for (i = 0; (i < strlen (revision)) && (stop == 0) ; ++i)
        if (ptr[i] != revision[i])
          stop = 1;

      if (stop == 0)
        break;

      *d = *ptr;
      ++d;
      ++ptr;
    }

    if (stop == 0)
    {
      *d = '\0';
      ret = 1;
    }
  }
  else
    if (strncmp (s, dir, strlen (dir)) == 0)
    {
      strcpy (d, &(s[strlen (dir)]));

      ret = 0;
    }

  printk (KERN_DEBUG "cvsfs: cvsfs_get_fname - rc = %d, '%s'\n", ret, d);

  return ret;
}



int cvsfs_loaddir (struct cvsfs_sb_info * info, char * name, struct cvsfs_directory * dir)
{
  struct cvsfs_dirlist_node *p;
  char res[CVSFS_MAX_LINE];
  char line[CVSFS_MAX_LINE];
  char basedir[CVSFS_MAX_LINE];
  char *ptr;
  int len;
  int i;
  int count;

  printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - directory '%s'\n", name);

  strcpy (basedir, info->mnt.project);
  strcat (basedir, name);
  len = strlen (basedir);
  if (basedir[len - 1] == '/')
  {
    basedir[len - 1] = '\0';         /* purge trailing slashes */
    --len;
  }

//  buf = (char *) kmalloc (strlen (name) + strlen (info->mnt.project) + 11, GFP_KERNEL);
//  if (!buf)
//  {
//    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - memory squeeze !\n");

//    return -1;
//  }

  if (cvsfs_connect (info, 0) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - connect failed !\n");

    return -1;
  }

  strcpy (res, "Root ");
  strcat (res, info->mnt.root);

  if (cvsfs_execute (info, res) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - '%s' failed !\n", res);

    cvsfs_disconnect (info);

    return -1;
  }

//  if (cvsfs_execute (info, "UseUnchanged") < 0)
//  {
//    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - 'UseUnchanged' failed !\n");

//    return -1;
//  }

  if (cvsfs_execute (info, "Argument -s") < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - 'Argument -s' failed !\n");

    cvsfs_disconnect (info);

    return -1;
  }

  if (cvsfs_execute (info, "Argument -r") < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - 'Argument -r' failed !\n");

    cvsfs_disconnect (info);

    return -1;
  }

  if (cvsfs_execute (info, "Argument 0") < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - 'Argument 0' failed !\n");

    cvsfs_disconnect (info);

    return -1;
  }
  
  strcpy (res, "Argument ");
  strcat (res, basedir);

  if (cvsfs_execute (info, res) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - '%s' failed !\n", res);

    cvsfs_disconnect (info);

    return -1;
  }

//  kfree (buf);

  if (cvsfs_execute (info, "rdiff") < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - 'rdiff' failed !\n");

    cvsfs_disconnect (info);

    return -1;
  }

  count = 40;

  i = cvsfs_readline (info->sock, line, CVSFS_MAX_LINE);
  if (i < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - no response !\n");

    cvsfs_disconnect (info);

    return -1;
  }

  while (strcmp (line, "ok") != 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - handle string '%s'\n", line);
    
    if (strcmp (line, "error") == 0)
    {
      printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - error while reading directory\n");

      cvsfs_disconnect (info);

      return -1;
    }

    i = cvsfs_get_fname (line, res);
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
        p = (struct cvsfs_dirlist_node *) kmalloc (sizeof (struct cvsfs_dirlist_node), GFP_KERNEL);
        memset (p, 0, sizeof (struct cvsfs_dirlist_node));

        p->entry.name = (char *) kmalloc (strlen (ptr) + 1, GFP_KERNEL);
        if (!p->entry.name)
        {
          kfree (p);

          return -1;
        }

        strcpy (p->entry.name, ptr);
        p->entry.size = 0;
        p->entry.blocksize = 1024;
        p->entry.blocks = (p->entry.size + 1023) >> 9;

        if (i == 0)				/* directory */
        {
          printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - add subdirectory '%s'\n", res);
          p->entry.mode |= S_IFDIR | S_IRUSR | S_IXUSR;
        }
        else
          if (i == 1)				/* file */
          {
            printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - add file '%s'\n", res);
            p->entry.mode |= S_IFREG | S_IRUSR;
          }

        p->prev = NULL;
        p->next = dir->head;
        dir->head = p;
      }
      
      count = 40;
    }
    else
    {
      long wait = jiffies + (HZ/4);
      for (;time_before (jiffies, wait);)
        schedule ();
	
      --count;
    }
    
    i = cvsfs_readline (info->sock, line, CVSFS_MAX_LINE);
    if ((i < 0) || (count < 0))
    {
      printk (KERN_DEBUG "cvsfs: cvsfs_loaddir - no response !\n");

      cvsfs_disconnect (info);

      return -1;
    }
  }

  cvsfs_disconnect (info);

  return 0;
}



int cvsfs_get_name (struct dentry * d, char * name)
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



inline void cvsfs_lock (struct cvsfs_sb_info * info)
{
  down (&(info->sem));
}



inline void cvsfs_unlock (struct cvsfs_sb_info * info)
{
  up (&(info->sem));
}



int cvsfs_get_attr (struct dentry * dentry, struct cvsfs_fattr * fattr, struct cvsfs_sb_info * info)
{
  struct cvsfs_directory *dir;
  char buf[CVSFS_MAX_LINE];
  struct cvsfs_dirlist_node *file;

  cvsfs_get_name (dentry->d_parent, buf);

  dir = cvsfs_cache_get (info, buf);

  if (!dir)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_get_attr - cvsfs_cache_get failed !\n");

    return -1;
  }

  for (file = dir->head; file != NULL; file = file->next)
    if (strcmp (dentry->d_name.name, file->entry.name) == 0)
      break;

  if (!file)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_get_attr - file not found in parent dir cache !\n");

    return -1;
  }

  fattr->f_mode = file->entry.mode;
  fattr->f_size = file->entry.size;
  fattr->f_blksize = file->entry.blocksize;
  fattr->f_blocks = file->entry.blocks;
  fattr->f_nlink = file->entry.nlink;
  fattr->f_mtime = CURRENT_TIME;
  fattr->f_uid = info->mnt.uid;
  fattr->f_gid = info->mnt.gid;

  return 0;
}



int cvsfs_read (struct dentry * dentry, unsigned long offset, unsigned long count, char *buffer)
{
  struct inode *inode = dentry->d_inode;
  struct super_block *sb = inode->i_sb;
  struct cvsfs_sb_info *info = (struct cvsfs_sb_info *) sb->u.generic_sbp;
  char buf[CVSFS_MAXPATHLEN + 6];
//  char buf2[CVSFS_MAXPATHLEN + 6];
  int res = 0;

  cvsfs_lock (info);

  cvsfs_get_name (dentry, buf);






  cvsfs_unlock (info);

  return res;
}
