/***************************************************************************
                          TModuleAction.h  -  description
                             -------------------
    begin                : Sun Jun 9 18:32:49 CEST 2002
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

#include "TModuleAction.h"

#include <stdio.h>
#include "TModuleServer.h"
#include "TSyslog.h"



TModuleAction::TModuleAction ()
: fServer (0)
{
}



TModuleAction::TModuleAction (const TModuleAction & action)
{
  fServer = action.fServer;
}



TModuleAction::~TModuleAction ()
{
}



void TModuleAction::SetServer (TModuleServer *server)
{
  fServer = server;
}



int TModuleAction::readLine (char *buf, int max)
{
  return fServer->readLine (buf, max);
}



int TModuleAction::readItem (char *buf, int max)
{
  return fServer->readItem (buf, max);
}



int TModuleAction::readData (char *buf, int max)
{
  return fServer->read (buf, max, max);
}



void TModuleAction::writeDummy ()
{
  fServer->write ("0", 1);
}



void TModuleAction::writeData (const std::string & data)
{
  writeData (data.c_str (), data.length ());
}



void TModuleAction::writeData (const char *buf, int size)
{
  TSyslog *log = TSyslog::instance ();
  char *ptr = new char[size + 32];
  int width;

  sprintf (ptr, "%i ", size);
  width = strlen (ptr);
  memcpy (&(ptr[width]), buf, size + 1);
  ptr[width + size] = '\0';

  log->debug << "writing >" << ptr << "<" << std::endl;

  fServer->write (ptr, size + width);
  delete [] ptr;
}
