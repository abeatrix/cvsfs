/***************************************************************************
                          TCvsSession.h  -  description
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

#ifndef __TCVSSESSION_H__
#define __TCVSSESSION_H__

#include <string>

// forward reference
class TCvsConnection;



class TCvsSession
{
  public:
    TCvsSession (TCvsConnection *);
    virtual ~TCvsSession ();

    virtual bool Test () = 0;

    virtual bool SendRdiff (const std::string &) const = 0;
    virtual bool SendCo (const std::string &, const std::string &) const = 0;

    virtual std::string ReadLine () const = 0;
    virtual int ReadRaw (char *, int) const = 0;

  protected:
    TCvsConnection	*fConnection;
};



#endif
