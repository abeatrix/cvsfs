/***************************************************************************
                          TCvsInterfacePserver.cpp  -  description
                             -------------------
    begin                : Fri Aug 9 2002
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "TCvsInterfacePserver.h"

#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include "TFile.h"
#include "TVersionedFile.h"
#include "TMountParameters.h"
#include "TCvsSessionPserver.h"
//#include "TCachedFile.h"
#include "TCacheSystemCheckedout.h"
#include "TCacheSystemSimple.h"
#include "TCacheSystemVersioned.h"
#include "TCvsPserverCommandTree.h"
#include "TCvsPserverCommandCheckout.h"
#include "TSyslog.h"
#include "XPserverTimeout.h"
#include "cvsfs_errno.h"



const std::string Slash = "/";


TCvsInterfacePserver::TCvsInterfacePserver (const TMountParameters & parms)
: TCvsInterface (), fConnection (parms), fCvsDir ("/"),
  fRemote (new TCacheSystemVersioned (Slash + fConnection.GetServer () + fConnection.GetRoot ())),
  fCheckedOut (new TCacheSystemCheckedout (fConnection.GetMountPoint ())),
  fLocal (new TCacheSystemSimple (fConnection.GetMountPoint (), false))
{
  // the sequence of adding is important !
  // the later added caches overlap the earlier ones.
  fCacheManager.AddCache (fRemote);
  fCacheManager.AddCache (fCheckedOut);
  fCacheManager.AddCache (fLocal);
}



bool TCvsInterfacePserver::Test ()
{
  bool result;

  TCvsSession *session = fConnection.Open ();
  if (session == 0)
    return false;

  result = session->Test ();

  delete session;

  return result;
}



const TEntry * TCvsInterfacePserver::GetEntry (const std::string & path, int index)
{
  if (!LoadTree ())
    return 0;

  std::string fullpath;

  if (path == "/")	// root ?
    fullpath = fConnection.GetProject ();
  else
    if (fConnection.GetProject () == ".")
    {
      fullpath = path;
      fullpath.erase (0, 1);
    }
    else
      fullpath = fConnection.GetProject () + path;

  const TEntry *entry = FindEntry (fRootDir, fullpath);
  if ((entry == 0) || (entry->isA () != TEntry::DirEntry))
    return 0;

  const TDirectory *dir = static_cast <const TDirectory *> (entry);

  return dir->GetEntry (index);
}



const TEntry * TCvsInterfacePserver::GetFullEntry (const std::string & path,
						   const std::string & version)
{
  if (!LoadTree ())
    return 0;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  TEntry *entry = FindEntry (fRootDir, fullpath);

  if (entry != 0)
  {
    // attributes not available - go to get them from cvs server
    if (entry->isA () == TEntry::VersionedFileEntry)
    {					// load attributes for a versioned file
      TVersionedFile *file = static_cast<TVersionedFile *> (entry);

      std::string realversion;
      if (version.length () == 0)
        realversion = file->GetHeadVersion ();
      else
        realversion = version;

      if (file->FindVersion (realversion) == 0)
      {
        // data for requested version not available - load it

        if (!LoadAttribute (fullpath, realversion, *file))
          return 0;

        return file->FindVersion (realversion);
      }
    }
    else
    {
      if (entry->isA () == TEntry::FileEntry)
      {					// load attributes for a file
        // attributes already set - nothing to do
      }
      else
        if (!(entry->ValidData ()))
        {				// load attributes for a directory
          TDirectory *dir = static_cast<TDirectory *> (entry);

          // a directory does not have attributes in cvs per se
          // so allocate a dummy file data record
          // this may change in future

          TFileData dummy;

          dummy.SetAttribute (S_IXUSR | S_IRUSR | S_IWUSR |
			      S_IXGRP | S_IRGRP | S_IWGRP |
			      S_IXOTH | S_IROTH | S_IWOTH);

          dir->SetData (dummy);
        }
    }
  }

  return entry;
}



const TEntry * TCvsInterfacePserver::MakeDirectory (const std::string & path,
						    const std::string & version,
						    int mode)
{
  if (!LoadTree ())
    return 0;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  TEntry *entry = FindEntry (fRootDir, fullpath);
  if (entry)
    return 0;

  if (!fLocal->CreateDirectory (fullpath, mode))
    return 0;

  TDirectory * dir = AllocateDir (fRootDir, fullpath);
  if (dir)
  {
    TFileData dummy;

    if (!fLocal->FileAttribute (fullpath, dummy))
      return 0;

    dir->SetData (dummy);
    dir->SetLayer (fLocal->GetLayer ());
  }

  return dir;
}



int TCvsInterfacePserver::RemoveDirectory (const std::string & path,
					   const std::string & version)
{
  if (!LoadTree ())
    return 0;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  TEntry *entry = FindEntry (fRootDir, fullpath);

  if (entry == 0)
    return ENOENT;

  if (entry->isA () != TEntry::DirEntry)
    return ENOTDIR;

  if (entry->GetLayer () != fLocal->GetLayer ())
    return EROFS;

  int retval = fLocal->DeleteDirectory (fullpath);

  if (retval == 0)
    RemoveEntry (fullpath);

  return retval;
}



const TEntry * TCvsInterfacePserver::MakeFile (const std::string & path,
					       const std::string & version,
					       int mode)
{
  if (!LoadTree ())
    return 0;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  std::string filename;

  TDirectory *dir = GetParentDirectory (fullpath, filename);
  if (!dir)
    return 0;

  // allocate the file in the working cache (with size = 0)
  if (!fLocal->CreateFile (fullpath, mode))
    return 0;

  TFile *file = new TFile (filename, "");
  if (!file)
    return 0;

  // add the file to the tree info
  TFileData dummy;

  if (!fLocal->FileAttribute (fullpath, dummy))
  {
    delete file;

    return 0;
  }

  file->SetData (dummy);
  file->SetLayer (fLocal->GetLayer ());

  dir->AddEntry (file);

  return file;
}



int TCvsInterfacePserver::RemoveFile (const std::string & path,
				      const std::string & version)
{
  if (!LoadTree ())
    return 0;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  std::string filename;

  TDirectory *dir = GetParentDirectory (fullpath, filename);
  if (!dir)
    return ENOENT;	// parent directory not found

  TEntry *entry = dir->FindEntry (filename);
  if (entry == 0)
    return ENOENT;

  if (entry->isA () == TEntry::DirEntry)
    return EISDIR;

  if (entry->GetLayer () == fRemote->GetLayer ())
    return EROFS;

  if (!fCacheManager.DeleteFile (entry->GetLayer (), fullpath))
    return ENOENT;

  // remove the file from directory listing
  dir->RemoveEntry (entry);

  // has the deleted file hidden a remote version ?
  entry = FindEntry (&fCvsDir, fullpath);
  if (entry)
    dir->AddEntry (entry->Clone ());	// then bring it back to front

  return 0;
}




int TCvsInterfacePserver::TruncateFile (const std::string & path,
				        const std::string & version)
{
  TSyslog *log = TSyslog::instance ();

  if (!LoadTree ())
    return EIO;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  log->debug << "TruncateFile begin: fullpath = " << fullpath << std::endl;

  std::string filename;

  TDirectory *dir = GetParentDirectory (fullpath, filename);
  if (!dir)
    return ENOENT;	// parent directory not found

  TEntry *entry = dir->FindEntry (filename);
  if (entry == 0)
    return ENOENT;

  if (entry->isA () == TEntry::DirEntry)
    return EISDIR;

  if (entry->GetLayer () == fRemote->GetLayer ())
    return EROFS;

  if (entry->isA () == TEntry::FileEntry)
  {
    TFile * file = static_cast<TFile *> (entry);

    if (!fCacheManager.CreateFile (entry->GetLayer (), fullpath,
                                   file->GetData ().GetAttribute ()))
      return ENOENT;

    TFileData dummy;

    if (!fCacheManager.FileAttribute (entry->GetLayer (), fullpath, dummy))
      return ENOENT;

    // update the file data
    file->SetData (dummy);
  }

  log->debug << "TruncateFile exit code 0" << std::endl;

  return 0;
}




int TCvsInterfacePserver::GetFile (const std::string & path,
				   const std::string & version,
				   long long start, int count,
				   char * buffer)
{
  if (!LoadTree ())
    return 0;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  TEntry *entry = FindEntry (fRootDir, fullpath);

  if (!entry)
    return -1;		// file not known

  if (entry->isA () != TEntry::DirEntry)
  {
    const TFile *f;

    if (entry->isA () == TEntry::VersionedFileEntry)
    {
      TVersionedFile *file = static_cast<TVersionedFile *> (entry);

//      if ((f = file->FindVersion (version)) == 0)	
//        if (!LoadAttribute (fullpath, version, *file))
//          return 0;

      // load file from cvs server if necessary and store it in cache
      if ((f = LoadFile (fullpath, version, *file)) == 0)
        return 0;
    }
    else		// standard locally edited file
      f = static_cast<TFile *> (entry);

    if (f->GetVersion ().length () != 0)
    {
      fullpath += "@@";
      fullpath += f->GetVersion ();
    }

    std::ifstream *in = fCacheManager.In (f->GetLayer (), fullpath);
    if (in)
    {
      if (start != 0)
        in->seekg (start, std::istream::beg);

      in->read (buffer, count);

      count = in->gcount ();

      delete in;

      return count;
    }
  }

  return -1;	// file not known
}



int TCvsInterfacePserver::PutFile (const std::string & path,
				   const std::string & version,
				   long long start, int count,
				   char * buffer)
{
  if (!LoadTree ())
    return 0;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  TEntry *entry = FindEntry (fRootDir, fullpath);

  if (entry == 0)
    return ENOENT;		// file not known

  if (entry->GetLayer () == fRemote->GetLayer ())
    return EROFS;	// CVS not direct writeable

  if (entry->isA () != TEntry::FileEntry)
    return EISDIR;

  TFile *changedFile = static_cast<TFile *> (entry);

  if (changedFile->GetVersion ().length () != 0)
  {
    fullpath += "@@";
    fullpath += changedFile->GetVersion ();
  }

  std::ostream * f = fCacheManager.Out (entry->GetLayer (), fullpath,
					std::ios::binary | std::ios::ate);
  if (!f)		// can not open file ?
  {
    delete f;

    return ENOENT;
  }

  f->seekp (start);
  f->write (buffer, count);

  delete f;

  // now the info in the file entry must be updated
  // Todo: handle this by an 'invalid'-flag in TEntry class
  TFileData newdata;

  if (!fCacheManager.FileAttribute (entry->GetLayer (), fullpath, newdata))
    return ENOENT;

  changedFile->SetData (newdata);

  return 0;	// everything ok
}



int TCvsInterfacePserver::Move (const std::string & oldpath,
				const std::string & version,
				const std::string & newpath)
{
  if (!LoadTree ())
    return EIO;

  if (oldpath == newpath)
    return 0;

  std::string old_fullpath;
  std::string new_fullpath;

  if (fConnection.GetProject () == ".")
  {
    old_fullpath = oldpath;
    old_fullpath.erase (0, 1);	// skip leading slash
    new_fullpath = newpath;
    new_fullpath.erase (0, 1);	// skip leading slash
  }
  else
  {
    old_fullpath = fConnection.GetProject () + oldpath;
    new_fullpath = fConnection.GetProject () + newpath;
  }

  std::string old_filename;

  TDirectory *olddir = GetParentDirectory (old_fullpath, old_filename);
  if (!olddir)
    return ENOENT;

  TEntry *oldentry = olddir->FindEntry (old_filename);
  if (!oldentry)
    return ENOENT;

  std::string new_filename;

  TDirectory *newdir = GetParentDirectory (new_fullpath, new_filename);
  if (!newdir)
    return ENOENT;		// parent directory of the destination must exist

  TCacheManager::size_type destLayer;
  TCacheManager::size_type sourceLayer = oldentry->GetLayer ();

  TEntry *newentry = newdir->FindEntry (new_filename);
  if (newentry)
    destLayer = newentry->GetLayer ();
  else
    destLayer = fLocal->GetLayer ();

  if ((oldentry->GetLayer () == fRemote->GetLayer ()) ||
      (destLayer == fRemote->GetLayer ()))
    return EROFS;

  if (!fCacheManager.Move (sourceLayer, old_fullpath, destLayer, new_fullpath))
    return errno;

  // trim the directory listing along the actions before
  if (!newentry)
  {
    // clone an updated version of the old entry
    newentry = CloneEntry (oldentry, destLayer);

    if (!newentry)
      return ENOMEM;

    newentry->SetName (new_filename);

    newdir->AddEntry (newentry);
  }

  // update the file data
  TFileData dummy;

  if (!fCacheManager.FileAttribute (newentry->GetLayer (), new_fullpath, dummy))
    return ENOENT;

  newentry->SetData (dummy);

  // remove the old entry
  olddir->RemoveEntry (oldentry);

  // if the source was a checked-out file - pop up the CVS version
  oldentry = FindEntry (&fCvsDir, old_fullpath);
  if (oldentry)
    olddir->AddEntry (oldentry->Clone ());	// then bring it back to front

  return 0;
}



int TCvsInterfacePserver::SetAttr (const std::string & path,
				   const std::string & version,
				   int attribute)
{
  if (!LoadTree ())
    return EIO;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  TEntry *entry = FindEntry (fRootDir, fullpath);
  if (!entry)
    return ENOENT;

  if (entry->GetLayer () == fRemote->GetLayer ())
    return EROFS;

  TFileData temp;

  if (!fCacheManager.FileAttribute (entry->GetLayer (), fullpath, temp))
    return EIO;

  temp.SetAttribute (attribute);

  if (!fCacheManager.SetAttribute (entry->GetLayer (), fullpath, temp))
    return EIO;

  entry->SetData (temp);

  return 0;
}



int TCvsInterfacePserver::Invalidate (const std::string & path,
				      const std::string & version)
{
  return 0;
}



int TCvsInterfacePserver::GetLocation (const std::string & path,
				       const std::string & version,
				       std::string & location)
{
  if (!LoadTree ())
    return EIO;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  TEntry *entry = FindEntry (fRootDir, fullpath);
  if (!entry)
    return ENOENT;

  if (entry->GetLayer () == fLocal->GetLayer ())
    location = "view private";
  else
    if (entry->GetLayer () == fCheckedOut->GetLayer ())
    {
      TFile *f = static_cast<TFile *> (entry);

      location = "checked out from ";
      location += f->GetVersion ();
    }
    else
      if (version.length () == 0)
      {
        TVersionedFile *f = static_cast<TVersionedFile *> (entry);

        location = f->GetHeadVersion ();
      }
      else
        location = version;

  return 0;	// everything went ok
}



int TCvsInterfacePserver::Checkout (const std::string & path,
				    const std::string & version)
{
  if (!LoadTree ())
    return EIO;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  std::string filename;

  TDirectory *dir = GetParentDirectory (fullpath, filename);
  if (!dir)
    return EIO;

  TEntry *entry = dir->FindEntry (filename);
  if (!entry)
    return ENOENT;

  if (entry->isA () == TEntry::DirEntry)
    return EISDIR;

  bool LocalVersionFound = entry->GetLayer () != fRemote->GetLayer ();

  // exists a local or checked-out version of this file ?
  if (LocalVersionFound)
  {
    // first delete the writeable version
    fCacheManager.DeleteFile (entry->GetLayer (), fullpath);

    dir->RemoveEntry (entry);

    // obtain info from CVS dir tree
    entry = FindEntry (&fCvsDir, fullpath);
    if (!entry)
      return EIO;
  }

  // now we have the CVS entry in 'entry'
  TVersionedFile *file = static_cast<TVersionedFile *> (entry);

  // load the file of the specified version from remote if necessary
  const TFile *loadedFile = LoadFile (fullpath, version, *file);
  if (loadedFile == NULL)
    return EIO;

  // allocate the file in the checkout cache (with size = 0)
  std::string versionedpath = fullpath + "@@" + loadedFile->GetVersion ();

  if (!fCheckedOut->CreateFile (versionedpath, loadedFile->GetData ().GetAttribute ()))
    return EIO;

  // copy the file contents from CVS cache to 'checked-out space'
  std::ostream * fout = fCheckedOut->Out (versionedpath, std::ios::binary);
  if (!fout)		// can not open file ?
  {
    delete fout;

    return ENOENT;
  }

  std::istream * fin = fRemote->In (versionedpath, std::ios::binary);
  if (!fin)		// can not open file ?
  {
    delete fout;
    delete fin;

    return ENOENT;
  }

  char buffer[4096];
  int count;

  fin->read (buffer, 4096);

  while ((count = fin->gcount ()) > 0)
  {
    fout->write (buffer, count);
    fin->read (buffer, 4096);
  }

  delete fout;
  delete fin;

  // directory cache management
  TFile *newfile = new TFile (*loadedFile);
  if (!newfile)
    return ENOMEM;

  // obtain the new file attributes
  TFileData dummy;

  if (!fCheckedOut->FileAttribute (fullpath, dummy))
  {
    delete newfile;

    return EIO;
  }

  newfile->SetData (dummy);
  newfile->ResetReadOnly ();	/* is now writeable */

  // remove the old directory entry
  if (!LocalVersionFound)
    dir->RemoveEntry (entry);

  // add the new entry
  newfile->SetLayer (fCheckedOut->GetLayer ());

  dir->AddEntry (newfile);

  return 0;
}



