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

#define _GNU_SOURCE

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
#include "TCvsSession.h"
//#include "TCachedFile.h"
#include "TCacheSystemCheckedout.h"
#include "TCacheSystemSimple.h"
#include "TCacheSystemVersioned.h"
#include "TSyslog.h"
#include "XPserverTimeout.h"



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
  TSyslog *log = TSyslog::instance ();
  std::string response;

  TCvsSession *session = fConnection.Open ();
  if (session == 0)
    return false;

  if (session->SendRdiff (""))
  {
    // now analyze the response we get from the CVS server
    TDirectory *dir = 0;
    std::string basedir;

    try
    {
      response = session->ReadLine ();
    }
    catch (XPserverTimeout e)
    {
      delete session;

      log->error << "LoadTree: " << e << std::endl;

      return false;
    }

    while (response != "ok")
    {
      std::string name;
      std::string version;

      if (response == "error")
        break;

      switch (AnalyzeRDiffLine (response, name, version))
      {
        case RDIFF_FILE:
          if (dir != 0)	// only add if a directory came along before
          {
            if (basedir.length () != 0)
              name.erase (0, basedir.length () + 1);

            AddVersionedFile (dir, name, version);

            log->debug << "File: " << name << " - version " << version << std::endl;
          }
          break;

        case RDIFF_DIR:
	  dir = AllocateDir (&fCvsDir, name);
          if (name == ".")
            basedir = "";
          else
            basedir = name;
          dir->SetLayer (fRemote->GetLayer ());

          log->debug << "Directory: " << name << std::endl;
          break;

        default:
          log->debug << "rdiff-Response: " << response << std::endl;
          break;
      }

      try
      {
        response = session->ReadLine ();
      }
      catch (XPserverTimeout e)
      {
        delete session;

        log->error << "LoadTree: " << e << std::endl;

        return false;
      }
    }

    if (response == "ok")
      fTreeLoaded = true;
  }

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



TVersionedFile *TCvsInterfacePserver::AddVersionedFile (TDirectory * dir,
							const std::string & name,
							const std::string & version) const
{
  TVersionedFile *file;
  TEntry *entry = dir->FindEntry (name);

  // in the list is already an entry - is it a file ?
  if ((entry) && (entry->isA () != TEntry::VersionedFileEntry))
  {
    // it is a directory - kill it
    dir->RemoveEntry (entry);

    entry = 0;
  }

  if (entry == 0)
  {
    file = new TVersionedFile (name, version);
    file->SetLayer (fRemote->GetLayer ());

    dir->AddEntry (file);
  }
  else
  {
    file = static_cast<TVersionedFile *> (entry);

    file->SetHeadVersion (version);
  }

  return file;
}



TCvsInterfacePserver::RDiffResult
TCvsInterfacePserver::AnalyzeRDiffLine (const std::string & line,
					std::string & name,
					std::string & version) const
{
  const std::string dir_pattern		= "E cvs server: Diffing ";
  const std::string file_pattern	= "M File ";
  const std::string newrev_pattern	= " is new; current revision ";
  const std::string changed1_pattern	= " changed from revision ";
  const std::string changed2_pattern	= " to ";
  const std::string unknown_version	= "<unknown>";
  RDiffResult result = RDIFF_ELSE;

  if (strncmp (file_pattern.c_str (), line.c_str (), file_pattern.length ()) == 0)
  {					// we have a valid line now
    std::string::size_type pos;

    result = RDIFF_FILE;		// is is a file

    version = unknown_version;

    name = line.substr (file_pattern.length ());
    pos = name.find (newrev_pattern);
    if (pos != std::string::npos)
    {					// a newly created file version
      version = name.substr (pos + newrev_pattern.length ());
      name.erase (pos);
    }
    else
    {
      pos = name.find (changed1_pattern);
      if (pos != std::string::npos)
      {					// a previous changed file version
        std::string temp = name.substr (pos + changed1_pattern.length ());
        name.erase (pos);

        pos = temp.find (changed2_pattern);
        if (pos != std::string::npos)
          version = temp.substr (pos + changed2_pattern.length ());
      }
      else
      {
        pos = name.find (" ");
        if (pos != std::string::npos)
          name.erase (pos);
      }
    }
  }
  else
    if (strncmp (dir_pattern.c_str (), line.c_str (), dir_pattern.length ()) == 0)
    {
      result = RDIFF_DIR;		// is is a directory

      name = line.substr (dir_pattern.length ());
    }

  return result;
}



bool TCvsInterfacePserver::LoadAttribute (const std::string & path,
				 	  const std::string & version,
					  TVersionedFile & file)
{
  TSyslog *log = TSyslog::instance ();
  std::string response;
  std::string realversion;

  // we do it in a seperate session because we need to kill the connection
  // after the relevant data have been read.
  TCvsSession *session = fConnection.Open ();
  if (session == 0)
    return false;

  if (version.length () == 0)
    realversion = file.GetHeadVersion ();
  else
    realversion = version;

  if (session->SendCo (path, realversion))
  {
    // now analyze the response we get from the CVS server
    TFileData fileData;

    try
    {
      response = session->ReadLine ();
    }
    catch (XPserverTimeout e)
    {
      delete session;

      log->error << "LoadAttribute: " << e << std::endl;

      return false;
    }

    while (response != "ok")
    {
      int time;
      std::string stringpart;

      if (response == "error")
        break;

      switch (AnalyzeCoLine (response, stringpart))
      {
        case CO_MODTIME:
          time = ConvertTime (stringpart);
          fileData.SetAtime (time);
          fileData.SetCtime (time);
          fileData.SetMtime (time);
          log->debug << "Modtime: " << stringpart << std::endl;
          break;

        case CO_ATTRIB:
          fileData.SetAttribute (ConvertAttr (response));
          log->debug << "Attribs: " << response << std::endl;
         break;

        case CO_SIZE:
          fileData.SetSize (strtoul (response.c_str (), 0, 0));
          file.AddVersion (realversion, fileData);
          log->debug << "Size: " << response << std::endl;
          delete session;
	  return true;		// skip the rest of the data

        case CO_ELSE:
          log->debug << "co-Response: " << response << std::endl;
          break;
      }

      try
      {
        response = session->ReadLine ();
      }
      catch (XPserverTimeout e)
      {
        delete session;

        log->error << "LoadAttribute: " << e << std::endl;

        return false;
      }
    }

    delete session;
  }

  return false;
}



