// $Id$
// garmin_serial_unix.h
// Douglas S. J. De Couto
// Feburary 13, 2000
// adapted from garmin_serial.h

// Copyright (C) 2000 Douglas S. J. De Couto
// <decouto@lcs.mit.edu>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#ifndef _garserunixh
#define _garserunixh

#include <string>

#include <termios.h>

#include "garmin_phys.h"

// adapter class to use unix serial port for garmin physical layer

class garmin_serial : public garmin::physical_layer 
{
public:	
  garmin_serial() : m_init(false) { }
  // can't copy serial ports
  garmin_serial(garmin_serial &) : m_init(false) { }
  // port_dev: device name, e.g. /dev/stty0; timeout in millisecs
  // 0 timeout means blobk indefinitely
  // this constructor may fail silently, should use init() instead
  garmin_serial(std::string port_dev, int timeout = 25);
  // can init more than once
  void init(std::string port_dev, int timeout = 25) throw (garmin::not_possible);

  void putbyte(const garmin::uint8 c) throw (garmin::not_possible);
  garmin::uint8 getbyte(void) throw (garmin::not_possible, garmin::timeout);

  virtual ~garmin_serial();

private:
  std::string m_build_err_string(char * msg);

  struct termios m_old_options;
  bool m_init; // true iff succesfully initialized port
  std::string m_port_dev; // name of serial port device
  int m_fd; // serial port file descriptor
  int m_timeout; // timeout in milisecs

};

#endif // _garserunixh
