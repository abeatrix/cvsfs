/***************************************************************************
                          TMountParameters.h  -  description
                             -------------------
    begin                : Mon Jun 11 22:54:48 CEST 2002
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

#ifndef __TMOUNTPARAMETERS_H__
#define __TMOUNTPARAMETERS_H__

#include <string>
#include <map>



class TMountParameters
{
  public:
    TMountParameters (const std::string &);
    ~TMountParameters ();

    const std::string & operator [] (const std::string &) const;

  private:
    typedef std::map<std::string, std::string> MapType;
    typedef MapType::value_type ValuePair;

    MapType fParameters;
};



#endif
