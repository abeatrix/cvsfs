/***************************************************************************
                          TSyslogFile.h  -  description
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

#ifndef __TSYSLOGFILE_H__
#define __TSYSLOGFILE_H__

#include <string>



class TSyslogFile
{
  public:
    TSyslogFile (const std::string &, int);
    ~TSyslogFile ();

    void reopen (const std::string &, int);

    void write (int, const std::string &) const;

  private:
    std::string	fId;
    int		fFacility;
};



#endif
