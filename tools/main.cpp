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
#include "TFunctionCheckout.h"
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
	    << "    checkout <filename> [<version>]" << std::endl
	    << std::endl;
}



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
  supportedFunctions.AddFunction ("checkout", new TFunctionCheckout ());

  // execute the requested function
  if (!supportedFunctions.Execute (argv[1], argc, argv))
    exit (1);

  return 0;
}
