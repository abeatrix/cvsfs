/***************************************************************************
                          TCvsConnection.h  -  description
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

#ifndef __TCVSCONNECTION_H__
#define __TCVSCONNECTION_H__

#include <string>
#include "TSocket.h"

// forward references
class TCvsSession;
class TMountParameters;



class TCvsConnection
{
  public:
    TCvsConnection (const TMountParameters &);
    virtual ~TCvsConnection ();

    const std::string & GetServer () const	{ return fServer; }
    const std::string & GetUser () const	{ return fUser; }
    const std::string & GetPassword () const	{ return fPass; }
    const std::string & GetRoot () const	{ return fRoot; }
    const std::string & GetProject () const	{ return fProject; }
    const std::string & GetMountPoint () const	{ return fMountPoint; }

//    virtual TCvsSession *Open () = 0;

  protected:
    std::string fServer;
    std::string fUser;
    std::string fPass;
    std::string fRoot;
    std::string fProject;
    std::string fMountPoint;
};



#endif
