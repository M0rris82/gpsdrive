// $Id$
// garmin_packet.h
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

// garmin packet IDs

#ifndef _garpack
#define _garpack

namespace garmin {

enum packet_id {
	// my temporary pid
	pid_no_packet = 0,

	// protocol L000
	pid_ack_byte = 6,
	pid_nack_byte = 21,
	pid_protocol_array = 253,
	pid_product_rqst = 254,
	pid_product_data = 255,
	
	// protocol L001
	pid_command_data = 10,
	pid_xfer_cmplt = 12,
	pid_date_time_data = 14,
	pid_position_data = 17,
	pid_prx_wpt_data = 19,
	pid_records = 27,
	pid_rte_hdr = 29,
	pid_rte_wpt_data = 30,
	pid_almanac_data = 31,
	pid_trk_data = 34,
	pid_wpt_data = 35,
	pid_pvt_data = 51,
	pid_rte_link_data = 98,
	pid_trk_hdr = 99,
	pid_display_data = 69,
	
	// protocol L002
	// Note: some packet ids are duplicated from L002, and some
	// ids are the same name but different values from L001.
	pid_almanac_data002 = 4,
	pid_command_data002 = 10,
	pid_xfer_cmplt002 = 12,
	pid_date_time_data002 = 14,
	pid_position_data002 = 17,
	pid_records002 = 35,
	pid_rte_hdr002 = 37,
	pid_rte_wpt_data002 = 39,
	pid_wpt_data002 = 43
	
};

} // namespace

#endif // _garpack
