// -*- mode: c++; c-basic-offset: 8; -*-
// $Id$
// garmin_legacy.h
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

#ifndef _garmin_legacy
#define _garmin_legacy

#include "garmin_types.h"

namespace garmin {

	struct legacy_protocol
	{
		char 	tag;
		uint16  id;
		uint16  types[4];
		// legacy has maximum of 2/3 datatypes per protocol
		// a 0xffff marks end
	};

	typedef legacy_protocol legacy_protocols[12];

	const uint16 type_end =0xffff;
	const uint16 max_version =0xffff;
	const uint16 min_version = 0;
	
	struct legacy_device 
	{
		uint16 id;
		uint16 version_min;
		uint16 version_max;
		legacy_protocol* protocols;
	} ;

	extern legacy_device legacy_devices[];
} // namespace

#endif // _garmin_legacy
