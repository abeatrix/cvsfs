/***************************************************************************
                          socket.c  -  description
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
#include "socket.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/string.h>

#include <net/scm.h>

#include <asm/uaccess.h>	// for get_fs, set_fs functions



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



int cvsfs_read_raw_data (struct socket * sock, unsigned long count, char * buffer)
{
  int i = 0;
  int retries = 10;             /* up to 10 retries */
  int size;
  char data;
  unsigned char *ptr;

  if (count == 0)
    return 0;

  if (buffer == NULL)
  {
    ptr = &data;
  }
  else
  {
    ptr = (unsigned char *) buffer;
  }
      
  do
  {
    size = sock_recv (sock, ptr, 1, MSG_PEEK | MSG_DONTWAIT);

    if (size >= 0)
    {
      if (size > 0)
      {
        sock_recv (sock, ptr, 1, 0);

        ++i;
	if (buffer != NULL)
          ++ptr;
	  
        retries = 10;
      }
      else
      {
        long wait = jiffies + (HZ/20) * (11 - retries);
        for (;time_before (jiffies, wait);)
          schedule ();
      }
    }
    else
    {
      *ptr = 0;

      return -1;
    }

    --retries;
    if (retries == 0)
      return -1;
      
  } while (i < count);

  return i;
}



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



int cvsfs_long_readline (struct socket * sock, char * buf, int len)
{
  int count;
  int ret;
  long wait;
  
  count = 40;
  
  if ((ret = cvsfs_readline (sock, buf, len)) < 0)
    return ret;
    
  while (ret == 0)
  {
    --count;
    
    wait = jiffies + (HZ/4);
    for (;time_before (jiffies, wait);)
      schedule ();
    
    if (count == 0)
      return -1;
      
    ret = cvsfs_readline (sock, buf, len);
  }
  
  return ret;
}



int cvsfs_execute (struct socket * sock, char * cmd)
{
  printk (KERN_DEBUG "cvsfs: cvsfs_execute - command: '%s'\n", cmd);

  if (sock_send (sock, cmd, strlen (cmd)) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_execute - send error !\n");

    return -1;
  }

  if (sock_send (sock, "\012", 1) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_execute - send error (newline) !\n");

    return -1;
  }

  return 0;
}



int cvsfs_execute_command (struct socket * sock, ...)
{
  va_list args;
  char *value;
  
  va_start (args, sock);

  value = va_arg (args, char *);
  while (value != NULL)
  {
    if (sock_send (sock, value, strlen (value)) < 0)
    {
      printk (KERN_DEBUG "cvsfs: cvsfs_execute_command - send error for: %s\n", value);

      return -1;
    }

    value = va_arg (args, char *);
  }

  va_end (args);
  
  if (sock_send (sock, "\012", 1) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_execute_command - send error (newline) !\n");

    return -1;
  }
  
  return 0;
}



int cvsfs_login (struct socket * sock, char * user, char * password, char * root, int test)
{
  char buf[CVSFS_MAX_LINE];
  int i;

  printk (KERN_DEBUG "cvsfs: cvsfs_login\n");

  if (test == 0)
  {
    if (cvsfs_execute (sock, "BEGIN AUTH REQUEST") < 0)
      return -1;
  }
  else
  {
    if (cvsfs_execute (sock, "BEGIN VERIFICATION REQUEST") < 0)
      return -1;
  }

  if (cvsfs_execute (sock, root) < 0)
    return -1;

  if (cvsfs_execute (sock, user) < 0)
    return -1;

  if (cvsfs_execute (sock, password) < 0)
    return -1;

  if (test == 0)
  {
    if (cvsfs_execute (sock, "END AUTH REQUEST") < 0)
      return -1;
  }
  else
  {
    if (cvsfs_execute (sock, "END VERIFICATION REQUEST") < 0)
      return -1;
  }

  i = cvsfs_readline (sock, buf, CVSFS_MAX_LINE);
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

  if (test == 0)
  {
    if (cvsfs_execute_command (sock, "Root ", root, NULL) < 0)
    {
      printk (KERN_DEBUG "cvsfs: cvsfs_login - 'Root %s' failed !\n", root);
      
      return -1;
    }
    
    if (cvsfs_execute (sock, "Valid-responses ok error Valid-requests Checked-in New-entry Checksum Copy-file Updated Created Update-existing Merged Patched Rcs-diff Mode Mod-time Removed Remove-entry Set-static-directory Clear-static-directory Set-sticky Clear-sticky Template  Set-checkin-prog Set-update-prog Notified Module-expansion Wrapper-rcsOption M  Mbinary E F MT") < 0)
    {
      printk (KERN_DEBUG "cvsfs: cvsfs_login - 'Valid-responses' failed !\n");

      return -1;
    }
  }

  return 0;
}



int cvsfs_connect (struct socket **sockptr, char * user, char * password, char * root, struct sockaddr_in address, int test)
{
  struct sockaddr_in local_address;
/*  struct timeval tv; */
/*  mm_segment_t fs; */

//  address->sin_family = AF_INET;

  printk (KERN_DEBUG "cvsfs: cvsfs_connect\n");

  if (*sockptr)
    cvsfs_disconnect (sockptr);

  if (sock_create (AF_INET, SOCK_STREAM, 0, sockptr) < 0)
  {
    printk (KERN_DEBUG "cvsfs: cvsfs_connect - create socket failed !\n");

    return -1;
  }

  local_address = address;
  local_address.sin_family = AF_INET;
  if ((*sockptr)->ops->connect (*sockptr, (struct sockaddr *) &local_address, sizeof (local_address), 0) < 0)
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

  if (cvsfs_login (*sockptr, user, password, root, test) < 0)
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
    sock_release (*sockptr);

    *sockptr = NULL;
  }

  return 0;
}



void cvsfs_disconnect (struct socket **sockptr)
{
  printk (KERN_DEBUG "cvsfs: cvsfs_disconnect\n");

  if (sockptr && (*sockptr))
  {
    sock_release (*sockptr);
    
    *sockptr = NULL;
  }
}
