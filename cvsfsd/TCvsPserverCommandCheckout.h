/***************************************************************************
                          TCvsPserverCommandCheckout.h  -  description
                             -------------------
    begin                : Fri Sep 27 2002
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

#ifndef __TCVSPSERVERCOMMANDCHECKOUT_H__
#define __TCVSPSERVERCOMMANDCHECKOUT_H__

#include "TCvsPserverCommand.h"
#include <string>
#include <iostream>
#include "TFileData.h"



class TCvsPserverCommandCheckout : public TCvsPserverCommand
{
  public:
    TCvsPserverCommandCheckout (const std::string &, const std::string &,
				std::ostream *);
    virtual ~TCvsPserverCommandCheckout ();

    virtual bool execute (TCvsSessionPserver &);

    const TFileData & GetData () const { return fFileData; }

  protected:
    std::string		fPath;
    std::string		fVersion;
    std::ostream	*fFileStream;
    TFileData		fFileData;
    bool		fDataReceived;

    virtual bool processLine (TCvsSessionPserver &, const std::string &);
};



#endif
