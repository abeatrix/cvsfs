/***************************************************************************
                          cvsfs_ioctl.h  -  global ioctl definitions
                             -------------------
    begin                : Sat Aug 24 2002
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

/***************************************************************************
 *   Following IOCTLs are defined:                                         *
 *                                                                         *
 *   CVSFS_IOC_XVERSION - obtain version of file                           *
 *     Entry:  Parameter contains pointer to 'limited string'              *
 *     Return: Parameter filled with version string                        *
 *             Return value >= 0 ... size of string                        *
 *                          < 0 .... error code                            *
 ***************************************************************************/

#ifndef __CVSFS_IOCTL_H__
#define __CVSFS_IOCTL_H__

#include <linux/ioctl.h>

#define CVSFS_IOC_MAGIC	245

/* the following IOCTL requests are supported */
#define CVSFS_RESCAN		0	/* re-evaluate file/dir attribs */
#define CVSFS_GET_VERSION	1	/* obtain version item */
#define CVSFS_CHECKOUT		2	/* check out file */

/* types to be used as parameters */
typedef struct
{
  int	maxsize;
  char	*string;
} limited_string;

/* the ioc commands are named in a specific style: */
/*                                                 */
/* CVSFS_IOCxyzzzzzzzz                             */
/*          !!   !                                 */
/*          !!   +-- the function to execute       */
/*          !+- return value style                 */
/*          !    _ = no return value               */
/*          !    P = return via pointer            */
/*          !    V = return by value               */
/*          !    X = return via pointer and value  */
/*          +-- parameter passing style            */
/*               _ = no parameters                 */
/*               P = set via a pointer             */
/*               V = set by value                  */
#define	CVSFS_IOC__RESCAN	_IO (CVSFS_IOC_MAGIC, CVSFS_RESCAN)
#define CVSFS_IOC_XVERSION	_IOR (CVSFS_IOC_MAGIC, CVSFS_GET_VERSION, limited_string)
#define CVSFS_IOCPVCHECKOUT	_IOW (CVSFS_IOC_MAGIC, CVSFS_CHECKOUT, limited_string)



#endif
