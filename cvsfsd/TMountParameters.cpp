/***************************************************************************
                          TMountParameters.cpp  -  description
                             -------------------
    begin                : Mon Jun 11 22:57:32 CEST 2002
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "TMountParameters.h"



const std::string nullString = "";



TMountParameters::TMountParameters (const std::string & parmString)
{
  if (parmString.length () != 0)
  {
    std::string::size_type pos;
    std::string::size_type next;

    pos = 0;
    next = parmString.find (',');
    while (pos != std::string::npos)
    {
      std::string parm;
      std::string key;
      std::string value;
      std::string::size_type equal;

      if (next == std::string::npos)
        parm = parmString.substr (pos);
      else
        parm = parmString.substr (pos, next - pos);

      if ((equal = parm.find ('=')) == std::string::npos)
        key = parm;
      else
      {
        key = parm.substr (0, equal);
	value = parm.substr (equal + 1);
      }
      
      fParameters.insert (ValuePair (key, value));

      if (next != std::string::npos)
      {
        pos = next + 1;
        next = parmString.find (',', pos);
      }
      else
        pos = next;
    }
  }
}



TMountParameters::~TMountParameters ()
{
};



const std::string & TMountParameters::operator [] (const std::string & key) const
{
  MapType::const_iterator iter;
  
  iter = fParameters.find (key);
  if (iter == fParameters.end ())
    return nullString;
    
  return (*iter).second;
}
