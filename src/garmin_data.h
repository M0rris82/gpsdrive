// -*- mode: c++; c-basic-offset: 8; -*-
// $Id$
// garmin_data.h
// Danilo Beuche

// Copyright (C) 2000 Danilo Beuche
// <danilo@topmail.de>
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

#ifndef _garmin_data
#define _garmin_data

#include "garmin_types.h"

namespace garmin {

class D108 {
	unsigned char *buf;
public:
	enum offsets {
		wpt_class 	= 0,
		color 		= 1,
		dspl  		= 2,
		attr  		= 3,
		smbl  		= 4,
		subclass 	= 6,
		posn     	= 24,
		alt		= 32,
		dpth		= 36,
		dist		= 40,
		state		= 44,
		cc		= 46,
		ident		= 52
	};

	enum WaypointClass { USER_WPT = 0x00 };
	enum Color { Black };

	D108(unsigned char *nbuf): buf(nbuf) {}

	WaypointClass 	Wpt_class	() const { return (WaypointClass)buf[wpt_class]; }
	Color         	Wpt_color    	() const { return (Color)buf[color]; }
	float	      	Wpt_altitude	() const { float alti;
                                                   alti = *(float*)(buf + alt);
						   if (alti > 1.0e20) // a GPS will never go that high
							   return 0;
	                                           else return alti;}
	semicircle_type Wpt_position	() const;
	symbol_type     Wpt_symbol      () const { return (symbol_type)buf[smbl]; };
	char *		Wpt_ident	() const { return (char*)buf + ident; }	
	char *		Wpt_comment	() const { return Wpt_ident() + strlen(Wpt_ident()) + 1 ; }
};

basic_waypt_type& operator<<(basic_waypt_type& , const D108&); 
	
class D301 {
	unsigned char *buf;
public:
	enum offsets {
		posn     	= 0,
		time		= 8,
		alt		= 12,
		dpth		= 16,
		new_trk 	= 20
	};

	D301(unsigned char *nbuf): buf(nbuf) {}

	float	      	Trk_altitude	() const { return *(float*)(buf + alt); }
	float	      	Trk_depth	() const { return *(float*)(buf + dpth); }
	semicircle_type Trk_position	() const;
	uint32		Trk_time	() const { return *(uint32*)(buf + time); }
	bool		Trk_newtrack	() const { return buf[new_trk]; }	
};

track_point_type& operator<<(track_point_type& , const D301&); 

} // namespace

#endif // _garmin_data
