/***************************************************************************
                          TCacheSystemVersioned.h  -  description
                             -------------------
    begin                : Thu Aug 29 2002
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

#ifndef __TCACHESYSTEMVERSIONED_H__
#define __TCACHESYSTEMVERSIONED_H__

#include "TCacheSystem.h"



class TCacheSystemVersioned : public TCacheSystem
{
  public:
    TCacheSystemVersioned (const std::string &);

    virtual bool LoadTree (TDirectory &) const;

    virtual bool HaveFile (const std::string &) const;
    virtual bool HaveDirectory (const std::string &) const;
    virtual bool CreateDirectory (const std::string &, int) const;
    virtual bool CreateFile (const std::string &, int) const;
    virtual bool DeleteDirectory (const std::string &) const;
    virtual bool DeleteFile (const std::string &) const;
    virtual bool Move (const std::string &, const TCacheSystem &, const std::string &) const;

    virtual bool FileAttribute (const std::string &, TFileData &) const;

    virtual std::ifstream * In (const std::string &, int = 0) const;
    virtual std::ofstream * Out (const std::string &, int = 0) const;
};



#endif
