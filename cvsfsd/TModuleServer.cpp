/***************************************************************************
                          TModuleServer.cpp  -  description
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

#include "TModuleServer.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "TModuleAction.h"
#include "TMountParameters.h"
#include "TCvsInterface.h"
#include "TSyslog.h"



TModuleServer::TModuleServer ()
: fDevice (""), fInDeviceFile (stdin), fOutDeviceFile (stdout), fInitSequence (true)
{
  init ();
}



TModuleServer::TModuleServer (const std::string & device)
: fDevice (device), fInitSequence (true)
{
  fInDeviceFile = fopen (device.c_str (), "r+");
  fOutDeviceFile = fInDeviceFile;
  
  init ();
}



TModuleServer::~TModuleServer ()
{
  MapType::iterator iter;

  if ((fInDeviceFile != NULL) && (fInDeviceFile == fOutDeviceFile))
    fclose (fInDeviceFile);
    
  for (iter = fActions.begin (); iter != fActions.end (); ++iter)
    delete (*iter).second;
};



void TModuleServer::AddAction (const std::string & key, TModuleAction * action)
{
  action->SetServer (this);

  fActions.insert (ValuePair (key, action));
}



bool TModuleServer::ready ()
{
  return (fInDeviceFile) && ((fMountParameters) || fInitSequence);
}



void TModuleServer::init ()
{
  TSyslog *log = TSyslog::instance ();
  char buf[512];

  if (fDevice == "")
  {
    strcpy (buf, "mount=/home/petric/zzz,server=192.168.1.8,module=.,uid=500,gid=100,fmask=420,dmask=493,mount_user=500,mount_group=100,user=petric,password=walter");
    //strcpy (buf, "mount=/home/petric/zzz,server=192.168.1.8,module=test,uid=500,gid=100,fmask=420,dmask=493,mount_user=500,mount_group=100,user=petric,password=walter");
  }
  else
  {
    if (readLine (buf, sizeof (buf)) == -1)
      return;
  }

  fInitSequence = false;

  fMountParameters = new TMountParameters (buf);
  log->debug << "Init: " << buf << std::endl;
}



void TModuleServer::run (TCvsInterface & interface)
{
  TSyslog *log = TSyslog::instance ();
  MapType::iterator iter;
  bool quit;

  // now run the communication with the kernel module
  quit = false;
  while ((ready ()) && (!quit))
  {
    char value[256];
    int size;

    size = readItem (value, sizeof (value));

    if (size > 0)
    {
      std::string key = value;

      log->debug << "Key: >" << key << "<" << std::endl;
    
      // issue action
      iter = fActions.find (key);
      if (iter != fActions.end ())
        quit = (*iter).second->doit (interface);	// is responsible to send a reply
      else
        write ("0", 1);	// dummy response to satisfy kernel module
    }
    else
      if (size == -1)
        quit = true;
  }
}



void TModuleServer::write (const char *buf, int count)
{
  if (ready ())
    fwrite (buf, count, 1, fOutDeviceFile);
}



int TModuleServer::read (char *buf, int max)
{
  if (ready ())
  {
    char *ptr;
    int count;
    int item;

    --max;
    ptr = buf;
    count = 0;
    item = fgetc (fInDeviceFile);
    while ((item != EOF) && (count < max))
    {
      *ptr = item;
      ++count;
      ++ptr;
      
      item =  fgetc (fInDeviceFile);
    }
    
    *ptr = '\0';
    
    return count;
  }
  
  return -1;
}



int TModuleServer::readLine (char *buf, int max)
{
  char *ptr;
  int count;
  int item;

  if (!ready ())
    return -1;

  --max;
  ptr = buf;
  count = 0;
  item = fgetc (fInDeviceFile);
  while ((item != EOF) && (count < max) && (item != '\n') && (item != '\0'))
  {
    *ptr = item;
    ++count;
    ++ptr;
      
    item =  fgetc (fInDeviceFile);
  }
    
  *ptr = '\0';
    
  return count;
}



int TModuleServer::readItem (char *buf, int max)
{
  char *ptr;
  int count;
  int item;

  --max;
  ptr = buf;
  count = 0;
  item = fgetc (fInDeviceFile);
  while ((item != EOF) && (count < max) && (item != '\0') && (item != '\n') && (item != ' '))
  {
    *ptr = item;
    ++count;
    ++ptr;
      
    item =  fgetc (fInDeviceFile);
  }
    
  *ptr = '\0';
    
  return count;
}
