// $Id$
// garmin_serial.h
// Douglas S. J. De Couto
// September 7, 1998

// Copyright (C) 1998 Douglas S. J. De Couto
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

#ifndef _garserh
#define _garserh

#include "garmin_phys.h"
#include "serial.h"

// adapter class to use serial port object for garmin physical layer

class garmin_serial : public garmin::physical_layer 
{
public:	
	garmin_serial() : m_init(false) { }
	garmin_serial(garmin_serial &) : m_init(false) { }
	garmin_serial(serial_port *s) : m_init(true), m_port(s) { }
	void init(serial_port *s) { m_port = s; m_init = true; }

	void putbyte(const garmin::uint8 c) throw (garmin::not_possible);
	garmin::uint8 getbyte(void) throw (garmin::not_possible, garmin::timeout);

private:
	bool m_init;
	serial_port *m_port;
};

#endif // _garserh
