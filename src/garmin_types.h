// -*- mode: c++; c-basic-offset: 8; -*-
// $Id$
// garmin_types.h
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

#ifndef _gartype
#define _gartype

#include <string>
#include <vector>
#include <map>
#include <ctime>

namespace garmin {

#if linux // yeah at least this will work on my i386
#include <linux/types.h>
#endif

#if macintosh
#include <MacTypes.h>

typedef UInt8 uint8; // garmin char
typedef SInt8 sint8;

typedef UInt16 uint16; // garmin int
typedef SInt16 sint16;

typedef UInt32 uint32; // garmin long
typedef SInt32 sint32;

typedef Float32 float32; // garmin float
typedef Float64 double64; // garmin double

#else

typedef unsigned char uint8; // garmin char
typedef char sint8;

typedef unsigned short uint16; // garmin int
typedef short sint16;

typedef unsigned long uint32; // garmin long
typedef long sint32;

typedef float float32; // garmin float
typedef double double64; // garmin double

#endif

// position in semicircles; 2^31 semicircles == 180 degrees.
// north, east are positive; south, west negative.
class semicircle_type {
public:
	sint32 lat;
	sint32 lon;
};

// position in radians, sign conventions as above.
struct radian_type {
	double64 lat; 
	double64 lon;
};

// for convenience
struct degree_type {
	double64 lat;
	double64 lon;
};


typedef std::vector<uint16> protocol_datatypes_t;
struct product_data_type {
	sint16 product_id;
	sint16 software_version;
	std::string product_description;
	std::map<uint32,protocol_datatypes_t> protocol_info; 

