/***************************************************************************
                          TFunctionServer.cpp  -  description
                             -------------------
    begin                : Thu Sep 5 05:13:12 CEST 2002
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

#include "TFunctionServer.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>



TFunctionServer::TFunctionServer ()
{
}



TFunctionServer::~TFunctionServer ()
{
  MapType::iterator iter;

  for (iter = fFunctions.begin (); iter != fFunctions.end (); ++iter)
    delete (*iter).second;
};



void TFunctionServer::AddFunction (const std::string & key, TFunction * function)
{
  fFunctions.insert (ValuePair (key, function));
}



bool TFunctionServer::Execute (const std::string & key, int argc, char * argv [])
{
  MapType::iterator iter;

  iter = fFunctions.find (key);
  if (iter == fFunctions.end ())
    return false;
    
  (*iter).second->DoIt (argc - 2, &(argv[2]));
    
  return true;
}
