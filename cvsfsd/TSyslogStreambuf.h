/***************************************************************************
                          TSyslogStreambuf.h  -  description
                             -------------------
    begin                : Fri Aug 16 2002
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

#ifndef __TSYSLOGSTREAMBUF_H__
#define __TSYSLOGSTREAMBUF_H__

#include <string>
#ifdef HAVE_STREAMBUF_H
#include <streambuf.h>
#else
#include <streambuf>
#endif

// forward reference
class TSyslogFile;


class TSyslogStreambuf : public std::streambuf
{
  public:
    TSyslogStreambuf (TSyslogFile &, int level);
    ~TSyslogStreambuf ();

    void Enable () { fEnable = true; }
    void Disable () { fEnable = false; }

  protected:
    virtual int overflow (int);

  private:
    TSyslogFile	&fFile;
    int		fLevel;
    std::string	fLineBuffer;
    bool	fEnable;
};



#endif
