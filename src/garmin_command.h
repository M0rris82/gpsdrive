// $Id$
// garmin_command.h
// Douglas S. J. De Couto
// September 9, 1998

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

#ifndef _garcmdh
#define _garcmdh

// garmin command protocol codes

namespace garmin
{

enum device_cmnd {
	// protocol A010
	cmnd_abort_transfer = 0,
	cmnd_transfer_alm = 1,
	cmnd_transfer_posn = 2,
	cmnd_transfer_prx = 3, 
	cmnd_transfer_rte = 4,
	cmnd_transfer_time = 5,
	cmnd_transfer_trk = 6,
	cmnd_transfer_wpt = 7,
	cmnd_turn_off_pwr = 8,
	cmnd_start_pvt_data = 49,
	cmnd_stop_pvt_data = 50,
	cmnd_transfer_screenbitmap = 32,
	
	// protocol A011
	cmnd_abort_transfer011 = 0,
	cmnd_transfer_alm011 = 4,
	cmnd_transfer_rte011 = 8,
	cmnd_transfer_time011 = 20,
	cmnd_transfer_wpt011 = 21,
	cmnd_turn_off_pwr011 = 26
};

} // namespace

#endif // _garcmdh
