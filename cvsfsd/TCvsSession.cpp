/***************************************************************************
                          TCvsSession.cpp  -  description
                             -------------------
    begin                : Mon Jun 11 18:32:49 CEST 2002
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

//#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "TCvsSession.h"

#include "TCvsConnection.h"



TCvsSession::TCvsSession (TCvsConnection *connection)
: fConnection (connection)
{
}



TCvsSession::~TCvsSession ()
{
}