int TCvsInterfacePserver::Checkin (const std::string & path,
				   const std::string & version)
{
  if (!LoadTree ())
    return EIO;

  std::string fullpath;

  if (fConnection.GetProject () == ".")
  {
    fullpath = path;
    fullpath.erase (0, 1);	// skip leading slash
  }
  else
    fullpath = fConnection.GetProject () + path;

  TEntry *entry = FindEntry (fRootDir, fullpath);
  if (!entry)
    return ENOENT;

  if (entry->GetLayer () != fCheckedOut->GetLayer ())
  {
    if (entry->GetLayer () == fLocal->GetLayer ())
      return CVSFS_ENOTVERSIONED;
    else
      return CVSFS_ENOTCHECKEDOUT;
  }

  if (entry->isA () == TEntry::DirEntry)
    return EISDIR;

  // evaluate parts of the file path
  std::string::size_type pos = fullpath.rfind ('/');
  std::string dir;
  if (pos != std::string::npos)
  {
    dir = fullpath;
    dir.erase (pos);
    dir.insert (0, "/");
  }

  // now we have the item to check in
//  if (!CheckIn ())
//    return EIO;

  return 0;
}



int TCvsInterfacePserver::Update (const std::string & path,
				  const std::string & version)
{
  return 0;
}



/***************************************************************
 **  The supporting methods                                   **
 ***************************************************************/



