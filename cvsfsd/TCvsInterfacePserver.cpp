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
#include "TCachedFile.h"
#include "TSyslog.h"
#include "XPserverTimeout.h"



TCvsInterfacePserver::TCvsInterfacePserver (const TMountParameters & parms)
: TCvsInterface (), fConnection (parms),
  fVersionedCache ("/var/cache/cvsfs", fConnection.GetServer (), fConnection.GetRoot ()),
  fWorkCache ("/var/cache/cvsfs", fConnection.GetMountPoint ())
{
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

  const TEntry *entry = FindEntry (&fRootDir, fullpath);
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

  TEntry *entry = FindEntry (&fRootDir, fullpath);

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

  TEntry *entry = FindEntry (&fRootDir, fullpath);
  if (entry)
    return 0;

  if (!fWorkCache.MakeDirectory (fullpath, mode))
    return 0;

  TDirectory * dir = AllocateDir (&fRootDir, fullpath);
  if (dir)
  {
    TFileData dummy;

    if (!fWorkCache.FileData (fullpath, dummy))
      return 0;

    dir->SetData (dummy);
    dir->SetSource (TEntry::Local);
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

  TEntry *entry = FindEntry (&fRootDir, fullpath);

  if (entry == 0)
    return ENOENT;

  if (entry->isA () != TEntry::DirEntry)
    return ENOTDIR;

  if (entry->GetSource () != TEntry::Local)
    return EROFS;

  int retval = fWorkCache.RemoveDirectory (fullpath);

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
  if (!fWorkCache.MakeFile (fullpath, mode))
    return 0;

  TFile *file = new TFile (filename, "");
  if (!file)
    return 0;

  // add the file to the tree info
  TFileData dummy;

  if (!fWorkCache.FileData (fullpath, dummy))
  {
    delete file;

    return 0;
  }

  file->SetData (dummy);
  file->SetSource (TEntry::Local);

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

  if (entry->GetSource () != TEntry::Local)
    return EROFS;

  int retval = fWorkCache.RemoveFile (fullpath);

  if (retval == 0)
    dir->RemoveEntry (entry);

  return retval;
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

  TEntry *entry = FindEntry (&fRootDir, fullpath);

  if (entry == 0)
    return -1;		// file not known

  if (entry->isA () != TEntry::DirEntry)
  {
    if (entry->isA () == TEntry::VersionedFileEntry)
    {
      TVersionedFile *file = static_cast<TVersionedFile *> (entry);

      if (file->FindVersion (version) == 0)
        if (!LoadAttribute (fullpath, version, *file))
          return 0;

        return LoadFile (fullpath, version, *file, start, count, buffer);
    }
    else
      if (entry->GetSource () == TEntry::Local)
      {
        TCachedFile *file = fWorkCache.CachedFile (fullpath);
        if (file)
        {
          count = file->ReadFile (buffer, start, count);

          delete file;

          return count;
        }
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

  TEntry *entry = FindEntry (&fRootDir, fullpath);

  if (entry == 0)
    return ENOENT;		// file not known

  if (entry->GetSource () != TEntry::Local)
    return EROFS;

  if (entry->isA () != TEntry::FileEntry)
    return EISDIR;

  TCachedFile *file = fWorkCache.CachedFile (fullpath);
  if (!file)
    return ENOMEM;

  std::ostream *f = file->OpenForWrite (ios::binary);
  if (!f)		// can not open file ?
  {
    delete file;

    return ENOENT;
  }

  if (start != 0)
    f->seekp (start);

  f->write (buffer, count);

  delete f;
  delete file;

  // now the info in the file entry must be updated
  TFileData newdata;

  if (!fWorkCache.FileData (fullpath, newdata))
    return ENOENT;

  TFile *changedFile = static_cast<TFile *> (entry);

  changedFile->SetData (newdata);

  return 0;	// file not known
}



bool TCvsInterfacePserver::LoadTree ()
{
  if (fTreeLoaded)
    return true;

  if (!LoadCvsTree ())
    return false;

  return fWorkCache.LoadTree (fRootDir);
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
	  dir = AllocateDir (&fRootDir, name);
          if (name == ".")
            basedir = "";
          else
            basedir = name;

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

  TDirectory *dir = &fRootDir;
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
  if ((entry != 0) && (entry->isA () != TEntry::FileEntry))
  {
    // it is a directory - kill it
    dir->RemoveEntry (entry);

    entry = 0;
  }

  if (entry == 0)
  {
    file = new TVersionedFile (name, version);

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



int TCvsInterfacePserver::LoadFile (const std::string & path,
				    const std::string & version,
				    TVersionedFile & file,
				    long long start, int count,
				    char * buffer)
{
  TSyslog *log = TSyslog::instance ();
  std::string response;

  if (version.length () == 0)
    response = file.GetHeadVersion ();
  else
    response = version;

  const TFile *item = file.FindVersion (response);
  if (item == 0)
    return -1;

  const TFileData & filedata = item->GetData ();

  if (start >= filedata.GetSize ())
    return -1;

  TCachedFile *cachedFile = fVersionedCache.CachedFile (path, version);
  if (cachedFile == 0)
    return -1;

  if (!(cachedFile->HaveFile ()))
  {
    // we do not have it in the file cache - so we have it to load first.
    TCvsSession *session = fConnection.Open ();
    if (session == 0)
    {
      delete cachedFile;
      return 0;
    }

    if (session->SendCo (path, response))
    {
      bool beginOfData = false;

      try
      {
        // now analyze the response we get from the CVS server
        response = session->ReadLine ();
        while ((response != "ok") && (!beginOfData))
        {
          if (response == "error")
            break;

          if (isdigit (response[0]) != 0)
            beginOfData = true;
          else
            response = session->ReadLine ();
        }

        if (beginOfData)
        {
          std::ostream *dest = cachedFile->OpenForWrite (std::ios::trunc | std::ios::binary);
          const int bufsize = 4096;
          char readbuf[4096];
          unsigned long pos;

          for (pos = 0; pos < filedata.GetSize (); pos += bufsize)
            if ((pos + bufsize) > filedata.GetSize ())
            {
              session->ReadRaw (readbuf, filedata.GetSize () - pos);
              dest->write (readbuf, filedata.GetSize () - pos);
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

  count = cachedFile->ReadFile (buffer, start, count);

  delete cachedFile;

  return count;
}



TDirectory *TCvsInterfacePserver::GetParentDirectory (const std::string & path,
						      std::string & filename)
{
  std::string::size_type pos;

  if ((path.length () == 0) || (path == "."))	// get the root ?
    return 0;

  TDirectory *dir = &fRootDir;
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
