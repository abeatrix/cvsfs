/***************************************************************************
                          TFunctionCheckout.h  -  description
                             -------------------
    begin                : Thu Sep 6 23:38:19 CEST 2002
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

#include "TFunctionCheckout.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <sys/ioctl.h>
#include "cvsfs_ioctl.h"



TFunctionCheckout::TFunctionCheckout ()
{
}



bool TFunctionCheckout::DoIt (int argc, char * argv [])
{
  if (argc < 1)
  {
    std::cerr << "  function parameter <filename> omitted" << std::endl;

    return false;
  }

  int fd = open (argv[0], 0);
  if (fd == -1)
  {
    std::cerr << "  could not access file " << argv[0] << std::endl
              << "  returned error #" << errno << " (" << strerror (errno) << ")" << std::endl;

    return false;
  }

  int err;

  if (argc > 1)
  {		// request a specific version
    limited_string fileversion;

    fileversion.maxsize = strlen (argv[1]) + 1;
    fileversion.string = new char[fileversion.maxsize];
    strcpy (fileversion.string, argv[1]);

    err = ioctl (fd, CVSFS_IOCPVCHECKOUT, &fileversion);

    delete [] fileversion.string;
  }
  else  
    err = ioctl (fd, CVSFS_IOC_VCHECKOUT);

  close (fd);
  
  if (err < 0)
    std::cerr << "  ioctl error code " << err << " (" << strerror (errno) << ")" << std::endl;

  return true;
}