bool TCvsInterfacePserver::LoadTree ()
{
  if (fTreeLoaded)
    return true;

  fCvsDir.SetLayer (fRemote->GetLayer ());

  if (!LoadCvsTree ())
    return false;

  if (fRootDir)
    delete fRootDir;

  fRootDir = new TDirectory (fCvsDir);
  if (!fRootDir)
  {
    fTreeLoaded = false;

    return false;
  }

  return fCacheManager.LoadTree (*fRootDir);
}



bool TCvsInterfacePserver::LoadCvsTree ()
{
  TCvsSessionPserver *session = fConnection.Open ();
  if (session == 0)
    return false;

  TCvsPserverCommandTree command ("", fCvsDir);

  fTreeLoaded = session->ExecuteCommand (command);

  delete session;

  return fTreeLoaded;
}



TEntry *TCvsInterfacePserver::FindEntry (TDirectory * root,
					 const std::string & path) const
{
  std::string::size_type pos;

  if ((path.length () == 0) || (path == "."))	// get the root ?
    return root;

  TDirectory *dir = root;
  std::string directory = path;

  pos = directory.find ("/");

  while (pos != std::string::npos)
  {
    std::string dirpart = directory;

    dirpart.erase (pos);
    directory.erase (0, pos + 1);

    TEntry *entry = dir->FindEntry (dirpart);

    if (entry == 0)
      return 0;

    if (entry->isA () != TEntry::DirEntry)
      return 0;

    dir = static_cast<TDirectory *> (entry);

    pos = directory.find ("/");
  };

  return dir->FindEntry (directory);
}



