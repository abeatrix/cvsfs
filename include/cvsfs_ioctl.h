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
 *                                                                         *
 *   CVSFS_IOC_VCHECKOUT - check-out file with current version             *
 *     Entry:  none                                                        *
 *     Return: Return value >= 0 ... size of string                        *
 *                          < 0 .... error code                            *
 *                                                                         *
 *   CVSFS_IOCPVCHECKOUT - check-out file of specific version              *
 *     Entry:  Parameter contains the version to obtain                    *
 *     Return: Return value >= 0 ... size of string                        *
 *                          < 0 .... error code                            *
 *                                                                         *
 *   CVSFS_IOC_VCHECKIN - check-in file                                    *
 *     Entry:  none                                                        *
 *     Return: Return value >= 0 ... size of string                        *
 *                          < 0 .... error code                            *
 *                                                                         *
 *   CVSFS_IOCPVCHECKIN - check-in file to specific version                *
 *     Entry:  Parameter contains the version to obtain                    *
 *     Return: Return value >= 0 ... size of string                        *
 *                          < 0 .... error code                            *
 *                                                                         *
 *   CVSFS_IOC_VUPDATE - update file with the one from CVS                 *
 *     Entry:  none                                                        *
 *     Return: Return value >= 0 ... size of string                        *
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
#define CVSFS_CHECKOUT_VERSION	3	/* check out specific file version */
#define CVSFS_CHECKIN		4	/* check in file */
#define CVSFS_CHECKIN_VERSION	5	/* check in to specific file version */
#define CVSFS_UPDATE		6	/* update a file */

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
#define CVSFS_IOC_VCHECKOUT	_IO (CVSFS_IOC_MAGIC, CVSFS_CHECKOUT)
#define CVSFS_IOCPVCHECKOUT	_IOW (CVSFS_IOC_MAGIC, CVSFS_CHECKOUT_VERSION, limited_string)
#define CVSFS_IOC_VCHECKIN	_IO (CVSFS_IOC_MAGIC, CVSFS_CHECKIN)
#define CVSFS_IOCPVCHECKIN	_IOW (CVSFS_IOC_MAGIC, CVSFS_CHECKIN_VERSION, limited_string)
#define CVSFS_IOC_VUPDATE	_IO (CVSFS_IOC_MAGIC, CVSFS_UPDATE)



#endif
