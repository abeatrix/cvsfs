/***************************************************************************
                          TCvsSessionPserver.cpp  -  description
                             -------------------
    begin                : Sat Aug 10 2002
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

#include "TCvsSessionPserver.h"

#include "TConnectedSocket.h"
#include "TCvsConnection.h"
#include "TCvsPserverCommand.h"
#include "TSyslog.h"
#include "XPserverTimeout.h"



static unsigned char shifts[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  114,120, 53, 79, 96,109, 72,108, 70, 64, 76, 67,116, 74, 68, 87,
  111, 52, 75,119, 49, 34, 82, 81, 95, 65,112, 86,118,110,122,105,
   41, 57, 83, 43, 46,102, 40, 89, 38,103, 45, 50, 42,123, 91, 35,
  125, 55, 54, 66,124,126, 59, 47, 92, 71,115, 78, 88,107,106, 56,
   36,121,117,104,101,100, 69, 73, 99, 63, 94, 93, 39, 37, 61, 48,
   58,113, 32, 90, 44, 98, 60, 51, 33, 97, 62, 77, 84, 80, 85,223,
  225,216,187,166,229,189,222,188,141,249,148,200,184,136,248,190,
  199,170,181,204,138,232,218,183,255,234,220,247,213,203,226,193,
  174,172,228,252,217,201,131,230,197,211,145,238,161,179,160,212,
  207,221,254,173,202,146,224,151,140,196,205,130,135,133,143,246,
  192,159,244,239,185,168,215,144,139,165,180,157,147,186,214,176,
  227,231,219,169,175,156,206,198,129,164,150,210,154,177,134,127,
  182,128,158,208,162,132,167,209,149,241,153,251,237,236,171,195,
  243,233,253,240,194,250,191,155,142,137,245,235,163,242,178,152
};

const char *beginAuthRequest	= "BEGIN AUTH REQUEST";
const char *beginVerifyRequest	= "BEGIN VERIFICATION REQUEST";
const char *endAuthRequest	= "END AUTH REQUEST";
const char *endVerifyRequest	= "END VERIFICATION REQUEST";
const char *validResponses      = "Valid-responses ok error Valid-requests Checked-in New-entry Checksum Copy-file Updated Created Update-existing Merged Patched Rcs-diff Mode Mod-time Removed Remove-entry Set-static-directory Clear-static-directory Set-sticky Clear-sticky Template  Set-checkin-prog Set-update-prog Notified Module-expansion Wrapper-rcsOption M  Mbinary E F MT";



TCvsSessionPserver::TCvsSessionPserver (TConnectedSocket * conn,
					TCvsConnection *connection)
: TCvsSession (connection), fSocket (conn), fLoggedIn (false)
{
  std::string::const_iterator iter;

  // encrypt password
  fEncryptedPassword = "A";

  for (iter = connection->GetPassword ().begin ();
       iter != connection->GetPassword ().end (); ++iter)
    fEncryptedPassword += shifts[(unsigned char) (*iter)];
}



TCvsSessionPserver::~TCvsSessionPserver ()
{
  delete fSocket;
}



bool TCvsSessionPserver::Test ()
{
  return DoLogin (false);
}



bool TCvsSessionPserver::ExecuteCommand (TCvsPserverCommand & command)
{
  if (!DoLogin (true))
    return false;

  return command.execute (*this);
}



bool TCvsSessionPserver::DoLogin (bool login) const
{
  TSyslog *log = TSyslog::instance ();
  bool ret;

  if (fLoggedIn)
    return true;

  if (login)
    ret = execute (beginAuthRequest);
  else
    ret = execute (beginVerifyRequest);

  if (!ret)
    return false;

  if (!execute (fConnection->GetRoot ()))
    return false;

  if (!execute (fConnection->GetUser ()))
    return false;

  if (!execute (fEncryptedPassword))
    return false;

  if (login)
    ret = execute (endAuthRequest);
  else
    ret = execute (endVerifyRequest);

  if (!ret)
    return false;

  std::string response;

  try
  {
    response = ReadLine ();
  }
  catch (XPserverTimeout e)
  {
    log->error << "LoadAttribute: " << e << std::endl;

    return false;
  }

  if (response == "I LOVE YOU")
  {
    if (login)
    {
      if (!execute ("Root " + fConnection->GetRoot ()))
        return false;

      if (!execute (validResponses))
        return false;

      if (!execute ("UseUnchanged"))
        return false;
    }

    const_cast<TCvsSessionPserver *> (this)->fLoggedIn = true;

    return true;
  }

  return false;
}



bool TCvsSessionPserver::SendRdiff (const std::string & base) const
{
  std::string buf;

  if (!DoLogin (true))
    return false;

  if (!execute ("Argument -s"))
    return false;

  if (!execute ("Argument -r"))
    return false;

  if (!execute ("Argument 0"))
    return false;

//  if (!execute ("Argument -s"))
//    return false;

  if (fConnection->GetProject () == ".")	// root ?
  {
    if (base.length () == 0)
      buf = ".";
    else
      buf = base;
  }
  else
  {
    buf = fConnection->GetProject ();
    if (base.length () > 0)
      buf += "/" + base;
  }

  if (!execute ("Argument " + buf))
    return false;

  return execute ("rdiff");
}


/*
bool TCvsSessionPserver::SendCo (const std::string & path,
				 const std::string & version) const
{
  if (!DoLogin (true))
    return false;

  if (!execute ("Argument -N"))
    return false;

  if (!execute ("Argument -P"))
    return false;

  if (!execute ("Argument -r"))
    return false;

  if (!execute ("Argument " + version))
    return false;

  if (!execute ("Argument " + path))
    return false;

  if (!execute ("Directory ."))
    return false;

  if (!execute (fConnection->GetRoot ()))
    return false;

  return execute ("co");
}
*/


