/***************************************************************************
                          TCacheManager.h  -  description
                             -------------------
    begin                : Tue Aug 27 2002
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

#ifndef __TCACHEMANAGER_H__
#define __TCACHEMANAGER_H__

#include <string>
#include <vector>
#include <fstream>

// forward reference
class TCacheSystem;
class TDirectory;
class TFileData;



class TCacheManager
{
  private:
    typedef std::vector<TCacheSystem *> CacheVector;
    typedef CacheVector::size_type	size_type;

  public:
    TCacheManager (const std::string & root) : fRoot (root) {}
    ~TCacheManager ();

    bool LoadTree (TDirectory &) const;

    int AddCache (TCacheSystem *);
    TCacheSystem * GetSystem (size_type);

//    bool HasFile (const std::string &) const;
//    bool HasFile (const std::string &, size_type) const;

    bool HaveFile (const std::string &) const;
    bool HaveFile (size_type, const std::string &) const;
    bool HaveDirectory (size_type, const std::string &) const;
    bool CreateDirectory (size_type, const std::string &, int) const;
    bool CreateFile (size_type, const std::string &, int) const;
    bool DeleteDirectory (size_type, const std::string &) const;
    bool DeleteFile (size_type, const std::string &) const;

    bool FileAttribute (size_type, const std::string &, TFileData &) const;

    std::ifstream * In (size_type, const std::string &, int = 0) const;
    std::ofstream * Out (size_type, const std::string &, int = 0) const;

  private:
    CacheVector fCaches;
    std::string fRoot;
};



#endif
