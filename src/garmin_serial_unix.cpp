// $Id$
// garmin_serial_unix.cp
// Douglas S. J. De Couto
// February 13, 2000

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

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "garmin_serial_unix.h"

#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

garmin_serial::garmin_serial(std::string port_dev, int timeout) 
  : m_init(false), m_port_dev(port_dev), m_timeout(timeout)
{
  try {
    init(m_port_dev, m_timeout);
  }
  catch (garmin::not_possible &ex) {
    // fail silently
  }
}

std::string
garmin_serial::m_build_err_string(std::string msg)
{
  std::string s(msg);
  s += m_port_dev;
  s += ": ";
  s += strerror(errno);
  return s;
}

void
garmin_serial::init(std::string port_dev, int timeout) throw (garmin::not_possible)
{
  if (m_init) {
    int err = close(m_fd);
    if (err == -1) 
      throw garmin::not_possible(m_build_err_string("Unable to close current serial port device "));
    m_init = false;
  }
  m_timeout = timeout;
  m_port_dev = port_dev;
#if defined (__OpenBSD__)
  m_fd = open(m_port_dev.c_str(), O_RDWR | O_NONBLOCK);
#else
  m_fd = open(m_port_dev.c_str(), O_RDWR | O_NOCTTY);
#endif
  if (m_fd == -1)
    throw garmin::not_possible(m_build_err_string("Unable to open serial port device "));

  // set proper speed, parity, char size
  int err;
  err = tcgetattr(m_fd, &m_old_options);
  if (err == -1)
    throw garmin::not_possible(m_build_err_string("Unable to get old device options for "));

  struct termios options;
  memset(&options, 0, sizeof(options));
  
  options.c_cflag |= (CLOCAL | CREAD); // enable local mode, reading characters
  
  // speed, 9600 bps for garmin physical layer
  err = cfsetispeed(&options, B9600);
  if (err == -1)
    throw garmin::not_possible(m_build_err_string("Unable to set input speed to 9600 bps for "));

  err = cfsetospeed(&options, B9600);
  if (err == -1)
    throw garmin::not_possible(m_build_err_string("Unable to set output speed to 9600 bps for "));
  
  // parity -- none
  options.c_cflag &= ~PARENB;
  // char size -- 8 bits
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;
  
  // set one stop bit
  options.c_cflag &= ~CSTOPB; // NOT two stop bits
  
  // set the new options
  err = tcsetattr(m_fd, TCSADRAIN, &options);
  if (err == -1)
    throw garmin::not_possible(m_build_err_string("Unable to set device options for "));

  m_init = true;  
}

void 
garmin_serial::putbyte(const garmin::uint8 c) throw (garmin::not_possible)
{
  if (!m_init) {
    throw garmin::not_possible("The serial port is not initialized");
  }

  int err = write(m_fd, &c, 1);
  if (err == -1) 
    throw garmin::not_possible(m_build_err_string("Unable to write to "));
  if (err != 1)
    throw garmin::not_possible(m_build_err_string("Couldn't write enough bytes to ") + m_port_dev);      
}



garmin::uint8 
garmin_serial::getbyte(void) throw (garmin::not_possible, garmin::timeout)
{
  if (!m_init) {
    throw garmin::not_possible("The serial port is not initialized");
  }

  fd_set the_set;
  struct timeval tv;
  
  // set up the data for the read select call
  FD_ZERO(&the_set);
  FD_SET(m_fd, &the_set);
  
  struct timeval *tvp = &tv;
  if (m_timeout == 0) {
    tvp = 0; // so select() will block
  }
  else { // fill in the timeval struct
    int usecs = m_timeout * 1000;
    tv.tv_sec = usecs / 1000000;
    tv.tv_usec = usecs % 1000000;
  }

  int err = select(m_fd + 1, &the_set, 0, 0, tvp); 
  if (err == -1)
    throw garmin::not_possible(m_build_err_string("Error waiting for data on "));

  if (!FD_ISSET(m_fd, &the_set))
    throw garmin::timeout(std::string("Timeout waiting for data on ") + m_port_dev);

  garmin::uint8 b;
  err = read(m_fd, &b, 1);
  if (err == -1) 
    throw garmin::not_possible(m_build_err_string("Unable to read from "));    
  if (err != 1)
    throw garmin::not_possible(std::string("Couldn't read enough bytes from ") + m_port_dev);    

  return b;
}


garmin_serial::~garmin_serial()
{
  if (m_init) {
    // reset port to how we found it
    int err = tcsetattr(m_fd, TCSADRAIN, &m_old_options);
    if (err == -1) {
      // too bad, what are we going to do?
    }
    err = close(m_fd);
    if (err == -1) {
      // punt
    }
  }
}