bool TCvsSessionPserver::SendCiInit (const std::string & path,
				     const std::string & name,
				     const std::string & version,
				     const std::string & commitinfo) const
{
  if (!DoLogin (true))
    return false;

  // send commit info
  if (!execute ("Argument -m"))
    return false;

  std::string::size_type pos = commitinfo.find ('\n');
  if (pos == std::string::npos)
  {
    if (!execute ("Argument " + commitinfo))
      return false;
  }
  else
  {
    std::string part = commitinfo;
    std::string next = commitinfo;
    part.erase (pos);

    if (!execute ("Argument " + part))
      return false;

    next.erase (0, pos + 1);
    while ((pos = next.find ('\n')) != std::string::npos)
    {
      part = next;
      part.erase (pos);
      next.erase (0, pos + 1);

      if (!execute ("Argumentx " + part))
        return false;
    }
    if (!execute ("Argumentx " + next))
      return false;
  }

  if (!execute ("Directory ."))
    return false;

  if (!execute (fConnection->GetRoot () + path))
    return false;

  if (!execute ("Entry /" + name + "/" + version + "///"))
    return false;

  return execute ("Modified " + name);
}



bool TCvsSessionPserver::SendCiExit (const std::string & name) const
{
  if (!execute ("Argument " + name))
    return false;

  return execute ("ci");
}



bool TCvsSessionPserver::execute (const std::string & s) const
{
  return execute ((s + '\012').c_str (), s.length () + 1);
}



bool TCvsSessionPserver::execute (const char * s, int size) const
{
  if (fSocket->Send (s, size) == size)
    return true;

  return false;
}



std::string TCvsSessionPserver::ReadLine () const
{
  std::string buf;
  char item;
  int count;

  buf = "";
  while (((count = fSocket->Recv (&item, 1)) == 1) && (item != '\012') && (item != '\015'))
    buf += item;

  if (count == -1)
    throw XPserverTimeout ();	// throw error due to timeout

  return buf;
}



int TCvsSessionPserver::ReadRaw (char * buf, int size) const
{
  return fSocket->Recv (buf, size);
}
