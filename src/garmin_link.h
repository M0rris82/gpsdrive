// $Id$
// garmin_link.h
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

// Class for handling garmin link level protocol L000/1 for gps receivers
// Based on information in ``Garmin GPS Interface Specification'' , from 
// Garmin tech support, May 6 1998.  Handles handshaking, checksumming, 
// and data escaping.

#ifndef _garlink
#define _garlink

#include "garmin_phys.h"
#include "garmin_packet.h"
#include "garmin_types.h"

namespace garmin {
	
class link_layer 
{
public:
	
	link_layer() : m_init(false) { }
	link_layer(const link_layer &) : m_init(false) { }
	
	link_layer(physical_layer *pl, int retries = 3) 
	: m_init(true), m_pl(pl), m_retries(retries) { }
	
	void init(physical_layer *pl, int retries = 5) 
	{ m_init = true, m_pl = pl; m_retries = retries; }
	
	packet_id get_packet(uint8 *data, uint8 &sz) throw (not_possible, timeout);
	// return the packet ID of the next available packet; packet
	// payload goes into DATA, which must point to at least 255 bytes
	// of storage.  The number of data bytes goes into SZ.
	
	void put_packet(packet_id id, uint8 *data, uint8 sz) throw (not_possible, timeout);
	// send a packet.
	
private:
	
	bool m_init;
	physical_layer *m_pl;
	uint8 m_buf[256];
	int m_retries;

};

} // namespace

#endif // _garlink
