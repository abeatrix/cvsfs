/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sun Jun 17 18:32:49 CEST 2001
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

/* actually hardcoded - must be configurable later ! */
#define BINDIR "/usr/bin"



void help ()
{
  printf ("\n");
  printf ("usage: mount.cvsfs service mount-point [-o options, ...]\n\n");
  printf ("Options:\n");
  printf ("  user=<arg>       cvs username (default: anonymous)\n");
  printf ("  password=<arg>   cvs password (default: none)\n");
  printf ("  cvsroot=<arg>    cvs root directory (default: /cvsroot)\n\n");
  printf ("This command is designed to be run from within /bin/mount by giving\n");
  printf ("the option '-t cvsfs'. For example:\n");
  printf ("  mount -t cvsfs -o=username=cvsuser,password=heck //server/cvsfs /cvsfs\n");
}



int parse_args (int argc, char *argv[], char **user, char **password, char **cvsroot)
{
  int opt;
  char *opts;
  char *opteq;

  *user = NULL;
  *password = NULL;
  *cvsroot = NULL;

  opt = getopt (argc, argv, "o:");
  if (opt == 'o')
  {
    for (opts = strtok (optarg, ","); opts; opts = strtok (NULL, ","))
    {
      if ((opteq = strchr (opts, '=')) != NULL)
      {
        *opteq = '\0';
        ++opteq;
	
        if (strcmp ("user", opts) == 0)
          *user = strdup (opteq);
        else
          if (strcmp ("password", opts) == 0)
            *password = strdup (opteq);
          else
            if (strcmp ("cvsroot", opts) == 0)
              *cvsroot = strdup (opteq);
            else
	    {
	      fprintf (stderr, "invalid option ('%s')\n", opts);
	      return -1;
	    }
      }
    }
  }

  return 0;
}



void init_mount (char *server, char *module, char *mountpoint,
                 char *user, char *password, char *cvsroot)
{
  char *args[20];
  int status;
  int i;
  
  memset (args, 0, sizeof (args));
  
  i = 0;
  args[i++] = "cvsmnt";
  args[i++] = mountpoint;
  args[i++] = "-s";
  args[i++] = &server[2];
  args[i++] = "-m";
  args[i++] = module;
  
  if (user != NULL)
  {
    args[i++] = "-u";
    args[i++] = user;
  }
  
  if (password != NULL)
  {
    args[i++] = "-p";
    args[i++] = password;
  }
  
  if (cvsroot != NULL)
  {
    args[i++] = "-r";
    args[i++] = cvsroot;
  }
  
  if (fork () == 0)
  {
/*    if (file_exist (BINDIR "/cvsmnt", NULL))
    {
      execv (BINDIR "/cvsmnt", args);
      fprintf (stderr, "execv of %s failed. Error was %s\n",
               BINDIR "/cvsmnt", strerror (errno));
    }
    else
    {
*/      execvp ("cvsmnt", args);
      fprintf (stderr, "execvp of cvsmnt failed. Error was %s\n", strerror (errno));
/*    } */
    
    exit (1);
  }
  
  if (waitpid (-1, &status, 0) == -1)
  {
    fprintf (stderr, "waitpid failed. Error was %s\n", strerror (errno));
    exit (1);
  }
  
  if (WIFEXITED (status) && WEXITSTATUS (status) != 0)
    fprintf (stderr, "cvsmnt failed: %d\n", WEXITSTATUS (status));
}



int main(int argc, char *argv[])
{
  char *mountpoint;
  char *server;
  char *module;
  char *user;
  char *password;
  char *cvsroot;
  
  if ((argc < 2) || (argv[1][0] == '-'))
  {
    help ();
    exit (1);
  }

  server = strdup (argv[1]);
  mountpoint = strdup (argv[2]);
  
  if ((strlen (server) < 5) ||
      (*server != '/') || (server[1] != '/') ||
      ((module = strchr (&server[2], '/')) == NULL))
  {
    fprintf (stderr, "parameter service ('%s') is invalid\n", server);
    exit (1);
  }
  
  *module = '\0';
  ++module;
  
  argv += 2;
  argc -= 2;
  
  if (parse_args (argc, argv, &user, &password, &cvsroot) != 0)
  {
    help ();
    return -1;
  }

  init_mount (server, module, mountpoint, user, password, cvsroot);  
  
  return EXIT_SUCCESS;
}