void TCvsInterfacePserver::RemoveEntry (const std::string & path)
{
  std::string::size_type pos;

  if ((path.length () == 0) || (path == "."))	// get the root ?
    return;

  TDirectory *dir = fRootDir;
  std::string directory = path;

  pos = directory.find ("/");
  while (pos != std::string::npos)
  {
    std::string dirpart = directory;

    dirpart.erase (pos);
    directory.erase (0, pos + 1);

    TEntry *actual = dir->FindEntry (dirpart);

    if (actual == 0)
      return;

    if (actual->isA () != TEntry::DirEntry)
      return;

    dir = static_cast<TDirectory *> (actual);

    pos = directory.find ("/");
  }

  // now we are in the parent of the to delete directory
  TEntry *entry = dir->FindEntry (directory);
  if (entry)
    dir->RemoveEntry (entry);
}



TDirectory *TCvsInterfacePserver::AllocateDir (TDirectory * rootdir,
					       const std::string & path) const
{
  std::string::size_type pos;

  if ((path.length () == 0) || (path == "."))	// get the root ?
    return rootdir;

  TDirectory *dir = rootdir;
  std::string directory = path;

  do
  {
    std::string dirpart = directory;

    pos = directory.find ("/");
    if (pos != std::string::npos)
    {
      dirpart.erase (pos);
      directory.erase (0, pos + 1);
    }
    else
      directory = "";

    TEntry *entry = dir->FindEntry (dirpart);
    if (entry != 0)
    {
      if (entry->isA () != TEntry::DirEntry)
      {
        dir->RemoveEntry (entry);
        entry = 0;
      }
    }

    if (entry == 0)
    {
      TDirectory *newdir = new TDirectory (dirpart);
      dir->AddEntry (newdir);
      dir = newdir;
    }
    else
      dir = static_cast<TDirectory *> (entry);

  } while (pos != std::string::npos);

  return dir;
}



