/***************************************************************************
                          TCvsConnectionPserver.cpp  -  description
                             -------------------
    begin                : Sat Aug 10 2002
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "TCvsConnectionPserver.h"

#include "TConnectedSocket.h"
#include "TCvsSessionPserver.h"
#include "TMountParameters.h"


TCvsConnectionPserver::TCvsConnectionPserver (const TMountParameters & parameters)
: TCvsConnection (parameters)
{
}



TCvsConnectionPserver::~TCvsConnectionPserver ()
{
}



TCvsSessionPserver * TCvsConnectionPserver::Open ()
{
  // connect to CVS pserver
  TConnectedSocket *connection = fSocket.Connect (2401, fServer.c_str ());
  if (connection)
    return new TCvsSessionPserver (connection, this);

  return 0;
}
