/***************************************************************************
                          TCvsSessionPserver.h  -  description
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

#ifndef __TCVSSESSIONPSERVER_H__
#define __TCVSSESSIONPSERVER_H__

#include <string>

#include "TCvsSession.h"

// forward references
class TConnectedSocket;
class TCvsConnection;
class TCvsPserverCommand;



class TCvsSessionPserver : public TCvsSession
{
  public:
    TCvsSessionPserver (TConnectedSocket *, TCvsConnection *);
    ~TCvsSessionPserver ();

    virtual bool Test ();

    virtual bool SendRdiff (const std::string &) const;
//    virtual bool SendCo (const std::string &, const std::string &) const;
    virtual bool SendCiInit (const std::string &, const std::string &,
                             const std::string &, const std::string &) const;
    virtual bool SendCiExit (const std::string &) const;

    virtual bool ExecuteCommand (TCvsPserverCommand &);

    virtual std::string ReadLine () const;
    virtual int ReadRaw (char *, int) const;

    bool execute (const std::string &) const;
    bool execute (const char *, int) const;

  private:
    bool DoLogin (bool) const;

    TConnectedSocket	*fSocket;
    std::string		fEncryptedPassword;
    bool		fLoggedIn;
};



#endif