TCvsInterfacePserver::CoResult
TCvsInterfacePserver::AnalyzeCoLine (const std::string & line,
				     std::string & result) const
{
  const std::string modtime_pattern	= "Mod-time ";
  const std::string attribs_pattern	= "u=";

  result = line;

  if (strncmp (modtime_pattern.c_str (), line.c_str (), modtime_pattern.length ()) == 0)
  {
    result.erase (0, modtime_pattern.length ());

    return CO_MODTIME;
  }
  else
    if (strncmp (attribs_pattern.c_str (), line.c_str (), attribs_pattern.length ()) == 0)
    {
      result.erase (0, attribs_pattern.length ());

      return CO_ATTRIB;
    }
    else
      if (isdigit (line[0]) != 0)
        return CO_SIZE;

  return CO_ELSE;
}



int TCvsInterfacePserver::ConvertTime (const std::string & data) const
{
  char *unprocessed;
  struct tm tm;

  memset (&tm, 0, sizeof (struct tm));
  unprocessed = strptime (data.c_str (), "%d %b %Y %T ", &tm);

  if (unprocessed != 0)
  {
    return mktime (&tm);
  }

  return 0;
}



int TCvsInterfacePserver::ConvertAttr (const std::string & data) const
{
  const int process[3][3] = {{S_IRUSR, S_IWUSR, S_IXUSR},
			     {S_IRGRP, S_IWGRP, S_IXGRP},
                             {S_IROTH, S_IWOTH, S_IXOTH}};
  std::string::const_iterator iter = data.begin ();
  int part = 0;
  int attr = 0;

  ++iter;
  ++iter;

  for (; iter != data.end (); ++iter)
  {
    if (*iter == ',')
    {
      ++iter;

      switch (*iter)
      {
        case 'g': part = 1; break;
        case 'o': part = 2; break;
      }

      ++iter;
    }
    else
      switch (*iter)
      {
        case 'r': attr |= process[part][0]; break;
        case 'w': attr |= process[part][1]; break;
        case 'x': attr |= process[part][2]; break;
      }
  }

  return attr;
}



const TFile * TCvsInterfacePserver::LoadFile (const std::string & path,
					      const std::string & version,
					      TVersionedFile & file)
{
  TSyslog *log = TSyslog::instance ();
  std::string realversion;

  if (version.length () == 0)
    realversion = file.GetHeadVersion ();
  else
    realversion = version;

  std::string fullpath = path + "@@" + realversion;
  bool obtainFile = false;

  const TFile *item = file.FindVersion (realversion);

  if (!item)
    obtainFile = true;
  else
    obtainFile = !fRemote->HaveFile (fullpath);

  if (obtainFile)
  {
    // we do not have it in the file cache - so we have to load it
    TCvsSession *session = fConnection.Open ();
    if (session == 0)
      return 0;

    if (session->SendCo (path, realversion))
    {
      bool beginOfData = false;

      try
      {
        int time;
        std::string response;
        std::string stringpart;
        TFileData fileData;

        // now analyze the response we get from the CVS server
        response = session->ReadLine ();
        while ((response != "ok") && (!beginOfData))
        {
          if (response == "error")
            break;

          switch (AnalyzeCoLine (response, stringpart))
          {
            case CO_MODTIME:
              time = ConvertTime (stringpart);
              fileData.SetAtime (time);
              fileData.SetCtime (time);
              fileData.SetMtime (time);
              log->debug << "Modtime: " << stringpart << std::endl;
              break;

            case CO_ATTRIB:
              fileData.SetAttribute (ConvertAttr (response));
              log->debug << "Attribs: " << response << std::endl;
             break;

            case CO_SIZE:
              fileData.SetSize (strtoul (response.c_str (), 0, 0));
              log->debug << "Size: " << response << std::endl;

              if (!item)		// add item read to version list
                file.AddVersion (realversion, fileData);

              beginOfData = true;
              break;

            case CO_ELSE:
              log->debug << "co-Response: " << response << std::endl;
              break;
          }

          if (!beginOfData)
            response = session->ReadLine ();
        }

        if (!item)
        {		// add item read to version list
          item = file.FindVersion (realversion);
        }

        if (beginOfData)
        {
          fRemote->CreateFile (fullpath, S_IRUSR | S_IWUSR);
          std::ofstream *dest = fRemote->Out (fullpath, std::ios::trunc | std::ios::binary);
          const int bufsize = 4096;
          char readbuf[4096];
          unsigned long pos;

          for (pos = 0; pos < fileData.GetSize (); pos += bufsize)
            if ((pos + bufsize) > fileData.GetSize ())
            {
              session->ReadRaw (readbuf, fileData.GetSize () - pos);
              dest->write (readbuf, fileData.GetSize () - pos);
            }
            else
            {
              session->ReadRaw (readbuf, bufsize);
              dest->write (readbuf, bufsize);
            }

          delete dest;
        }
      }
      catch (XPserverTimeout e)
      {
        log->error << "LoadFile: " << e << std::endl;
      }

      delete session;
    }
  }

  return item;
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
