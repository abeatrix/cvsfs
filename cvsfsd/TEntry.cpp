/***************************************************************************
                          TEntry.cpp  -  description
                             -------------------
    begin                : Mon Aug 5 2002
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

#include "TEntry.h"



TEntry::~TEntry ()
{
}



std::ostream & TEntry::operator << (std::ostream & str) const
{
  streamData (str);

  return str;
}
