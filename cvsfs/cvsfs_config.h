/***************************************************************************
                          cvsfs_config.h  -  description
                             -------------------
    begin                : Wed Nov 7 2001
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

#ifndef __CVSFS_CONFIG_H__
#define __CVSFS_CONFIG_H__

#include <linux/config.h>

#if defined (CONFIG_MODVERSIONS) && !defined (MODVERSIONS)
#define MODVERSIONS
#endif

#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif


#define CVSFS_MAX_LINE	512



#endif
