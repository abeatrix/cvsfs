/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sat Aug 24 19:22:44 CEST 2002
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

#include <iostream>
#include "TFunctionServer.h"
#include "TFunctionVersion.h"



void
program_version ()
{
  std::cout << "cvsfsctl " << VERSION << std::endl;
}



void
help ()
{
  program_version ();
  
  std::cout << std::endl
            << "usage: cvsctl function [options]" << std::endl
	    << "  functions are:" << std::endl
	    << "    version <filename>" << std::endl
	    << std::endl;
/*	    
  printf ("\n");
  printf ("usage: cvsctl function [options]\n");
  printf ("  --server server\n");
  printf ("  -s server         cvs-pserver\n");
  printf ("  --root root\n");
  printf ("  -r root           cvs root directory\n");
  printf ("  --project project\n");
  printf ("  -m project        module name\n");
  printf ("  --user user\n");
  printf ("  -u user           user account at cvs server (anonymous if not given)\n");
  printf ("  --password password\n");
  printf ("  -p password       password at cvs server (default is <none>)\n");
  printf ("  --uid uid\n");
  printf ("  -i uid            user id of the remote files (default is current uid)\n");
  printf ("  --gid gid\n");
  printf ("  -g gid            group id of the remote files (default is current uid)\n");
  printf ("  --filemask mask\n");
  printf ("  -f mask           file mask of the remote files (default is current umask)\n");
  printf ("  --dirmask mask\n");
  printf ("  -d mask           file mask of the remote directories (default is current umask)\n");
  printf ("  --help\n");
  printf ("  -h                print this help screen\n");
  printf ("  --version         print the program version\n\n");
*/
}


/*
int
parse_args (int argc, char *argv[], char **mount_point,
            char **data, char **share, int id_change_allowed)
{
  static struct option options[] = {
                                     { "server",   required_argument, 0, 's'},
                                     { "root",     required_argument, 0, 'r'},
				     { "project",  required_argument, 0, 'm'},
				     { "user",     required_argument, 0, 'u'},
				     { "password", required_argument, 0, 'p'},
				     { "uid",      required_argument, 0, 'i'},
				     { "gid",      required_argument, 0, 'g'},
				     { "filemask", required_argument, 0, 'f'},
				     { "dirmask",  required_argument, 0, 'd'},
				     { "help",     no_argument,	      0, 'h'},
				     { "version",  no_argument,	      0, 'v'},
				     { 0,          no_argument,       0, 0 }
				   };
  int opt;
  int index = 0;
  int server_found = 0;
  int module_found = 0;
  int uid_found = 0;
  int gid_found = 0;
  int fmask_found = 0;
  int dmask_found = 0;
  char *server;
  char *module;
  char *help;

  *data = malloc (1);
  **data = '\0';

  while ((opt = getopt_long (argc, argv,
                             "-s:r:m:u:p:i:g:f:d:h",
			     options, &index)) != -1)
  {
    switch (opt)
    {
      case 1:
        if (*mount_point != NULL)
	{
          fprintf (stderr, "mount point ('%s') is doubly defined\n", optarg);
	
	  return -1;
	}
	
	*mount_point = fullpath (optarg);
        add_option (data, "mount", *mount_point);
        break;
	
      case 's':
	if (isdigit (*optarg) == 0)
	{
	  char *ip = evaluate_ip (optarg);
	  if (ip == NULL)
	    return -1;
	
          add_option (data, "server", ip);
	  free (ip);
	}
	else
          add_option (data, "server", optarg);
	
        server = strdup (optarg);
	
	server_found = 1;
        break;
	
      case 'r':
        add_option (data, "cvsroot", optarg);
	break;
	
      case 'm':
        add_option (data, "module", optarg);
	module = strdup (optarg);
	module_found = 1;
        break;
	
      case 'u':
        add_option (data, "user", optarg);
        break;
	
      case 'p':
        add_option (data, "password", optarg);
        break;
	
      case 'i':
        if (id_change_allowed == 0)
          if ((help = convert_uid (optarg)) != NULL)
	  {
            add_option (data, "uid", help);
	    uid_found = 1;
	  }
	  else
	  {
            fprintf (stderr, "user id '%s' does not exist\n", optarg);
	
	    return -1;
	  }
	else
          fprintf (stderr, "ownership modification (user id) is only allowed as root\n");
        break;
	
      case 'g':
        if (id_change_allowed == 0)
          if ((help = convert_gid (optarg)) != NULL)
  	  {
            add_option (data, "gid", help);
	    gid_found = 1;
	  }
	  else
	  {
            fprintf (stderr, "group id '%s' does not exist\n", optarg);

	    return -1;
	  }
	else
          fprintf (stderr, "ownership modification (group id) is only allowed as root\n");
        break;
	
      case 'f':
        add_option (data, "fmask", convert_umask (optarg));
	fmask_found = 1;
        break;
	
      case 'd':
        add_option (data, "dmask", convert_umask (optarg));
	dmask_found = 1;
        break;
	
      case 'h':
	return -1;
	
      case 'v':
        program_version ();
	exit (0);

      default:
        return -1;
    }
  }

  if (server_found == 0)
  {
    fprintf (stderr, "No cvs pserver given\n");
    return -1;
  }

  if (module_found == 0)
  {
    fprintf (stderr, "No cvs module given\n");
    return -1;
  }

  if (uid_found == 0)
    add_option (data, "uid", current_uid ());

  if (gid_found == 0)
    add_option (data, "gid", current_gid ());

  if (fmask_found == 0)
    add_option (data, "fmask", current_umask ());

  if (dmask_found == 0)
    add_option (data, "dmask", current_dmask ());

  *share = malloc (strlen (server) + strlen (module) + 4);

  strcpy (*share, "//");
  strcat (*share, server);
  strcat (*share, "/");
  strcat (*share, module);

  free (server);
  free (module);

  return 0;
}
*/


int main(int argc, char *argv[])
{
  TFunctionServer supportedFunctions;

  if (argc < 2)
  {
    help ();
    
    exit (1);
  }

  // fill the server with supported functions
  supportedFunctions.AddFunction ("version", new TFunctionVersion ());

  // execute the requested function
  if (!supportedFunctions.Execute (argv[1], argc, argv))
    exit (1);

  return 0;
}
