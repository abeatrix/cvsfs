/***************************************************************************
                          TCvsConnection.cpp  -  description
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "TCvsConnection.h"

#include "TMountParameters.h"


TCvsConnection::TCvsConnection (const TMountParameters & parameters)
: fServer (parameters["server"]), fUser (parameters["user"]),
  fPass (parameters["password"]), fRoot (parameters["cvsroot"]),
  fProject (parameters["module"]), fMountPoint (parameters["mount"])
{
  if (fUser.length () == 0)
    fUser = "anonymous";

  if (fRoot.length () == 0)
    fRoot = "/cvsroot";

  if ((fProject.length () == 0) || (fProject == "/"))
    fProject = ".";
}



TCvsConnection::~TCvsConnection ()
{
}
