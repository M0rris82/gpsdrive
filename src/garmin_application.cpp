// -*- mode: c++; c-basic-offset: 8; -*-
// $Id$
// garmin_application.cp
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


#include "garmin_application.h"
#include "garmin_command.h"
#include "garmin_util.h"
#include "garmin_data.h"
#include "garmin_legacy.h"

#include <iostream>

#include <cctype>
#include <algorithm>
#include <stdio.h>
#include <assert.h>

namespace garmin 
{
using namespace std;

#define check_init() if (!m_init) throw not_possible("The garmin application layer is not initialized")


/////
// Aborts any pending transfer
////
void
application_layer::abort_transfer(void) throw (timeout)
{
	check_init();

	//	uint8 sz;
	sint16 *buf16 = (sint16 *) m_buf;
	//packet_id pid;

	buf16[0] = host2garmin16((sint16) cmnd_abort_transfer);
	m_ll->put_packet(pid_command_data, m_buf, 2);
	
}

//////////////////

product_data_type
application_layer::get_product_data(void) throw (not_possible, timeout)
{
	check_init();
	
	uint8 sz;
	packet_id pid;


	// First, abort any pending transfer:
	abort_transfer();
	
	m_ll->put_packet(pid_product_rqst, m_buf, 0);
	pid = m_ll->get_packet(m_buf, sz);
	
	if (pid != pid_product_data) {
		throw not_possible("GPS did not reply with product data packet");
	}
	
	if (sz < 5) {
		throw not_possible("Product data packet is too small");
	}

	product_data_type pd;	
	sint16 *buf16 = (sint16 *) m_buf;
	pd.product_id = garmin2host16(buf16[0]);
	pd.software_version = garmin2host16(buf16[1]);
	pd.product_description = &((char *) m_buf)[4];
	
	// get capability protocol info... (A001),
	// but not supported on all units!
	try {
		pid = m_ll->get_packet(m_buf, sz);
	}
	catch (timeout &ex) {
		// timeout indicates gps is not sending capability protocol info
		m_got_protocol_info = false;

		// we should compose our own capability info from the device id 
		for (int i = 0; legacy_devices[i].id; i++)
		{
			legacy_device *ldp = &legacy_devices[i];

			if (	ldp->id == pd.product_id && 
				ldp->version_min <= pd.software_version &&
				ldp->version_max >= pd.software_version)
			{
				// found it, creating capability info
				for (int j = 0; ldp->protocols[j].tag ; j++)
				{
					protocol_datatypes_t x;
					for (int k = 0; ldp->protocols[j].types[k] != type_end; k++)
					{
						x.push_back(ldp->protocols[j].types[k]);
					}
					pd.protocol_info[(ldp->protocols[j].id << 8) | ((unsigned char)ldp->protocols[j].tag)] =  x;			   
				}

			
				break;	
			}
		}
		return pd;
	}
	m_got_protocol_info = true;
	if (pid != pid_protocol_array) {
		throw not_possible("Product data packet followed by some packet besides capability protocol");
	}
	if ((sz % 3) != 0) {
		throw not_possible("Capability protocol packet is strange size");
	}


	// we store now the protocols support by the device and the
	// associated data types 

	int lastdata(-1);
	// contains the last found protocol id
	// used to known where to store the datatype infos
 
	for (sint16 i = 0; i < sz; i +=3) {
		char tag = (char) m_buf[i];
		uint16 data = garmin2host16(*(uint16 *) (m_buf + i + 1));
		
		if ( tag != 'D' )
		{
			// is a protocol info
			protocol_datatypes_t x;
			lastdata = (data << 8) | ((unsigned char)tag);
			pd.protocol_info[lastdata] =  x;
		}
		else
		{
			// is a protocol data type
			assert(lastdata != -1);
			pd.protocol_info[lastdata].push_back(data);
		}
		
				
	}
	return pd;
}


//////
// This reads the display bitmap on GPS models that support this.
// Saves the bitmap into a BMP file
//
//
// Screen data is stored as unsigned long format, each long (32 bits)
// contains 16 pixels, with 2 bits per pixel.
//
//  0= white, 1=Light grey, 2=dark grey, 3=black
//
// Please note that display is actually mirrored! We would need to put everything
// into a table in memory before flushing it to disk if we wanted to have the right
// display.
/////
void
application_layer::get_display_bitmap(void) throw (not_possible, timeout)
{
	check_init();
	product_data_type pd = get_product_data();

	uint8 sz;
	uint16 width, height;
	sint16 *buf16 = (sint16 *) m_buf;
	uint32 tlong,q,ptr;
	FILE *outfile;
	// I'm using the BMP format because it's the one used by g7to and I don't
	// have enough time to create PNG or something fancy like this.
	// This is a standard BMP header adapted to our purpose:
	unsigned char bmh[118] = {
		66,  77, 182,  31,   0,   0,   0,   0,   0,   0, 118,   0,   0, 
		0,  40,   0,   0,   0, 160,   0,   0,   0, 100,   0,   0,   0, 
		1,   0,   4,   0,   0,   0,   0,   0,  64,  31,   0,   0,   0, 
		0,   0,   0,   0,   0,   0,   0,  16,   0,   0,   0,  16,   0, 
		0,   0,   0,   0,   0,   0,  99,  99,  99,   0, 181, 181, 181, 
		0, 255, 255, 255,   0, 255, 255, 255,   0, 255, 255, 255,   0, 
		255, 255, 255,   0, 255, 255, 255,   0, 255, 255, 255,   0, 255, 
		255, 255,   0, 255, 255, 255,   0, 255, 255, 255,   0, 255, 255, 
		255,   0, 255, 255, 255,   0, 255, 255, 255,   0, 255, 255, 255,   0 
	}; // bmh[118]

	// Abort any pending/failed transfer:
	abort_transfer();

	buf16[0] = host2garmin16((sint16) cmnd_transfer_screenbitmap);
	m_ll->put_packet(pid_command_data, m_buf, 2);
	
	packet_id pid = m_ll->get_packet(m_buf, sz);
	
	if (pid != pid_display_data) {
		throw not_possible("GPS did not reply with start of screen bitmap data packet");
	}

	// First packet contains screen width and lenght at the following offsets:
	// 16: width
	// 20: height (number of lines)
	width = *(uint16 *) (m_buf + 16);
	height =*(uint16 *) (m_buf + 20);
	cout << "Screen size: " << width << "x" << height << endl;
	
	// Open the screen.bmp file and write the header
	bmh[18]= width;
	bmh[22]= height;
	outfile = fopen("screenshot.bmp","w");
        tlong=width*height/2;
        memcpy(&bmh[34],&tlong,4);
        tlong+=118;
        memcpy(&bmh[2],&tlong,4);
        fwrite(bmh,118,1,outfile);

	// Then parse each subsequent packet and
	// write the data into the BMP file after parsing.
	int pixnum = 0;
	for (int i=0; i< height; i++) {
		// Get next packet with screen line
		pid = m_ll->get_packet(m_buf, sz);
		if (pid != pid_display_data) throw not_possible("GPS did not reply with screen bitmap data packet");		       
		q = 8;
		ptr = *(uint32*) (m_buf+q);
		// Offset 4 is the pixel number starting the line
		pixnum = *(uint32*) (m_buf + 4);
		cout << "Line number " << i << "\r" << flush;
		// Add the pixels to the new pixel bitmap:
		uint32 mask = 0x3;
		uint32 ptr;
		uint16 bit_shift;
		uint8 a,b,c;
		for (int j=0; j< width / 16; j++) {
			mask=0x3L;
			bit_shift=0;
			for(int j=1;j<5;j++) {              // for each byte in long
				for(int k=1;k<3;k++) {      // for each pixel in the byte
					//
					// Our bmp output file has one nibble per pixel
					// first nibble of byte
					//
					ptr = *(uint32*)(m_buf+q);
					a = (~(uint8)(((ptr) & mask) >> bit_shift)) & 3;
					bit_shift += 2;
					mask=mask << 2;
					//
					// second nibble of byte
					//
					b= (~(uint8)(((ptr)&mask)>>bit_shift))&3;
					bit_shift += 2;
					mask=mask << 2;
					//
					// Write line into file:
					//
					c = (a<<4)|b;
					fwrite(&c,1,1,outfile);
				}
			}
			q +=4;  // index pointer to next 4 bytes (unsigned long)
		}
	}
	fclose(outfile);
}

/////////////////

radian_type
application_layer::get_pvt(void) throw (not_possible, timeout)
{
        check_init();
                                                                                                                      
        sint16 *sbuf16 = (sint16 *) m_buf;
                                                                                                                      
        sbuf16[0] = host2garmin16((sint16) cmnd_start_pvt_data);
        m_ll->put_packet(pid_command_data, m_buf, 2);
                                                                                                                      
        uint8 sz;
        packet_id pid = m_ll->get_packet(m_buf, sz);
                                                                                                                      
        if (pid != pid_pvt_data) {
                throw not_possible("GPS did not respond with position packet");
        }
        if (sz != 16) {
                throw not_possible("Position packet is the wrong size");
        }
                                                                                                                      
        double64 *dbuf = (double64 *) m_buf;
        radian_type retval;
        retval.lat = garmin2host64(dbuf[0]);
        retval.lon = garmin2host64(dbuf[1]);
 
        return retval;
}

///////////////////





waypt_vec_t *
application_layer::get_waypoints(void) throw (not_possible, timeout, unsupported_protocol)
{
	check_init();

	product_data_type pd = get_product_data();

	if (!pd.protocol_info.empty() && !pd.has_protocol('A',100))
	{
		throw unsupported_protocol("GPS does not support selected protocol");
	}
	
	uint8 sz;
	sint16 *buf16 = (sint16 *) m_buf;
	char ident[7];
	char cmnt[41];
	ident[6] = cmnt[40] = 0;
	
	buf16[0] = host2garmin16((sint16) cmnd_transfer_wpt);
	m_ll->put_packet(pid_command_data, m_buf, 2);
	
	packet_id pid = m_ll->get_packet(m_buf, sz);
	
	if (pid != pid_records) {
		throw not_possible("GPS did not reply with start of records data packet for waypoint data");
	}
	if (sz != 2) {
		throw not_possible("Waypoint data start of records packet is wrong size");
	}
	
	sint16 num_recs = garmin2host16(buf16[0]);
	waypt_vec_t *waypts = new waypt_vec_t(num_recs);
	bool unsupp = false;
		
	for (sint16 i = 0; i < num_recs; i++) {
		basic_waypt_type &waypt = (*waypts)[i];
		
		pid = m_ll->get_packet(m_buf, sz);
		
		if (pid != pid_wpt_data) {
			delete waypts;
			throw not_possible("GPS did not send waypoint data packet");
		}
		/*
		if (sz < (6 + 8 + 4 + 40)) {
			delete waypts;
			throw not_possible("Waypoint data packet is too small");
		}
		*/
		switch (pd.protocol_datatype('A',100,0))
		{
  		        case 108:
		        case 109:
			{
				D108 my_d108(m_buf);
				waypt << my_d108;
				break;
			}
				
			case 103:
			{
				uint32 *buf32 = (uint32 *) (m_buf + 6);
				waypt.pos.lat = garmin2host32(buf32[0]);
				waypt.pos.lon = garmin2host32(buf32[1]);

				strncpy(ident, (char *) m_buf, 6);
				waypt.id = string(ident);
	
				strncpy(cmnt, (char *) (m_buf + 18), 40);
				waypt.comment = string(cmnt);
				break;
			}
			default:
				unsupp = true; // Cannot just bail out, otherwise the GPS is confused
		}
	}
	if (unsupp)  throw unsupported_protocol("Waypoint Datatype of GPS not supported");

	pid = m_ll->get_packet(m_buf, sz);
	if (pid != pid_xfer_cmplt) {
		delete waypts;
		throw not_possible("GPS did not terminate waypoint data with end of records packet");
	}
	if (garmin2host16(buf16[0]) != cmnd_transfer_wpt) {
		delete waypts;
		throw not_possible("End of records packet does not match original waypoint transfer command");
	}
		
	return waypts;
		
}

/////////////////////

void 
application_layer::send_waypoints(waypt_vec_t *waypts) throw (not_possible, timeout)
{
	check_init();
	
	const	int d103_sz = 6 + 8 + 4 + 40 + 2;

	product_data_type pd = get_product_data();

	if (!(pd.has_protocol('A',100) && pd.protocol_datatype('A',100,0) == 103))
		{
		throw not_possible("Waypoint download not supported for this GPS");
	}

	sint16 *buf16 = (sint16 *) m_buf;
	buf16[0] = host2garmin16(waypts->size());
	
	m_ll->put_packet(pid_records, m_buf, 2);
	
	for (unsigned int i = 0; i < waypts->size(); i++) {
		basic_waypt_type &w = (*waypts)[i];
		
		// clear all fields
		memset(m_buf, 0, d103_sz);
		// set all waypt strings to blank spaces
		memset(m_buf, ' ', 6);
		memset(m_buf + 18, ' ', 40);
		
		// copy in strings, fixing up for GPS; leave bad characters as spaces
		for (size_t j = 0; j < min<size_t>(6, w.id.size()); j++) {
			char c = w.id[j];
			if (isalnum(c)) {
				m_buf[j] = toupper(c);
			}
		}
		for (size_t j = 0; j < min<size_t>(40, w.comment.length()); j++) {
			char c = w.comment[j];
			if (isalnum(c) || c == '-') {
				m_buf[j + 18] = toupper(c);
			}
		}
		
		// copy in position data
		uint32 *buf32 = (uint32 *) (m_buf + 6);
		buf32[0] = host2garmin32(w.pos.lat);
		buf32[1] = host2garmin32(w.pos.lon);
		
		m_ll->put_packet(pid_wpt_data, m_buf, d103_sz);
	}
	
	buf16[0] = host2garmin16(cmnd_transfer_wpt);
	m_ll->put_packet(pid_xfer_cmplt, m_buf, 2);
}

/////////////////////

route_list_t *
application_layer::get_routes() throw (not_possible, timeout)
{
	check_init();
	pd = get_product_data();

	if (!pd.protocol_info.empty() && pd.has_protocol('A',201))
	{
		return get_routes_201();	
	}

	if (!pd.protocol_info.empty() && pd.has_protocol('A',200))
	{
		return get_routes_200();	
	}

	return get_routes_200();
	// give it a try
}
		


route_list_t *
application_layer::get_routes_201() throw (not_possible, timeout, unsupported_protocol)
{
	check_init();

	if (!pd.protocol_info.empty() && !pd.has_protocol('A',201))
	{
		throw unsupported_protocol("GPS does not support selected protocol");
	}
		

	uint16 *buf16 = (uint16 *) m_buf;
	buf16[0] = host2garmin16((sint16) cmnd_transfer_rte);
	m_ll->put_packet(pid_command_data, m_buf, 2);
	uint8 sz;

	packet_id pid = m_ll->get_packet(m_buf, sz);
	if (pid != pid_records) {
		throw not_possible("GPS did not reply with start of records data packet for route data");
	}
	if (sz != 2) {
		throw not_possible("Route data start of records packet is wrong size");
	}
	sint16 num_packets = garmin2host16(buf16[0]);

	// create route list with no routes
	route_list_t *retval = new route_list_t(0);
	
	bool curr_rt_exists = false;
	for (sint16 i = 0; i < num_packets; i++) {
		pid = m_ll->get_packet(m_buf, sz);
		if (pid == pid_rte_hdr) {
			// start new route
			retval->push_back(list<basic_waypt_type>());
			curr_rt_exists = true;
			continue;
		}
		if (pid == pid_rte_link_data) {
			curr_rt_exists = true;
			continue;
		}
		
		if (pid != pid_rte_wpt_data) {
			delete retval;
			throw not_possible("Packet is not route waypoint data or route header");
		}
		if (!curr_rt_exists) {
			delete retval;
			throw not_possible("Route waypoint data packet was not preceded by a route header packet");
		}
		if (sz < (6 + 8 + 2 + 40)) {
			delete retval;
			throw not_possible("Route waypoint data packet is too small");
		}

		basic_waypt_type waypt;

		D108 my_d108(m_buf);
		waypt << my_d108;
		
		// add this waypoint to end of last route in route list.
		retval->back().push_back(waypt);
		
	}
	
	pid = m_ll->get_packet(m_buf, sz);
	if (pid != pid_xfer_cmplt) {
		delete retval;
		throw not_possible("GPS did not terminate route data with end of records packet");
	}
	if (garmin2host16(buf16[0]) != cmnd_transfer_rte) {
		delete retval;
		throw not_possible("End of records packet does not match original route transfer command");
	}
		
	return retval;
		
}
/////////////////////

route_list_t *
application_layer::get_routes_200(void) throw (not_possible, timeout, unsupported_protocol)
{
	check_init();

	product_data_type pd = get_product_data();

	if (!pd.protocol_info.empty() && !pd.has_protocol('A',200))
	{
		throw unsupported_protocol("GPS does not support selected protocol");
	}

	uint16 *buf16 = (uint16 *) m_buf;
	buf16[0] = host2garmin16((sint16) cmnd_transfer_rte);
	m_ll->put_packet(pid_command_data, m_buf, 2);
	uint8 sz;

	packet_id pid = m_ll->get_packet(m_buf, sz);
	if (pid != pid_records) {
		throw not_possible("GPS did not reply with start of records data packet for route data");
	}
	if (sz != 2) {
		throw not_possible("Route data start of records packet is wrong size");
	}
	sint16 num_packets = garmin2host16(buf16[0]);

	// create route list with no routes
	route_list_t *retval = new route_list_t(0);
	
	bool curr_rt_exists = false;
	for (sint16 i = 0; i < num_packets; i++) {
		pid = m_ll->get_packet(m_buf, sz);
		if (pid == pid_rte_hdr) {
			// start new route
			retval->push_back(list<basic_waypt_type>());
			curr_rt_exists = true;
			continue;
		}
		
		if (pid != pid_rte_wpt_data) {
			delete retval;
			throw not_possible("Packet is not route waypoint data or route header");
		}
		if (!curr_rt_exists) {
			delete retval;
			throw not_possible("Route waypoint data packet was not preceded by a route header packet");
		}
		if (sz < (6 + 8 + 2 + 40)) {
			delete retval;
			throw not_possible("Route waypoint data packet is too small");
		}
		
		semicircle_type *sp = (semicircle_type *) (m_buf + 6);
		sp->lat = garmin2host32(sp->lat);
		sp->lon = garmin2host32(sp->lon);

		basic_waypt_type waypt;

		char ident[7], cmnt[41];
		ident[6] = cmnt[40] = 0;
		strncpy(ident, (char *) m_buf, 6);
		waypt.id = string(ident);

		waypt.pos = *sp;
		
		strncpy(cmnt, (char *) m_buf + 14, 40);
		waypt.comment = string(cmnt);
		
		// add this waypoint to end of last route in route list.
		retval->back().push_back(waypt);
		
	}
	
	pid = m_ll->get_packet(m_buf, sz);
	if (pid != pid_xfer_cmplt) {
		delete retval;
		throw not_possible("GPS did not terminate route data with end of records packet");
	}
	if (garmin2host16(buf16[0]) != cmnd_transfer_rte) {
		delete retval;
		throw not_possible("End of records packet does not match original route transfer command");
	}
		
	return retval;
		
}

/////////////////////

void 
application_layer::send_routes(route_list_t *routes) throw (not_possible, timeout)
{
	check_init();
		
	product_data_type pd = get_product_data();

	if (!(pd.has_protocol('A',200) && pd.protocol_datatype('A',200,1) == 103))
	{
		throw not_possible("Waypoint download not supported for this GPS");
	}

	const	int d103_sz = 6 + 8 + 4 + 40 + 2;
	
	// how many data packets?
	sint16 num_recs = 0;
	route_list_t::const_iterator llw;
	for (llw = routes->begin(); llw != routes->end(); llw++) {
		num_recs += llw->size();
	}
	num_recs += routes->size(); // plus 1 header packet for each route
	
	sint16 *buf16 = (sint16 *) m_buf;
	buf16[0] = host2garmin16(num_recs);
	
	m_ll->put_packet(pid_records, m_buf, 2);
	
	for (llw = routes->begin(); llw != routes->end(); llw++) {
		
		// put route header
		m_buf[0] = 10;
		char *test = "TESTING COMMENT     "; 
		for (int k = 0; k < 20; k++) {
			m_buf[k + 1] = test[k];
		}
		m_ll->put_packet(pid_rte_hdr, m_buf, 21);
		
		// put route waypoints
		route_t::const_iterator lw;
		for (lw = llw->begin(); lw != llw->end(); lw++) {
			memset(m_buf, 0, d103_sz);
			// set all waypt strings to blank spaces
			memset(m_buf, ' ', 6);
			memset(m_buf + 18, ' ', 40);
		
			// copy in strings, fixing up for GPS; leave bad characters as spaces
			for (size_t j = 0; j < min<size_t>(6, lw->id.size()); j++) {
				char c = lw->id[j];
				if (isalnum(c)) {
					m_buf[j] = toupper(c);
				}
			}
			for (size_t j = 0; j < min<size_t>(40, lw->comment.length()); j++) {
				char c = lw->comment[j];
				if (isalnum(c) || c == '-') {
					m_buf[j + 18] = toupper(c);
				}
			}
		
			// copy in position data
			uint32 *buf32 = (uint32 *) (m_buf + 6);
			buf32[0] = host2garmin32(lw->pos.lat);
			buf32[1] = host2garmin32(lw->pos.lon);
		
			m_ll->put_packet(pid_rte_wpt_data, m_buf, d103_sz);
		}
	}
	buf16[0] = host2garmin16(cmnd_transfer_rte);
	m_ll->put_packet(pid_xfer_cmplt, m_buf, 2);
}
	
/////////////////////

void
application_layer::send_track_logs(track_list_t *tracks) throw (not_possible, timeout)
{
	check_init();

	// how many track points?
	sint16 num_recs = 0;
	track_list_t::const_iterator llti;
	for (llti = tracks->begin(); llti != tracks->end(); llti++) {
		num_recs += llti->size();
	}
	
	sint16 *sbuf16 = (sint16 *) m_buf;
	sbuf16[0] = host2garmin16(num_recs);
		
	m_ll->put_packet(pid_records, m_buf, 2);
	
	for (llti = tracks->begin(); llti != tracks->end(); llti++) {
		track_t::const_iterator lti;
		bool first = true;
		m_buf[8 + 4] = 1;
		for (lti = llti->begin(); lti != llti->end(); lti++, first = false) {
		
			uint32 *buf32 = (uint32 *) m_buf;
			buf32[0] = host2garmin32(lti->pos.lat);
			buf32[1] = host2garmin32(lti->pos.lon);
			
			// GPS ignores time field, leave it as 0 
			
			m_ll->put_packet(pid_trk_data, m_buf, 16);
			
			// set new_track element at first point of each segment, but not for other
			// points in track segment
			if (first) {
				m_buf[8 + 4] = 0;
			}
		}
	}
	
	sbuf16[0] = host2garmin16(cmnd_transfer_trk);
	m_ll->put_packet(pid_xfer_cmplt, m_buf, 2);
}

/////////////////////

track_list_t *
application_layer::get_track_logs(void) throw (not_possible, timeout, unsupported_protocol)
{
	check_init();
	product_data_type pd = get_product_data();

	if (!pd.protocol_info.empty() && pd.has_protocol('A',301))
	{
		return get_track_logs_301();	
	}

	if (!pd.protocol_info.empty() && pd.has_protocol('A',300))
	{
		return get_track_logs_300();	
	}

	return get_track_logs_300();	
	// give it a try
}

track_list_t *
application_layer::get_track_logs_300(void) throw (not_possible, timeout, unsupported_protocol)
{
	check_init();

	product_data_type pd = get_product_data();

	if (!pd.protocol_info.empty() && !pd.has_protocol('A',300))
	{
		throw unsupported_protocol("GPS does not support selected protocol");
	}
	
	uint8 sz;
	sint16 *buf16 = (sint16 *) m_buf;
	
	buf16[0] = host2garmin16((sint16) cmnd_transfer_trk);
	m_ll->put_packet(pid_command_data, m_buf, 2);
	
	packet_id pid = m_ll->get_packet(m_buf, sz);
	
	if (pid != pid_records) {
		throw not_possible("GPS did not reply with start of records data packet for track log data");
	}
	if (sz != 2) {
		throw not_possible("Track log data start of records packet is wrong size");
	}
	
	sint16 num_recs = garmin2host16(buf16[0]);
	track_list_t *tracks = new track_list_t(0);
		
	for (sint16 i = 0; i < num_recs; i++) {
		pid = m_ll->get_packet(m_buf, sz);
		
		if (pid != pid_trk_data) {
			delete tracks;
			throw not_possible("GPS did not send track point data packet");
		}

		if (sz < (8 + 4 + 1)) {
			delete tracks;
			throw not_possible("Track point data packet is too small");
		}
		
		bool new_trk = m_buf[8 + 4];
		if (new_trk || i == 0) {
			tracks->push_back(list<track_point_type>());
		}		
		
		semicircle_type *sp = (semicircle_type *) m_buf;
		sp->lat = garmin2host32(sp->lat);
		sp->lon = garmin2host32(sp->lon);

		track_point_type track_point;
		track_point.pos = *sp;
		track_point.new_track = new_trk;
		uint32 *buf32 = (uint32 *) m_buf;
		
		struct tm my_tm;
		memset(&my_tm, 0, sizeof(struct tm));
		my_tm.tm_sec = garmin2host32(buf32[2]);
		my_tm.tm_year = 89;
		my_tm.tm_mday = 31; // 1 based counting
		my_tm.tm_mon  = 11; // 0 based counting
		my_tm.tm_isdst = -1; // unknown
		
		track_point.time = mktime(&my_tm);
		
		tracks->back().push_back(track_point);
	}
	
	pid = m_ll->get_packet(m_buf, sz);
	if (pid != pid_xfer_cmplt) {
		delete tracks;
		throw not_possible("GPS did not terminate track log data with end of records packet");
	}
	if (garmin2host16(buf16[0]) != cmnd_transfer_trk) {
		delete tracks;
		throw not_possible("End of records packet does not match original track log transfer command");
	}
		
	return tracks;
}

track_list_t *
application_layer::get_track_logs_301(void) throw (not_possible, timeout, unsupported_protocol)
{
	check_init();

	product_data_type pd = get_product_data();

	if (!pd.protocol_info.empty() && !pd.has_protocol('A',301))
	{
		throw unsupported_protocol("GPS does not support selected protocol");
	}
	
	uint8 sz;
	sint16 *buf16 = (sint16 *) m_buf;
	
	buf16[0] = host2garmin16((sint16) cmnd_transfer_trk);
	m_ll->put_packet(pid_command_data, m_buf, 2);
	
	packet_id pid = m_ll->get_packet(m_buf, sz);
	
	if (pid != pid_records) {
		throw not_possible("GPS did not reply with start of records data packet for track log data");
	}
	if (sz != 2) {
		throw not_possible("Track log data start of records packet is wrong size");
	}
	
	sint16 num_recs = garmin2host16(buf16[0]);
	track_list_t *tracks = new track_list_t(0);

		
	for (sint16 i = 0; i < num_recs; i++) {
		pid = m_ll->get_packet(m_buf, sz);

		
		if (pid == pid_trk_hdr) {
			cout << "New track: " << m_buf + 2 << endl;
			continue;
		}

		if (pid != pid_trk_data) {
			delete tracks;
			throw not_possible("GPS did not send track point data packet");
		}

		if (sz < (8 + 4 + 1)) {
			delete tracks;
			throw not_possible("Track point data packet is too small");
		}
		
		bool new_trk = m_buf[8 + 4 + 8];
		if (new_trk || i == 0) {
			tracks->push_back(list<track_point_type>());
		}		
	
		track_point_type track_point;
		D301 my_d301(m_buf);	

		track_point << my_d301;
		tracks->back().push_back(track_point);
	}
	
	pid = m_ll->get_packet(m_buf, sz);
	if (pid != pid_xfer_cmplt) {
		delete tracks;
		throw not_possible("GPS did not terminate track log data with end of records packet");
	}
	if (garmin2host16(buf16[0]) != cmnd_transfer_trk) {
		delete tracks;
		throw not_possible("End of records packet does not match original track log transfer command");
	}
		
	return tracks;
}

/////////////////

prox_waypt_vec_t *
application_layer::get_proximity_waypoints(void) throw (not_possible, timeout, unsupported_protocol)
{
	check_init();

	product_data_type pd = get_product_data();

	if (!pd.protocol_info.empty() && !pd.has_protocol('A',400))
	{
		throw unsupported_protocol("GPS does not support selected protocol");
	}
	
	uint8 sz;
	sint16 *buf16 = (sint16 *) m_buf;
	
	char ident[7];
	char cmnt[41];
	ident[6] = cmnt[40] = 0;
	
	buf16[0] = host2garmin16((sint16) cmnd_transfer_prx);
	m_ll->put_packet(pid_command_data, m_buf, 2);
	
	packet_id pid = m_ll->get_packet(m_buf, sz);
	
	if (pid != pid_records) {
		throw not_possible("GPS did not reply with start of records data packet for proximity waypoint data");
	}
	if (sz != 2) {
		throw not_possible("Proximity waypoint data start of records packet is wrong size");
	}
	
	sint16 num_recs = garmin2host16(buf16[0]);
	prox_waypt_vec_t *waypts = new prox_waypt_vec_t(num_recs);
		
	for (sint16 i = 0; i < num_recs; i++) {
		proximity_waypt_type &waypt = (*waypts)[i];
		
		pid = m_ll->get_packet(m_buf, sz);
		
		if (pid != pid_prx_wpt_data) {
			delete waypts;
			throw not_possible("GPS did not send proximity waypoint data packet");
		}
		if (sz < (6 + 8 + 4 + 40 + 2 + 4)) {
			delete waypts;
			throw not_possible("Proximity waypoint data packet is too small");
		}
		
		uint32 *buf32 = (uint32 *) (m_buf + 6);
		waypt.waypt.pos.lat = garmin2host32(buf32[0]);
		waypt.waypt.pos.lon = garmin2host32(buf32[1]);

		strncpy(ident, (char *) m_buf, 6);
		waypt.waypt.id = string(ident);
		
		strncpy(cmnt, (char *) m_buf + 18, 40);
		waypt.waypt.comment = string(cmnt);
		
		buf32 = (uint32 *) (m_buf + sz - 4);
		*buf32 = garmin2host32(buf32[0]);
		float32 *dist = (float32 *) buf32;
		waypt.dist = *dist;
	}
	
	pid = m_ll->get_packet(m_buf, sz);
	if (pid != pid_xfer_cmplt) {
		delete waypts;
		throw not_possible("GPS did not terminate proximity waypoint data with end of records packet");
	}
	if (garmin2host16(buf16[0]) != cmnd_transfer_prx) {
		delete waypts;
		throw not_possible("End of records packet does not match original proximity waypoint transfer command");
	}
		
	return waypts;
		
}

/////////////////////

void
application_layer::send_proximity_waypoints(prox_waypt_vec_t *prx_waypts) throw (not_possible, timeout)
{
	check_init();
	
	product_data_type pd = get_product_data();

	if (!(pd.has_protocol('A',400) && pd.protocol_datatype('A',400,0) == 403))
	{
		throw not_possible("Waypoint download not supported for this GPS");
	}
	const	int d103_sz = 6 + 8 + 4 + 40 + 2;
	
	sint16 *buf16 = (sint16 *) m_buf;
	buf16[0] = host2garmin16(prx_waypts->size());
	
	m_ll->put_packet(pid_records, m_buf, 2);
	
	for (unsigned int i = 0; i < prx_waypts->size(); i++) {
		basic_waypt_type &w = (*prx_waypts)[i].waypt;
		
		// clear all fields
		memset(m_buf, 0, d103_sz + 4);
		// set all waypt strings to blank spaces
		memset(m_buf, ' ', 6);
		memset(m_buf + 18, ' ', 40);
		
		// copy in strings, fixing up for GPS; leave bad characters as spaces
		for (size_t j = 0; j < min<size_t>(6, w.id.size()); j++) {
			char c = w.id[j];
			if (isalnum(c)) {
				m_buf[j] = toupper(c);
			}
		}
		for (size_t j = 0; j < min<size_t>(40, w.comment.length()); j++) {
			char c = w.comment[j];
			if (isalnum(c) || c == '-') {
				m_buf[j + 18] = toupper(c);
			}
		}
		
		// copy in position data
		uint32 *buf32 = (uint32 *) (m_buf + 6);
		buf32[0] = host2garmin32(w.pos.lat);
		buf32[1] = host2garmin32(w.pos.lon);
		
		// copy in proximity distance
		buf32 = (uint32 *) (m_buf + d103_sz);
		float32 d = (*prx_waypts)[i].dist;
		uint32 *t32 = (uint32 *) &d;
		*buf32 = host2garmin32(*t32);
		
		m_ll->put_packet(pid_prx_wpt_data, m_buf, d103_sz + 4);
	}
	
	buf16[0] = host2garmin16(cmnd_transfer_prx);
	m_ll->put_packet(pid_xfer_cmplt, m_buf, 2);

}


/////////////////////

void
application_layer::turn_off_gps(void) throw (not_possible, timeout)
{
	check_init();
	
	sint16 *buf16 = (sint16 *) m_buf;
	
	buf16[0] = host2garmin16((sint16) cmnd_turn_off_pwr);
	m_ll->put_packet(pid_command_data, m_buf, 2);
	
}

/////////////////

time_t 
application_layer::get_date_time(void) throw (not_possible, timeout)
{
	check_init();

	sint16 *sbuf16 = (sint16 *) m_buf;
	
	sbuf16[0] = host2garmin16((sint16) cmnd_transfer_time);
	m_ll->put_packet(pid_command_data, m_buf, 2);
	
	uint8 sz;
	packet_id pid = m_ll->get_packet(m_buf, sz);
	
	if (pid != pid_date_time_data) {
		throw not_possible("GPS did not respond with date and time packet");
	}
	if (sz != 8) {
		throw not_possible("Date and time packet is the wrong size");
	}
	
	struct tm my_tm;
	memset(&my_tm, 0, sizeof(struct tm));
	
	my_tm.tm_mon = m_buf[0] - 1;
	my_tm.tm_mday = m_buf[1];
	uint16 *ubuf16 = (uint16 *) m_buf;
	my_tm.tm_year = garmin2host16(ubuf16[1]) - 1900;
	my_tm.tm_hour = garmin2host16(sbuf16[2]);
	my_tm.tm_min = m_buf[6];
	my_tm.tm_sec = m_buf[7];
	
	my_tm.tm_isdst = -1;
	
	return mktime(&my_tm);
}

/////////////////////

void 
application_layer::send_date_time(time_t t) throw (bad_time, not_possible, timeout)
{
	check_init();

	struct tm my_tm = *gmtime(&t);
	if (my_tm.tm_year < 90) {
		throw bad_time();
	}
	
	m_buf[0] = my_tm.tm_mon + 1;
	m_buf[1] = my_tm.tm_mday;
	uint16 *buf16 = (uint16 *) m_buf;;
	buf16[1] = host2garmin16(my_tm.tm_year + 1900);
	sint16 *sbuf16 = (sint16 *) buf16;
	sbuf16[2] = host2garmin16(my_tm.tm_hour);
	m_buf[6] = my_tm.tm_min;
	m_buf[8] = my_tm.tm_sec;
	
	m_ll->put_packet(pid_date_time_data, m_buf, 8);
}

//////////////////

radian_type
application_layer::get_position(void) throw (not_possible, timeout)
{
	check_init();

	sint16 *sbuf16 = (sint16 *) m_buf;
	
	sbuf16[0] = host2garmin16((sint16) cmnd_transfer_posn);
	m_ll->put_packet(pid_command_data, m_buf, 2);
	
	uint8 sz;
	packet_id pid = m_ll->get_packet(m_buf, sz);
	
	if (pid != pid_position_data) {
		throw not_possible("GPS did not respond with position packet");
	}
	if (sz != 16) {
		throw not_possible("Position packet is the wrong size");
	}
	
	double64 *dbuf = (double64 *) m_buf;
	radian_type retval;
	retval.lat = garmin2host64(dbuf[0]);
	retval.lon = garmin2host64(dbuf[1]);
	
	return retval;
}

/////////////////////

void 
application_layer::send_position(radian_type pos) throw (not_possible, timeout)
{
	check_init();

	double64 *dbuf = (double64 *) m_buf;
	dbuf[0] = host2garmin64(pos.lat);
	dbuf[1] = host2garmin64(pos.lon);
	
	m_ll->put_packet(pid_position_data, m_buf, 16);
}

//////////////////

} // namespace
