/***************************************************************************
                          TCache.h  -  description
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

#ifndef __TCACHE_H__
#define __TCACHE_H__

#include <string>



class TCache
{
  // this class may not instantiated by itself !
  protected:
    TCache (const std::string &, bool = false);
    virtual ~TCache ();

  protected:
    bool MakeDir (const std::string &, int);
    int RemoveDir (const std::string &);
    bool TouchFile (const std::string &, int);

    std::string	fRoot;
    bool	fCleanOnExit;

//    bool DirExist (const std::string &);
//    bool FileExist (const std::string &);
    void GetPath (const std::string &, std::string &);
//    bool CreateDir (const std::string &);
};



#endif
