// -*- mode: c++; c-basic-offset: 8; -*-
// $Id$
// garmin_d108.cp
// Danilo Beuche

// Copyright (C) 2000	Danilo Beuche 
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

#include "garmin_data.h" 
#include "garmin_util.h" 

namespace garmin {

using namespace std;


semicircle_type D108::Wpt_position	() const { semicircle_type ret = *(semicircle_type*)(buf + posn);
						   ret.lat = garmin2host32(ret.lat);
						   ret.lon = garmin2host32(ret.lon);
						   return ret; }

semicircle_type D301::Trk_position	() const { semicircle_type ret = *(semicircle_type*)(buf + posn);
						   ret.lat = garmin2host32(ret.lat);
						   ret.lon = garmin2host32(ret.lon);
						   return ret; }


basic_waypt_type& operator<<(basic_waypt_type& out, const D108& in)
{
	out.id = string(in.Wpt_ident());
	out.pos = in.Wpt_position();
	out.comment = string(in.Wpt_comment());
	out.symbol = in.Wpt_symbol();
	out.altitude = in.Wpt_altitude();
	return out;
}
	
track_point_type& operator<<(track_point_type& out, const D301& in)
{
	out.pos = in.Trk_position();
	out.new_track = in.Trk_newtrack();

	struct tm my_tm;
	memset(&my_tm, 0, sizeof(struct tm));
	my_tm.tm_sec = garmin2host32(in.Trk_time());
	my_tm.tm_year = 89;
	my_tm.tm_mday = 31; // 1 based counting
	my_tm.tm_mon = 11; // 1 based counting
	my_tm.tm_isdst = -1; // unknown
		
	out.time = mktime(&my_tm);
	return out;
}
	
} // namespace
