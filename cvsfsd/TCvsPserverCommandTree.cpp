/***************************************************************************
                          TCvsPserverCommandTree.cpp  -  description
                             -------------------
    begin                : Wed Oct 2 2002
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

#include "TCvsPserverCommandTree.h"

#include "TCvsConnection.h"
#include "TCvsSessionPserver.h"
#include "TDirectory.h"
#include "TVersionedFile.h"
#include "TSyslog.h"



TCvsPserverCommandTree::TCvsPserverCommandTree (const std::string & root,
						TDirectory & tree)
: TCvsPserverCommand (), fRootPath (root), fTree (tree), fOldTree (0)
{
}



TCvsPserverCommandTree::~TCvsPserverCommandTree ()
{
  delete fOldTree;
}



bool TCvsPserverCommandTree::execute (TCvsSessionPserver & session)
{
  std::string buf;

  if (!session.execute ("Argument -s"))
    return false;

  if (!session.execute ("Argument -r"))
    return false;

  if (!session.execute ("Argument 0"))
    return false;

//  if (!session.execute ("Argument -s"))
//    return false;

  if (session.GetConnection().GetProject () == ".")	// root ?
  {
    if (fRootPath.length () == 0)
      buf = ".";
    else
      buf = fRootPath;
  }
  else
  {
    buf = session.GetConnection().GetProject ();
    if (fRootPath.length () > 0)
      buf += "/" + fRootPath;
  }

  if (!session.execute ("Argument " + buf))
    return false;

  if (!session.execute ("rdiff"))
    return false;

  fCurrentDir = 0;
  fCurrentOldDir = 0;
  fOldTree = new TDirectory (fTree);

  TEntry *entry;
  while ((entry = fTree.GetEntry (0)) != 0)
    fTree.RemoveEntry (entry);

  bool result = processData (session);

  delete fOldTree;
  fOldTree = 0;

  return result;
}



bool TCvsPserverCommandTree::processLine (TCvsSessionPserver & session,
					  const std::string & line)
{
  const char FileMarker[]	= "M File ";
  const char DirMarker[]	= "E cvs server: Diffing ";
  const char VersionMarker[]	= " is new; current revision ";
  const int FileMarkerSize	= sizeof (FileMarker) - 1;
  const int DirMarkerSize	= sizeof (DirMarker) - 1;
  const int VersionMarkerSize	= sizeof (VersionMarker) - 1;
  TSyslog *log = TSyslog::instance ();

  // is it a file ?
  if (std::string::traits_type::compare (line.data (), FileMarker, FileMarkerSize) == 0)
  {
    if (!fCurrentDir)
      return true;	// skip if no directory detected before

    std::string name = line.substr (FileMarkerSize);
    std::string version;

    std::string::size_type pos = name.find (VersionMarker);
    if (pos == std::string::npos)
      version = "<unknown>";
    else
    {
      version = name.substr (pos + VersionMarkerSize);
      name.erase (pos);
    }

    if (fCurrentPath.length () != 0)
      name.erase (0, fCurrentPath.length () + 1);	// kill path part

    log->debug << "  Treat file: " << name << " - version " << version << std::endl;

    // now we have the data - insert it into the tree
    TVersionedFile *item = 0;

    if (fCurrentOldDir)
    {
      TVersionedFile *entry = dynamic_cast<TVersionedFile *> (fCurrentOldDir->FindEntry (name));
      if (entry)
        item = new TVersionedFile (*entry);
    }

    if (!item)
    {
      item = new TVersionedFile (name, version);
      item->SetLayer (fTree.GetLayer ());
    }
    else
      item->SetHeadVersion (version);

    fCurrentDir->AddEntry (item);

    return true;
  }

  // is it a directory ?
  if (std::string::traits_type::compare (line.data (), DirMarker, DirMarkerSize) == 0)
  {
    fCurrentPath = line.substr (DirMarkerSize);
    if (fCurrentPath == ".")
      fCurrentPath = "";

    log->debug << "Treat directory: /" << fCurrentPath << std::endl;

    // now we have the data - insert it into the tree
    fCurrentDir = &fTree;
    std::string path = fCurrentPath;
    while (path.length () != 0)
    {
      std::string dir = path;
      std::string::size_type pos = dir.find ('/');
      if (pos == std::string::npos)
        path = "";
      else
      {
        path.erase (0, pos + 1);
        dir.erase (pos);
      }

      TDirectory *item = dynamic_cast<TDirectory *> (fCurrentDir->FindEntry (dir));
      if (!item)
      {
        item = new TDirectory (dir);
        item->SetLayer (fTree.GetLayer ());
        fCurrentDir->AddEntry (item);
      }

      fCurrentDir = item;
    }

    if (fCurrentPath.length () != 0)
      fCurrentOldDir = dynamic_cast<TDirectory *> (fOldTree->FindEntry (fCurrentPath));
    else
      fCurrentOldDir = fOldTree;

    return true;
  }

  return true;
}
