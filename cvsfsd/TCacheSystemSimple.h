/***************************************************************************
                          TCacheSystemSimple.h  -  description
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

#ifndef __TCACHESYSTEMSIMPLE_H__
#define __TCACHESYSTEMSIMPLE_H__

#include "TCacheSystem.h"



class TCacheSystemSimple : public TCacheSystem
{
  public:
    TCacheSystemSimple (const std::string &, bool);

    virtual bool LoadTree (TDirectory &) const;

    virtual bool HaveFile (const std::string &) const;
    virtual bool HaveDirectory (const std::string &) const;
    virtual bool CreateDirectory (const std::string &, int) const;
    virtual bool CreateFile (const std::string &, int) const;
    virtual bool DeleteDirectory (const std::string &) const;
    virtual bool DeleteFile (const std::string &) const;
    virtual bool Move (const std::string &, const TCacheSystem &, const std::string &) const;

    virtual bool FileAttribute (const std::string &, TFileData &) const;
    virtual bool SetAttribute (const std::string &, TFileData &) const;

    virtual std::ifstream * In (const std::string &, std::ios::openmode = std::ios::openmode(0)) const;
    virtual std::ofstream * Out (const std::string &, std::ios::openmode = std::ios::openmode(0)) const;
};



#endif
