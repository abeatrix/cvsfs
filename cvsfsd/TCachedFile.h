/***************************************************************************
                          TCachedFile.h  -  description
                             -------------------
    begin                : Mon Aug 12 2002
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

#ifndef __TCACHEDFILE_H__
#define __TCACHEDFILE_H__

#include <string>
#include <iostream>

// forward references
class TCache;



class TCachedFile
{
  public:
    TCachedFile (const std::string &, const std::string &);

    bool HaveFile () const;
    std::ostream * OpenForWrite () const;
    std::ostream * OpenForWrite (int) const;
    std::istream * OpenForRead () const;

    int ReadFile (char *, long long, int) const;

  private:
    std::string	fDir;
    std::string	fName;

    bool DirExist () const;
    bool FileExist () const;
    bool CreateDir () const;
};



#endif
