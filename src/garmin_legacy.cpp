// -*- mode: c++; c-basic-offset: 8; -*-
// $Id$
// garmin_legacy.cp
// Danilo Beuche
// May 26, 1998

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

#include "garmin_legacy.h"

namespace garmin {

	legacy_protocols dev73 =
	{
		{ 'L', 	1, { type_end } },
		{ 'A', 10, { type_end } },
		{ 'A', 100, { 103, type_end } },
		{ 'A', 200, { 201, 103, type_end } },
		{ 'A', 300, { 301, type_end } },
		{ 'A', 500, { 501, type_end } },
		{ 'A', 600, { 600, type_end } },
		{ 'A', 700, { 700, type_end } },
		{ 'A', 000, { type_end } },
		{ 'P', 000, { type_end } },
		{  0, 000, { 0 } },
	};
	legacy_protocols dev96 =
	{
		{ 'L', 	1, { type_end } },
		{ 'A', 10, { type_end } },
		{ 'A', 100, { 103, type_end } },
		{ 'A', 200, { 201, 103, type_end } },
		{ 'A', 300, { 301, type_end } },
		{ 'A', 400, { 403, type_end } },
		{ 'A', 500, { 501, type_end } },
		{ 'A', 600, { 600, type_end } },
		{ 'A', 700, { 700, type_end } },
		{ 'A', 000, { type_end } },
		{ 'P', 000, { type_end } },
		{  0, 000, { 0 } },
	};

	// Below is am example from a capability return device
	// it is an eTrex version 2.06
	legacy_protocols dev130 =
	{
		{ 'L', 	1, { type_end } },
		{ 'A', 10, { type_end } },
		{ 'A', 100, { 108, type_end } },
		{ 'A', 201, { 202, 108, 201, type_end } },
		{ 'A', 301, { 310, 301, type_end } },
		{ 'A', 500, { 501, type_end } },
		{ 'A', 600, { 600, type_end } },
		{ 'A', 700, { 700, type_end } },
		{ 'A', 800, { 800, type_end } },
		{ 'A', 000, { type_end } },
		{ 'P', 000, { type_end } },
		{  0, 000, { 0 } },
	};


	struct legacy_device legacy_devices[11] =
	{
		{ 73, min_version, max_version, dev73 },
		{ 77, 301, 349, dev96 },
		{ 77, 361, max_version, dev96 },
		{ 87, min_version, max_version, dev96 },
		{ 95, min_version, max_version, dev96 },
		{ 96, min_version, max_version, dev96 },
		{ 100, min_version, max_version, dev96 },
		{ 105, min_version, max_version, dev96 },
		{ 106, min_version, max_version, dev96 },
		{ 130, min_version, max_version, dev130 },
		{ 0, 0, 0 , 0 }
	};

} // namespace