bool TCvsInterfacePserver::LoadAttribute (const std::string & path,
				 	  const std::string & version,
					  TVersionedFile & file)
{
  std::string realversion;
  bool result;

  TCvsSessionPserver *session = fConnection.Open ();
  if (session == 0)
    return false;

  if (version.length () == 0)
    realversion = file.GetHeadVersion ();
  else
    realversion = version;

  TCvsPserverCommandCheckout command (path, realversion, 0);

  result = session->ExecuteCommand (command);
  if (result)
    file.AddVersion (realversion, command.GetData ());

  delete session;

  return result;
}



const TFile * TCvsInterfacePserver::LoadFile (const std::string & path,
					      const std::string & version,
					      TVersionedFile & file)
{
  std::string realversion;
  bool result;

  if (version.length () == 0)
    realversion = file.GetHeadVersion ();
  else
    realversion = version;

  std::string fullpath = path + "@@" + realversion;

  const TFile *item = file.FindVersion (realversion);

  if (item && fRemote->HaveFile (fullpath))
    return item;

  // we do not have it in the file cache - so we have to load it
  TCvsSessionPserver *session = fConnection.Open ();
  if (session == 0)
    return false;

  fRemote->CreateFile (fullpath, S_IRUSR | S_IWUSR);
  std::ofstream *dest = fRemote->Out (fullpath, std::ios::trunc | std::ios::binary);

  TCvsPserverCommandCheckout command (path, realversion, dest);

  result = session->ExecuteCommand (command);

  delete dest;

  if (result)
  {
    if (!item)
      file.AddVersion (realversion, command.GetData ());
  }
  else
    fRemote->DeleteFile (fullpath);

  delete session;

  return result ? item : 0;
}



