// $Id$
// garmin_util.h
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

#ifndef _garmutil
#define _garmutil

namespace garmin {

// byte swap routines
uint16
host2garmin16(uint16 x);

uint16
garmin2host16(uint16 x);

uint32
host2garmin32(uint32 x);

uint32
garmin2host32(uint32 x);

double64
host2garmin64(double64 x);

double64
garmin2host64(double64 x);


// position unit conversions
radian_type
semicircle2radian(semicircle_type pos);

degree_type
semicircle2degree(semicircle_type pos);

semicircle_type
radian2semicircle(radian_type pos);

semicircle_type
degree2semicircle(degree_type pos);

degree_type
radian2degree(radian_type pos);

radian_type
degree2radian(degree_type pos);

double64
rad2deg(double64 r);

double64
deg2rad(double64 d);

} // namespace

#endif // _garmutil
