// -*- mode: c++; c-basic-offset: 8; -*-
// $Id$
// garmin_util.cp
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

#include "garmin_types.h"
#include "garmin_util.h"
#include <cmath>

#include <iostream>

#if macintosh
#include <limits>
namespace garmin {
static const garmin::double64 pi = M_PI; // ???
static const garmin::double64 maxval_sint32 = (garmin::double64) std::numeric_limits<garmin::sint32>::max();
}
#else
#include <limits.h> // g++ has no <limits> header?
namespace garmin {
static const garmin::double64 pi = M_PI;
static const garmin::double64 maxval_sint32 = INT_MAX;
}
#endif

namespace garmin {

#ifdef DO_BYTE_SWAP // big-endian, garmin wants little-endian

uint16
garmin2host16(uint16 x)
{ return host2garmin16(x); }

uint16
host2garmin16(uint16 x)
{
	union {
		struct {
			uint8 lo;
			uint8 hi;
		} s;
		uint16 i;
	} u;
	
	u.i = x;
	uint8 t = u.s.lo;
	u.s.lo = u.s.hi;
	u.s.hi = t;
	
	return u.i;
}

uint32
garmin2host32(uint32 x)
{ return host2garmin32(x); }

uint32
host2garmin32(uint32 x)
{
	union {
		struct {
			uint16 lo;
			uint16 hi;
		} s;
		uint32 i;
	} u;
	
	u.i = x;
	uint16 t = u.s.lo;
	u.s.lo = host2garmin16(u.s.hi);
	u.s.hi = host2garmin16(t);
	
	return u.i;
}

double64
garmin2host64(double64 x)
{ return host2garmin64(x); }

double64
host2garmin64(double64 x)
{
	union {
		struct {
			uint32 lo;
			uint32 hi;
		} s;
		double64 d;
	} u;
	
	u.d = x;
	uint32 t = u.s.lo;
	u.s.lo = host2garmin32(u.s.hi);
	u.s.hi = host2garmin32(t);
	
	return u.d;
}

#else

uint16
garmin2host16(uint16 x) { return x; }

uint16
host2garmin16(uint16 x) { return x; }

uint32
garmin2host32(uint32 x) { return x; }

uint32
host2garmin32(uint32 x) { return x; }

double64
host2garmin64(double64 x) { return x; }

double64
garmin2host64(double64 x) { return x; }

#endif  // DO_BYTE_SWAP

////////////////

radian_type
semicircle2radian(semicircle_type pos)
{
	radian_type d;
	d.lat = pi * (pos.lat / maxval_sint32);
	d.lon = pi * (pos.lon / maxval_sint32);

	return d;
}


degree_type
semicircle2degree(semicircle_type pos)
{
	radian_type d = semicircle2radian(pos);
	degree_type d2;
	d2.lat = rad2deg(d.lat);
	d2.lon = rad2deg(d.lon);
	return d2;
}

semicircle_type
radian2semicircle(radian_type pos)
{
	semicircle_type s;
	s.lat = (sint32) ((pos.lat / pi) * maxval_sint32);
	s.lon = (sint32) ((pos.lon / pi) * maxval_sint32);

	return s;
}

semicircle_type
degree2semicircle(degree_type pos)
{
	radian_type d;
	d.lat = deg2rad(pos.lat);
	d.lon = deg2rad(pos.lon);
	return radian2semicircle(d);
}

degree_type
radian2degree(radian_type pos)
{
	degree_type p2;
	p2.lat = rad2deg(pos.lat);
	p2.lon = rad2deg(pos.lon);
	return p2;
}

radian_type
degree2radian(degree_type pos)
{
	radian_type p2;
	p2.lat = rad2deg(pos.lat);
	p2.lon = rad2deg(pos.lon);
	return p2;
}

////////////////

double64
rad2deg(double64 r)
{
	return r * 180 / pi;
}

double64
deg2rad(double64 d)
{
	return d * pi / 180;
}

//////////////////
} // namespace
