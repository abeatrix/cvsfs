/***************************************************************************
                          TFunctionServer.h  -  description
                             -------------------
    begin                : Thu Sep 5 05:09:49 CEST 2002
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

#ifndef __TFUNCTIONSERVER_H__
#define __TFUNCTIONSERVER_H__

#include <string>
#include <map>
#include "TFunction.h"



class TFunctionServer
{
  public:
    TFunctionServer ();
    ~TFunctionServer ();

    void AddFunction (const std::string &, TFunction *);

    bool Execute (const std::string &, int, char * []);

  private:
    typedef std::map<std::string, TFunction *> MapType;
    typedef MapType::value_type ValuePair;

    MapType fFunctions;
};



#endif
