/***************************************************************************
                          main.c  -  description
                             -------------------
    begin                : Mit Jun 14 18:32:49 CEST 2001
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include "TModuleServer.h"
#include "TModuleActionAttr.h"
#include "TModuleActionDir.h"
#include "TModuleActionGet.h"
#include "TModuleActionPut.h"
#include "TModuleActionMkdir.h"
#include "TModuleActionRmdir.h"
#include "TModuleActionMkfile.h"
#include "TModuleActionRmfile.h"
#include "TModuleActionTruncfile.h"
#include "TModuleActionMove.h"
#include "TModuleActionIoctl.h"
#include "TModuleActionSetattr.h"
#include "TModuleActionQuit.h"
#include "TCvsInterfacePserver.h"
#include "TSyslog.h"



void
program_version ()
{
  TSyslog::instance ()->info << "cvsfsd " << VERSION << std::endl;
}



int
main(int argc, char *argv[])
{
  char name[32];
  TSyslog *log = TSyslog::instance ("cvsfsd", TSyslog::Daemon);

  log->debug.Disable ();	// disable debug logging

  program_version ();

  if (argc == 1)
  {			// no arguments - start control daemon
    FILE *fControl;

    log->info << "Starting control daemon ..." << std::endl;

    fControl = fopen ("/dev/cvsfs/0", "r+");
    if (fControl == NULL)
    {
      log->error << "Can not open control device '/dev/cvsfs/0'" << std::endl;

      exit (1);
    }

    /* this loop should never be exited - except with a kill command */
    while (1 == 1)
    {
      char *ptr;
      int count;
      int item;
      int max;
      fd_set rset;

      FD_ZERO (&rset);
      FD_SET (fileno (fControl), &rset);

      if (select (fileno (fControl) + 1, &rset, NULL, NULL, NULL) < 0)
        break;		// something went wrong - exit the daemon

      max = sizeof (name) - 1;
      ptr = name;
      count = 0;
      item = fgetc (fControl);
      while ((item != EOF) && (count < max) && (item != '\n'))
      {
        *ptr = item;
        ++count;
        ++ptr;

        item =  fgetc (fControl);
      }

      *ptr = '\0';

      // id evaluated - start daemon
      if (strlen (name) > 0)
      {
        std::string command = argv[0];

        command += std::string (" ") + name + " &";

        system (command.c_str ());
      }
    }

    fclose (fControl);
  }
  else
  {			// argument given - start single daemon
    TModuleServer *server;
    int id;
    std::string idname = "cvsfsd:";

    idname += argv[1];
    log = TSyslog::instance (idname, TSyslog::Daemon);

    switch (*(argv[1]))
    {
      case 'i':
        server = new TModuleServer;
        break;

      default:
        id = atoi (argv[1]);

        if (id <= 0)
        {
          log->error << "Unsupported id '" << argv[1] << "' given - aborted" << std::endl;

	  exit (1);
        }

        sprintf (name, "/dev/cvsfs/%i", id);
  
        server = new TModuleServer (name);
    }

    // set up a list of supported actions sent by the kernel module
    server->AddAction ("quit", new TModuleActionQuit);
    server->AddAction ("ls", new TModuleActionDir);
    server->AddAction ("attr", new TModuleActionAttr);
    server->AddAction ("get", new TModuleActionGet);
    server->AddAction ("put", new TModuleActionPut);
    server->AddAction ("mkdir", new TModuleActionMkdir);
    server->AddAction ("rmdir", new TModuleActionRmdir);
    server->AddAction ("mkfile", new TModuleActionMkfile);
    server->AddAction ("rmfile", new TModuleActionRmfile);
    server->AddAction ("truncfile", new TModuleActionTruncfile);
    server->AddAction ("move", new TModuleActionMove);
    server->AddAction ("ioctl", new TModuleActionIoctl);
    server->AddAction ("setattr", new TModuleActionSetattr);

    log->info << "Starting mount daemon (" << argv[1] << ") ..." << std::endl;

    if (server->ready ())
    {
      TCvsInterfacePserver interface (server->parameters ());
			       
      if (!interface.Test ())
        log->error << "Can not login to server " << server->parameters ()["server"] << std::endl;

      server->run (interface);
    }
    else
      log->error << "Device " << name << " can not be opened." << std::endl;

    delete server;
  }

  return EXIT_SUCCESS;
}