TDirectory *TCvsInterfacePserver::GetParentDirectory (const std::string & path,
						      std::string & filename)
{
  std::string::size_type pos;

  if ((path.length () == 0) || (path == "."))	// get the root ?
    return 0;

  TDirectory *dir = fRootDir;
  std::string directory = path;

  pos = directory.find ("/");
  while (pos != std::string::npos)
  {
    std::string dirpart = directory;

    dirpart.erase (pos);
    directory.erase (0, pos + 1);

    TEntry *actual = dir->FindEntry (dirpart);

    if (actual == 0)
    {
      TDirectory *newdir = new TDirectory (dirpart);
      if (!newdir)
        return 0;	// out of memory

      dir->AddEntry (newdir);
      actual = newdir;
    }

    if (actual->isA () != TEntry::DirEntry)
      return 0;

    dir = static_cast<TDirectory *> (actual);

    pos = directory.find ("/");
  }

  // now we are in the parent of the to be added file
  filename = directory;

  return dir;
}



bool TCvsInterfacePserver::StoreFile (const std::string & commitstring,
				      const std::string & path, TFile & file,
				      std::string outcome)
{
//  TSyslog *log = TSyslog::instance ();

  TCvsSession *session = fConnection.Open ();
  if (session == 0)
    return false;
/*
  if (session->SendCiInit (path, file.GetName (), "",
                           file.GetData ().GetVersion (), commitstring))
  {
    if (!SendFileData (file.GetData ()))
    {
      if (session->SendCiExit (file.GetName ()))
      {
        try
        {
          std::string response;

          // now analyze the response we get from the CVS server
          response = session->ReadLine ();
          while (response != "ok")
          {
            if (response == "error")
              break;

            log->debug << "ci-Response: " << response << std::endl;

            outcome += response;
            outcome += '\n';

            response = session->ReadLine ();
          }
        }
        catch (XPserverTimeout e)
        {
          log->error << "StoreFile: " << e << std::endl;
        }
      }
      else
        log->error << "StoreFile: Failed sending checkin trailer for file " << file.GetName () << std::endl;
    }
    else
      log->error << "StoreFile: Failed sending file" << file.GetName () << std::endl;
  }
  else
    log->error << "StoreFile: Failed sending checkin prefix for file " << file.GetName () << std::endl;
*/
  delete session;

  return true;
}



TEntry * TCvsInterfacePserver::CloneEntry (TEntry * old,
					   TCacheManager::size_type layer) const
{
  if (old->isA () != TEntry::DirEntry)
  {
    // it is not a directory - clone it
    TFile * file = new TFile (old->GetName (), "");

    if (!file)
      return 0;

    if (old->ValidData ())
      file->SetData (old->GetData ());

    file->SetLayer (layer);

    return file;
  }

  // it is a directory - clone it and traverse through its contents
  TDirectory * dir = new TDirectory (old->GetName ());

  if (!dir)
    return 0;

  if (old->ValidData ())
    dir->SetData (old->GetData ());

  dir->SetLayer (layer);

  // now traverse through the contents
  int pos = 0;
  TDirectory *olddir = static_cast<TDirectory *> (old);
  TEntry *entry;

  while ((entry = olddir->GetEntry (pos)) != 0)
  {
    TEntry *newentry = CloneEntry (entry, layer);

    if (!newentry)
    {
      delete dir;

      return 0;
    }

    dir->AddEntry (newentry);

    ++pos;
  }

  return dir;
}
