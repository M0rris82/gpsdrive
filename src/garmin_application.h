// $Id$
// garmin_application.h
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

#ifndef _garapph
#define _garapph

#include "garmin_link.h"
#include "garmin_types.h"
#include <vector>
#include <list>

namespace garmin {

  typedef std::vector<basic_waypt_type> waypt_vec_t;

  typedef std::vector<proximity_waypt_type> prox_waypt_vec_t;

  typedef std::list<basic_waypt_type> route_t;
  typedef std::list<route_t> route_list_t;

  typedef std::list<track_point_type> track_t;
  typedef std::list<track_t> track_list_t;

class application_layer 
{
  // NOTE: for routines which return a pointer to a list or vector,
  // the user is responsible for deleting that list or vector.
public:

	application_layer() : m_init(false), m_ll(0) { }
	application_layer(const application_layer &) : m_init(false), m_ll(0) { }
	
	application_layer(link_layer *ll) : m_init(true), m_ll(ll) { }
	
	void 
	init(link_layer *ll)
	{ m_init = true; m_ll = ll; }
	
	// protocol A000 -- product data (protocol A001 is
	// automatically initiated by GPS after protocol A000 in
	// products which support A001, and is partially implemented
	// here, i.e. we expect A001).
	product_data_type 
	get_product_data(void) throw (not_possible, timeout);
	
	
	// protocol A100 -- waypoints
	waypt_vec_t *
	get_waypoints(void) throw (not_possible, timeout, unsupported_protocol);
	
	void 
	send_waypoints(waypt_vec_t *waypts) throw (not_possible, timeout);
	// only works on units that use the D103 waypoint type (GPS 12, GPS 12 XL, GPS 48, 
	// GPS II Plus, GPS 128).
	
	
	// protocol A2xx -- routes
	// although the GPS may identify routes with a number, this function returns
	// a list of anonymous routes, where each route is a list of waypoints.
	// it checks which protocol type the device supports and choose the right 
	// protocol method
	route_list_t *
	get_routes(void) throw (not_possible, timeout);

	// protocol A200 -- routes
	// although the GPS may identify routes with a number, this function returns
	// a list of anonymous routes, where each route is a list of waypoints.
	route_list_t *
	get_routes_200(void) throw (not_possible, timeout, unsupported_protocol);
	
	// protocol A201 -- routes
	// although the GPS may identify routes with a number, this function returns
	// a list of anonymous routes, where each route is a list of waypoints.
	route_list_t *
	get_routes_201(void) throw (not_possible, timeout, unsupported_protocol);

	void 
	send_routes(route_list_t *routes) throw (not_possible, timeout); 
	// only works on same units as send_waypoints
	
	// protocol A3xx -- track logs
	// returns list of track log sengments, where each track log segment is a list 
	// of track points.
	track_list_t *
	get_track_logs(void) throw (not_possible, timeout, unsupported_protocol);

	// protocol A300 -- track logs
	// returns list of track log sengments, where each track log segment is a list 
	// of track points.
	track_list_t *
	get_track_logs_300(void) throw (not_possible, timeout, unsupported_protocol);
	
	// protocol A301 -- track logs
	// returns list of track log sengments, where each track log segment is a list 
	// of track points.
	track_list_t *
	get_track_logs_301(void) throw (not_possible, timeout, unsupported_protocol);

	void
	send_track_logs(track_list_t *tracks) throw (not_possible, timeout); 
	
	
	// protocol A400 -- proximity waypoints
	prox_waypt_vec_t *
	get_proximity_waypoints(void) throw (not_possible, timeout, unsupported_protocol);
	
	void
	send_proximity_waypoints(prox_waypt_vec_t *prx_waypts) throw (not_possible, timeout);
	// only works on same units as send_waypoints
	
	
	// protocol A500 -- almanac
	
	
	// protocol A600 -- date/time
	std::time_t 
	get_date_time(void) throw (not_possible, timeout);
	// returns UTC time_t of current GPS time
	
	void 
	send_date_time(std::time_t t) throw (bad_time, not_possible, timeout); // buggy?
	// expects local time_t
	// initialize GPS time
	
	
	// protocol A700 -- position
	radian_type 
	get_position(void) throw (not_possible, timeout);
	// get current GPS position
	
	void 
	send_position(radian_type pos) throw (not_possible, timeout);
	// initialize GPS with current position
	
	// protocol A800 -- realtime position, velocity, time
//Fritz works on it
        radian_type
        get_pvt(void) throw (not_possible, timeout);

	
	// misc	
	void 
	turn_off_gps(void) throw (not_possible, timeout);

	void
	  get_display_bitmap(void) throw (not_possible, timeout);

	void
	  abort_transfer(void) throw (timeout);

private:
	bool m_init;
	bool m_got_protocol_info;
	product_data_type pd;
	link_layer *m_ll;
	uint8 m_buf[255];
};

} // namespace

#endif // _garapph
