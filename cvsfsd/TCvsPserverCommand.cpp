/***************************************************************************
                          TCvsPserverCommand.cpp  -  description
                             -------------------
    begin                : Thu Sep 26 2002
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

#include "TCvsPserverCommand.h"

#include <sys/stat.h>
#include "TSyslog.h"
#include "XPserverTimeout.h"
#include "TCvsSessionPserver.h"



TCvsPserverCommand::~TCvsPserverCommand ()
{
}



bool TCvsPserverCommand::processData (TCvsSessionPserver & session)
{
  TSyslog *log = TSyslog::instance ();

  try
  {
    std::string response;

    // now analyze the response we get from the CVS server
    response = session.ReadLine ();
    while (response != "ok")
    {
      if (response == "error")
        return false;

      log->debug << "Response: " << response << std::endl;

      if (!processLine (session, response))
        return false;

      response = session.ReadLine ();
    }
  }
  catch (XPserverTimeout e)
  {
    log->error << "processData: " << e << std::endl;

    return false;
  }

  return true;
}



int TCvsPserverCommand::CvsAttr2SysAttr (const std::string & line)
{
  const int bits[9] =  {S_IRUSR, S_IWUSR, S_IXUSR,
			S_IRGRP, S_IWGRP, S_IXGRP,
			S_IROTH, S_IWOTH, S_IXOTH};
  std::string::const_iterator iter = line.begin ();
  int stage = 0;
  int attr = 0;
  bool doStage = true;

  do
  {
    if (doStage)
    {
      switch (*iter)
      {
        case 'u':	stage = 0;		break;
        case 'g':	stage = 3;		break;
        case 'o':	stage = 6;		break;
        case '=':	doStage = false;	break;
        default:	return attr;
      }
    }
    else
    {
      switch (*iter)
      {
        case 'r':	attr |= bits[stage];	break;
        case 'w':	attr |= bits[stage+1];	break;
        case 'x':	attr |= bits[stage+2];	break;
	case ',':	doStage = true;		break;
	default:	return attr;
      }
    }

    ++iter;
  } while (iter != line.end ());

  return attr;
}



void TCvsPserverCommand::SysAttr2CvsAttr (int attr, std::string & line)
{
  line = "u=";
  if (attr & S_IRUSR)
    line += "r";
  if (attr & S_IWUSR)
    line += "w";
  if (attr & S_IXUSR)
    line += "x";

  line = ",g=";
  if (attr & S_IRGRP)
    line += "r";
  if (attr & S_IWGRP)
    line += "w";
  if (attr & S_IXGRP)
    line += "x";

  line = "o=";
  if (attr & S_IROTH)
    line += "r";
  if (attr & S_IWOTH)
    line += "w";
  if (attr & S_IXOTH)
    line += "x";
}
