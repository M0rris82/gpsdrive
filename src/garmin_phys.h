// $Id$
// garmin_phys.h
// Douglas S. J. De Couto
// May 26, 1998

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

#ifndef _garphys
#define _garphys

#include "garmin_types.h"
#include "garmin_error.h"

// interface to serial ports or whatever for garmin data transfer

namespace garmin {

class physical_layer 
{
public:
	virtual void putbyte(const uint8 c) throw (not_possible, timeout) = 0;
	virtual uint8 getbyte(void) throw (not_possible, timeout) = 0;
};

} // namespace

#endif // _garphys