	bool has_protocol(const unsigned char tag, uint16 id) { return protocol_info.find( (id << 8) | tag) != protocol_info.end(); }  
	uint16 protocol_datatype(const unsigned tag, uint16 id, uint16 idx) { return (protocol_info[(id << 8)|tag])[idx]; }
	
};

enum symbol_type {
	/*---------------------------------------------------------------
	  Symbols for marine (group 0...0-8191...bits 15-13=000).
	  ---------------------------------------------------------------*/
	sym_anchor = 0, /* white anchor symbol */
	sym_bell = 1, /* white bell symbol */
	sym_diamond_grn = 2, /* green diamond symbol */
	sym_diamond_red = 3, /* red diamond symbol */
	sym_dive1 = 4, /* diver down flag 1 */
	sym_dive2 = 5, /* diver down flag 2 */
	sym_dollar = 6, /* white dollar symbol */ 
	sym_fish = 7, /* white fish symbol */ 
	sym_fuel = 8, /* white fuel symbol */
	sym_horn = 9, /* white horn symbol */
	sym_house = 10, /* white house symbol */
	sym_knife = 11, /* white knife & fork symbol */
	sym_light = 12, /* white light symbol */
	sym_mug = 13, /* white mug symbol */
	sym_skull = 14, /* white skull and crossbones symbol*/
	sym_square_grn = 15, /* green square symbol */
	sym_square_red = 16, /* red square symbol */
	sym_wbuoy = 17, /* white buoy waypoint symbol */
	sym_wpt_dot = 18, /* waypoint dot */
	sym_wreck = 19, /* white wreck symbol */
	sym_null = 20, /* null symbol (transparent) */
	sym_mob = 21, /* man overboard symbol */
	/*------------------------------------------------------
	  marine navaid symbols
	  ------------------------------------------------------*/
	sym_buoy_ambr = 22, /* amber map buoy symbol */
	sym_buoy_blck = 23, /* black map buoy symbol */
	sym_buoy_blue = 24, /* blue map buoy symbol */
	sym_buoy_grn = 25, /* green map buoy symbol */
	sym_buoy_grn_red = 26, /* green/red map buoy symbol */
	sym_buoy_grn_wht = 27, /* green/white map buoy symbol */
	sym_buoy_orng = 28, /* orange map buoy symbol */
	sym_buoy_red = 29, /* red map buoy symbol */
	sym_buoy_red_grn = 30, /* red/green map buoy symbol */
	sym_buoy_red_wht = 31, /* red/white map buoy symbol */
	sym_buoy_violet = 32, /* violet map buoy symbol */
	sym_buoy_wht = 33, /* white map buoy symbol */ 
	sym_buoy_wht_grn = 34, /* white/green map buoy symbol */
	sym_buoy_wht_red = 35, /* white/red map buoy symbol */ 
	sym_dot = 36, /* white dot symbol */
	sym_rbcn = 37, /* radio beacon symbol */
	/*------------------------------------------------------ 
	  leave space for more navaids (up to 128 total)
	  ------------------------------------------------------*/
	sym_boat_ramp = 150, /* boat ramp symbol */
	sym_camp = 151, /* campground symbol */
	sym_restrooms = 152, /* restrooms symbol */
	sym_showers = 153, /* shower symbol */
	sym_drinking_wtr = 154, /* drinking water symbol */
	sym_phone = 155, /* telephone symbol */
	sym_1st_aid = 156, /* first aid symbol */
	sym_info = 157, /* information symbol */
	sym_parking = 158, /* parking symbol */
	sym_park = 159, /* park symbol */
	sym_picnic = 160, /* picnic symbol */
	sym_scenic = 161, /* scenic area symbol */
	sym_skiing = 162, /* skiing symbol */
	sym_swimming = 163, /* swimming symbol */
	sym_dam = 164, /* dam symbol */
	sym_controlled = 165, /* controlled area symbol */
	sym_danger = 166, /* danger symbol */
	sym_restricted = 167, /* restricted area symbol */
	sym_null_2 = 168, /* null symbol */
	sym_ball = 169, /* ball symbol */
	sym_car = 170, /* car symbol */
	sym_deer = 171, /* deer symbol */
	sym_shpng_cart = 172, /* shopping cart symbol */
	sym_lodging = 173, /* lodging symbol */
	sym_mine = 174, /* mine symbol */
	sym_trail_head = 175, /* trail head symbol */
	sym_truck_stop = 176, /* truck stop symbol */
	sym_user_exit = 177, /* user exit symbol */ 
	sym_flag = 178, /* flag symbol */
	sym_circle_x = 179, /* circle with x in the center */
	/*---------------------------------------------------------------
	  Symbols for land (group 1...8192-16383...bits 15-13=001).
	  ---------------------------------------------------------------*/
	sym_is_hwy = 8192, /* interstate hwy symbol */
	sym_us_hwy = 8193, /* us hwy symbol */
	sym_st_hwy = 8194, /* state hwy symbol */
	sym_mi_mrkr = 8195, /* mile marker symbol */
	sym_trcbck = 8196, /* TracBack (feet) symbol */
	sym_golf = 8197, /* golf symbol */
	sym_sml_cty = 8198, /* small city symbol */
	sym_med_cty = 8199, /* medium city symbol */
	sym_lrg_cty = 8200, /* large city symbol */
	sym_freeway = 8201, /* intl freeway hwy symbol */
	sym_ntl_hwy = 8202, /* intl national hwy symbol */
	sym_cap_cty = 8203, /* capitol city symbol (star) */
	sym_amuse_pk = 8204, /* amusement park symbol */
	sym_bowling = 8205, /* bowling symbol */
	sym_car_rental = 8206, /* car rental symbol */
	sym_car_repair = 8207, /* car repair symbol */
	sym_fastfood = 8208, /* fast food symbol */
	sym_fitness = 8209, /* fitness symbol */
	sym_movie = 8210, /* movie symbol */
	sym_museum = 8211, /* museum symbol */
	sym_pharmacy = 8212, /* pharmacy symbol */
	sym_pizza = 8213, /* pizza symbol */
	sym_post_ofc = 8214, /* post office symbol */ 
	sym_rv_park = 8215, /* RV park symbol */
	sym_school = 8216, /* school symbol */
	sym_stadium = 8217, /* stadium symbol */
	sym_store = 8218, /* dept. store symbol */
	sym_zoo = 8219, /* zoo symbol */
	sym_gas_plus = 8220, /* convenience store symbol */
	sym_faces = 8221, /* live theater symbol */
	sym_ramp_int = 8222, /* ramp intersection symbol */
	sym_st_int = 8223, /* street intersection symbol */
	sym_weigh_sttn = 8226, /* inspection/weigh station symbol */
	sym_toll_booth = 8227, /* toll booth symbol */
	sym_elev_pt = 8228, /* elevation point symbol */
	sym_ex_no_srvc = 8229, /* exit without services symbol */
	sym_geo_place_mm = 8230, /* Geographic place name, man-made */
	sym_geo_place_wtr = 8231, /* Geographic place name, water */
	sym_geo_place_lnd = 8232, /* Geographic place name, land */
	sym_bridge = 8233, /* bridge symbol */
	sym_building = 8234, /* building symbol */
	sym_cemetery = 8235, /* cemetery symbol */
	sym_church = 8236, /* church symbol */
	sym_civil = 8237, /* civil location symbol */
	sym_crossing = 8238, /* crossing symbol */
	sym_hist_town = 8239, /* historical town symbol */
	sym_levee = 8240, /* levee symbol */
	sym_military = 8241, /* military location symbol */
	sym_oil_field = 8242, /* oil field symbol */
	sym_tunnel = 8243, /* tunnel symbol */
	sym_beach = 8244, /* beach symbol */
	sym_forest = 8245, /* forest symbol */
	sym_summit = 8246, /* summit symbol */
	sym_lrg_ramp_int = 8247, /* large ramp intersection symbol */
	sym_lrg_ex_no_srvc = 8248, /* large exit without services smbl */
	sym_badge = 8249, /* police/official badge symbol */
	sym_cards = 8250, /* gambling/casino symbol */
	sym_snowski = 8251, /* snow skiing symbol */
	sym_iceskate = 8252, /* ice skating symbol */
	sym_wrecker = 8253, /* tow truck (wrecker) symbol */
	sym_border = 8254, /* border crossing (port of entry) */
	/*---------------------------------------------------------------
	  Symbols for aviation (group 2...16383-24575...bits 15-13=010).
	  ---------------------------------------------------------------*/
	sym_airport = 16384, /* airport symbol */
	sym_int = 16385, /* intersection symbol */
	sym_ndb = 16386, /* non-directional beacon symbol */
	sym_vor = 16387, /* VHF omni-range symbol */
	sym_heliport = 16388, /* heliport symbol */
	sym_private = 16389, /* private field symbol */
	sym_soft_fld = 16390, /* soft field symbol */
	sym_tall_tower = 16391, /* tall tower symbol */
	sym_short_tower = 16392, /* short tower symbol */
	sym_glider = 16393, /* glider symbol */
	sym_ultralight = 16394, /* ultralight symbol */
	sym_parachute = 16395, /* parachute symbol */
	sym_vortac = 16396, /* VOR/TACAN symbol */
	sym_vordme = 16397, /* VOR-DME symbol */
	sym_faf = 16398, /* first approach fix */
	sym_lom = 16399, /* localizer outer marker */
	sym_map = 16400, /* missed approach point */
	sym_tacan = 16401, /* TACAN symbol */
	sym_seaplane = 16402, /* Seaplane Base */
};


// each product has its own specific, more extensive waypoint type.  
// this type is the lowest common denominator of those types, except 
// for the D105 and D160 waypoint types.
struct basic_waypt_type {
	std::string id;
	semicircle_type pos;
	std::string comment;
	float altitude;
	symbol_type symbol;
};

struct track_point_type {
	semicircle_type pos;
	std::time_t time;
	bool new_track;
};

struct proximity_waypt_type {
	basic_waypt_type waypt;
	float32 dist; // meters
};


} // namespace

#endif // _gartype
