/***************************************************************************
                          TModuleAction.h  -  description
                             -------------------
    begin                : Sun Jun 9 18:32:49 CEST 2002
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

#ifndef __TMODULEACTION_H__
#define __TMODULEACTION_H__

#include <string>

// forward references
class TModuleServer;
class TCvsInterface;



class TModuleAction
{
  public:
    TModuleAction ();
    TModuleAction (const TModuleAction &);
    virtual ~TModuleAction ();

    void SetServer (TModuleServer *);

    virtual bool doit (TCvsInterface &) = 0;

  protected:
    int readLine (char *, int);
    int readItem (char *, int);
    int readData (char *, int);
    void writeDummy ();
    void writeData (const char *, int);
    void writeData (const std::string &);

  private:
    TModuleServer *fServer;
};



#endif
