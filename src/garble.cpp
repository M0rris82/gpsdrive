// $Id$
// garble.cp
// Douglas S. J. De Couto
// 17 February 2000

// Copyright (C) 2000 Douglas S. J. De Couto
// <decouto@lcs.mit.edu>
//
// Copyright (C) 2002 Edouard Lafargue
// <elafargue@hotmail.com>
// 
// Revision History:
// 2002-08-04: Added support for waypoint type D109 (newest Garmin GPSes)
//             Extended waypoint listing with symbol codes and altitude
//
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

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// $Id$

static const char *garble_version = "Garble v1.0.2 04 Aug 2002";

#include <getopt.h>

#include <iostream>
#include <string>
#include <cstdlib>
//  #include <ctime>
 
using namespace std;
 

#include "garmin_serial_unix.h"
#include "garmin_link.h"
#include "garmin_application.h"
#include "garmin_util.h"
#include <assert.h>

static const char *default_dev = "/dev/gps";
static const int default_timeout = 1500; // milliseconds
int garble_precision = 8; // sig figs
int xplanet = 0; // xplanet format for waypoint export

static bool verbose = false;

enum garble_action_t {
  get_routes,
  get_waypoints,
  get_proximity_waypoints,
  get_tracks,
  get_product_info,
  turn_off_gps,
  get_time,
  get_position,
  get_screenshot,
  no_action
};

char *action_to_name(garble_action_t act) {
  switch (act) {
  case get_routes: return "download routes"; break;
  case get_waypoints: return "download waypoints"; break;
  case get_proximity_waypoints: return "download proximity waypoints"; break;
  case get_tracks: return "download tracks"; break;
  case get_product_info: return "get GPS product info"; break;
  case turn_off_gps: return "turn off GPS"; break;
  case get_time: return "get time"; break;
  case get_position: return "get position"; break;
  case no_action: return "none"; break;
  default: 
    ; // punt
  }
  assert(0);
}


static struct option long_options[] = {
  { "echo", 0, 0, 'e' },
  { "device", 1, 0, 'd' },
  { "get-routes", 0, 0, 'r' }, 
  { "get-waypoints", 0, 0, 'w' },
  { "get-proximity-waypoints", 0, 0, 'x' },
  { "get-tracks", 0, 0, 't' }, 
  { "get-gps-info", 0, 0, 'i' }, 
  { "turn-off", 0, 0, 'o' },
  { "get-time", 0, 0, 'z' },
  { "get-pos", 0, 0, 'p' }, 
  { "version", 0, 0, 'v' }, 
  { "help", 0, 0, 'h' },
  { "timeout", 1, 0, 'm' },
  { "precision", 1, 0, 's' }, 
  { "verbose", 0, 0, 'l' },
  { "waypoint-symbols", 0, 0, 'W' },
  { "get-screenshot", 1, 0, 'S' },
  { "xplanet-format", 0, 0, 'X' },
  { 0, 0, 0, 0 }
};

static const char *short_options = "rwxtiozpvhd:m:elWSX";

void
usage(void) {
  cerr << "usage: " << endl 
       << "All data is sent to standard out.  Only the last GPS action option is used." << endl
       << "-d, --device dev-name             use device specified by dev-name, defaults to " << default_dev << endl
       << "-m, --timeout t                   wait up to t milliseconds to receive data, 0 means wait forever, defaults to " << default_timeout << endl
       << "-s, --precision p                 ouput precision (number of digits), defaults to " << garble_precision << endl
       << "-r, --get-routes                  download routes" << endl
       << "-w, --get-waypoints               download waypoints" << endl
       << "-x, --get-proximity-waypoints     download proximity waypoints" << endl
       << "-t, --get-tracks                  download tracks" << endl
       << "-i, --get-gps-info                get GPS product info" << endl
       << "-o, --turn-off                    turn off GPS" << endl
       << "-z, --get-time                    get time from the GPS" << endl
       << "-p, --get-pos                     get the current position" << endl
       << "-v, --version                     current version of Garble" << endl
       << "-e, --echo                        echo parameters and quit" << endl
       << "-l, --verbose                     use verbose error messages" << endl
       << "-h, --help                        print this message" << endl
       << "-W, --waypoint-symbols            prints a list of all waypoint symbol codes" << endl
       << "-S, --get-screenshot filename     saves a screendump of the GPS into file 'filename'" << endl
       << "-X, --xplanet-format              saves waypoints in a format compatible with xplanet's markerfile" << endl;
}

void
list_symbols(void) {
  cout << 
      "#---------------------------------------------------------------"  << endl <<
      "# Symbols for marine (group 0...0-8191...bits 15-13=000)." << endl <<
      "#---------------------------------------------------------------" << endl <<
      "   0, white anchor" << endl <<
      "   1, white bell" << endl <<
      "   2, green diamond" << endl <<
      "   3, red diamond" << endl <<
"   4, diver down flag 1" << endl <<
"   5, diver down flag 2" << endl <<
"   6, white dollar " << endl <<
"   7, white fish " << endl <<
"   8, white fuel" << endl <<
"   9, white horn" << endl <<
"  10, white house" << endl <<
"  11, white knife & fork" << endl <<
"  12, white light" << endl <<
"  13, white mug" << endl <<
"  14, white skull and crossbones" << endl <<
"  15, green square" << endl <<
"  16, red square" << endl <<
"  17, white buoy waypoint" << endl <<
"  18, waypoint dot" << endl <<
"  19, white wreck" << endl <<
"  20, null symbol (transparent)" << endl <<
"  21, man overboard" << endl <<
"#------------------------------------------------------" << endl <<
"# Marine navaid symbols" << endl <<
"#------------------------------------------------------" << endl <<
"  22, amber map buoy" << endl <<
"  23, black map buoy " << endl <<
"  24, blue map buoy " << endl <<
"  25, green map buoy " << endl <<
"  26, green/red map buoy " << endl <<
"  27, green/white map buoy " << endl <<
"  28, orange map buoy " << endl <<
"  29, red map buoy " << endl <<
"  30, red/green map buoy " << endl <<
"  31, red/white map buoy " << endl <<
"  32, violet map buoy " << endl <<
"  33, white map buoy  " << endl <<
"  34, white/green map buoy " << endl <<
"  35, white/red map buoy  " << endl <<
"  36, white dot " << endl <<
"  37, radio beacon " << endl <<
"#------------------------------------------------------ " << endl <<
"# leave space for more navaids (up to 128 total)" << endl <<
"#------------------------------------------------------" << endl <<
" 150, boat ramp " << endl <<
" 151, campground " << endl <<
" 152, restrooms " << endl <<
" 153, shower " << endl <<
" 154, drinking water " << endl <<
" 155, telephone " << endl <<
" 156, first aid " << endl <<
" 157, information " << endl <<
" 158, parking " << endl <<
" 159, park " << endl <<
" 160, picnic " << endl <<
" 161, scenic area " << endl <<
" 162, skiing " << endl <<
" 163, swimming " << endl <<
" 164, dam " << endl <<
" 165, controlled area " << endl <<
" 166, danger " << endl <<
" 167, restricted area " << endl <<
" 168, null " << endl <<
" 169, ball " << endl <<
" 170, car " << endl <<
" 171, deer " << endl <<
" 172, shopping cart " << endl <<
" 173, lodging " << endl <<
" 174, mine " << endl <<
" 175, trail head " << endl <<
" 176, truck stop " << endl <<
" 177, user exit  " << endl <<
" 178, flag " << endl <<
" 179, circle with x in the center" << endl <<
"#---------------------------------------------------------------" << endl <<
"# Symbols for land (group 1...8192-16383...bits 15-13=001)." << endl <<
"# ---------------------------------------------------------------" << endl <<
"8192, interstate hwy " << endl <<
"8193, us hwy " << endl <<
"8194, state hwy " << endl <<
"8195, mile marker " << endl <<
"8196, TracBack (feet) " << endl <<
"8197, golf " << endl <<
"8198, small city " << endl <<
"8199, medium city " << endl <<
"8200, large city " << endl <<
"8201, intl freeway hwy " << endl <<
"8202, intl national hwy " << endl <<
"8203, capitol city symbol (star)" << endl <<
"8204, amusement park " << endl <<
"8205, bowling " << endl <<
"8206, car rental " << endl <<
"8207, car repair " << endl <<
"8208, fast food " << endl <<
"8209, fitness " << endl <<
"8210, movie " << endl <<
"8211, museum " << endl <<
"8212, pharmacy " << endl <<
"8213, pizza " << endl <<
"8214, post office  " << endl <<
"8215, RV park " << endl <<
"8216, school " << endl <<
"8217, stadium " << endl <<
"8218, dept. store " << endl <<
"8219, zoo " << endl <<
"8220, convenience store " << endl <<
"8221, live theater " << endl <<
"8222, ramp intersection " << endl <<
"8223, street intersection " << endl <<
"8226, inspection/weigh station " << endl <<
"8227, toll booth " << endl <<
"8228, elevation point " << endl <<
"8229, exit without services " << endl <<
"8230, Geographic place name, man-made" << endl <<
"8231, Geographic place name, water" << endl <<
"8232, Geographic place name, land" << endl <<
"8233, bridge " << endl <<
"8234, building " << endl <<
"8235, cemetery " << endl <<
"8236, church " << endl <<
"8237, civil location " << endl <<
"8238, crossing " << endl <<
"8239, historical town " << endl <<
"8240, levee " << endl <<
"8241, military location " << endl <<
"8242, oil field" << endl <<
"8243, tunnel" << endl <<
"8244, beach" << endl <<
"8245, forest" << endl <<
"8246, summit" << endl <<
"8247, large ramp intersection" << endl <<
"8248, large exit without services smbl" << endl <<
"8249, police/official badge" << endl <<
"8250, gambling/casino" << endl <<
"8251, snow skiing" << endl <<
"8252, ice skating" << endl <<
"8253, tow truck (wrecker)" << endl <<
"8254, border crossing (port of entry)" << endl <<
"#---------------------------------------------------------------" << endl <<
"# Symbols for aviation (group 2...16383-24575...bits 15-13=010)." << endl <<
"#---------------------------------------------------------------" << endl <<
"16384, airport " << endl <<
"16385, intersection " << endl <<
"16386, non-directional beacon " << endl <<
"16387, VHF omni-range " << endl <<
"16388, heliport " << endl <<
"16389, private field " << endl <<
"16390, soft field " << endl <<
"16391, tall tower " << endl <<
"16392, short tower " << endl <<
"16393, glider " << endl <<
"16394, ultralight " << endl <<
"16395, parachute " << endl <<
"16396, VOR/TACAN " << endl <<
"16397, VOR-DME " << endl <<
"16398, first approach fix" << endl <<
"16399, localizer outer marker" << endl <<
"16400, missed approach point" << endl <<
"16401, TACAN " << endl <<
"16402, Seaplane Base" << endl;

}  

// Note: the application layer object must be passed by reference, as
// we do not allow copying of the protocol layer object!
void
do_get_product_info(garmin::application_layer &al) {
  garmin::product_data_type pd = al.get_product_data();
  cout << "GPS product ID: " << pd.product_id << endl
       << "Software version: " << pd.software_version << endl
       << "Product description: " << pd.product_description << endl;
}

void
do_get_routes(garmin::application_layer &al) {
  garmin::route_list_t *rl = al.get_routes();
  garmin::route_list_t::const_iterator rli;
  for (rli = rl->begin(); rli != rl->end(); rli++) {
    garmin::route_t::const_iterator ri;
    for (ri = rli->begin(); ri != rli->end(); ri++) {
      garmin::basic_waypt_type waypt = *ri;
      garmin::degree_type pos = garmin::semicircle2degree(waypt.pos);
      cout << waypt.id << " / ";
      streamsize p = cout.precision();
      cout.precision(garble_precision);
      cout << pos.lat << ", " << pos.lon;
      cout.precision(p);
      cout << " / " << waypt.comment << endl;
    }
    cout << endl;
  }
  delete rl;
}

void
do_get_waypoints(garmin::application_layer &al) {
  garmin::waypt_vec_t *wv = al.get_waypoints();
  garmin::waypt_vec_t::const_iterator wvi;
    cout << "# Name, Symbol, Description, Latitude, Longitude, Altitude, Depth" << endl;
    for (wvi = wv->begin(); wvi != wv->end(); wvi++) {
      garmin::basic_waypt_type waypt = *wvi;
      garmin::degree_type pos = garmin::semicircle2degree(waypt.pos);
      if (xplanet == 1) {
	streamsize p = cout.precision();
	cout.precision(garble_precision);
	cout << pos.lat << "    " << pos.lon;
	cout << "   \"" << waypt.id << "\"";
	cout << endl;
	cout.precision(p);
      } else {
	cout << waypt.id << ", ";
	cout << waypt.symbol;
	cout << ", " << waypt.comment << ", ";
	streamsize p = cout.precision();
	cout.precision(garble_precision);
	cout << pos.lat << ", " << pos.lon;
	cout << ", " << waypt.altitude;
	cout << endl;
	cout.precision(p);
      }
    }
    
  
  delete wv;
}

void
do_get_proximity_waypoints(garmin::application_layer &al) {
  garmin::prox_waypt_vec_t *wv = al.get_proximity_waypoints();
  garmin::prox_waypt_vec_t::const_iterator wvi;
  for (wvi = wv->begin(); wvi != wv->end(); wvi++) {
    garmin::basic_waypt_type waypt = wvi->waypt;
    garmin::degree_type pos = garmin::semicircle2degree(waypt.pos);
    cout << waypt.id << " / ";
      streamsize p = cout.precision();
      cout.precision(garble_precision);
      cout << wvi->dist << " / " << pos.lat << ", " << pos.lon;
      cout.precision(p);
      cout << " / " << waypt.comment << endl;
  }
  delete wv;
}

void
do_get_tracks(garmin::application_layer &al) {
  garmin::track_list_t *tl = al.get_track_logs();
  garmin::track_list_t::const_iterator tli;
  for (tli = tl->begin(); tli != tl->end(); tli++) {
    garmin::track_t::const_iterator ti;
    for (ti = tli->begin(); ti != tli->end(); ti++) {
      garmin::track_point_type trackpt = *ti;
      garmin::degree_type pos = garmin::semicircle2degree(trackpt.pos);
      // ctime() includes "\n" at end of string, omit endl
      streamsize p = cout.precision();
      cout.precision(garble_precision);
      cout << pos.lat << ", " << pos.lon;
      cout.precision(p);
      cout << " / " << ctime(&trackpt.time);
    }
    cout << endl;
  }
  delete tl;
}

void
do_get_position(garmin::application_layer &al) {
  garmin::degree_type pos = garmin::radian2degree(al.get_position());
  streamsize p = cout.precision();
  cout.precision(garble_precision);
  cout << pos.lat << ", " << pos.lon << endl;
  cout.precision(p);
}

void
do_get_time(garmin::application_layer &al) {
  // ctime includes "\n", omit endl
  time_t t = al.get_date_time();
  cout << ctime(&t);
}

void
do_turn_off_gps(garmin::application_layer &al) {
  al.turn_off_gps();
}

void
do_get_screenshot(garmin::application_layer &al) {
  al.get_display_bitmap();
}


//////////////////////////////////////////

int
main(int argc, char **argv) {
  
  string *dev_name = new string(default_dev);
  garble_action_t action = no_action;
  int timeout = default_timeout;
  bool echo_params = false;

  while (true) {
    int opt_index;
    int opt = getopt_long(argc, argv, short_options, long_options, &opt_index);
    if (opt == -1)
      break;
    
    switch (opt) {
    case 0: // is a long option
      cout << long_options[opt_index].name << endl;
      break;
    case 'd':
      delete dev_name;
      dev_name = new string(optarg);
      break;
    case 's':
      garble_precision = atoi(optarg);
      if (garble_precision < 1) {
	cerr << "Precision must be greater than 0" << endl;
	exit(-1);
      }
      break;
    case 'm':
      timeout = atoi(optarg);
      if (timeout < 0) {
	cerr << "Timeout must be 0 or greater" << endl;
	exit(-1);
      }
      break;
    case 'X':
      xplanet=1;
      break;
    case 'r':
      action = get_routes;
      break;
    case 'S':
      action = get_screenshot;
      break;
    case 'e':
      echo_params = true;
      break;
    case 'w':
      action = get_waypoints;
      break;
    case 'W':
      list_symbols();
      exit(0);
    case 'x':
      action = get_proximity_waypoints;
      break;
    case 't':
      action = get_tracks;
      break;
    case 'i':
      action = get_product_info;
      break;
    case 'o':
      action = turn_off_gps;
      break;
    case 'z':
      action = get_time;
      break;
    case 'p':
      action = get_position;
      break;
    case 'v':
      cerr << garble_version << endl;
      exit(0);
      break;
    case 'l':
      verbose = true;
      break;
    case 'h':
      // fall through to default
    default:
      usage();
      exit(-1);
    }
  }

  if (echo_params) {
    cout << "Using the following parameters:" << endl
	 << "Device name: " << *dev_name << endl
	 << "Timeout (milliseconds): " << timeout << endl
      	 << "Precision (digits): " << garble_precision << endl
	 << "Action: " << action_to_name(action) << endl
	 << "Verbose: " << (verbose ? "yes" : "no") << endl;
    exit(0);
  }

  // setup the serial port etc.
  garmin_serial gs;
  try {
    gs.init(*dev_name, timeout);
  }
  catch (garmin::not_possible &ex) {
    cerr << "Unable to open the serial port" << endl;
    if (verbose) 
      cerr << ex.m_msg << endl;
    exit(-1);
  }

  garmin::link_layer ll(&gs);
  garmin::application_layer al(&ll);
  
  try {
    switch (action) {
    case get_product_info: do_get_product_info(al); break;
    case get_routes: do_get_routes(al); break;
    case get_waypoints: do_get_waypoints(al); break;
    case get_proximity_waypoints: do_get_proximity_waypoints(al); break;
    case get_tracks: do_get_tracks(al); break;
    case get_position: do_get_position(al); break;
    case get_time: do_get_time(al); break;
    case turn_off_gps: do_turn_off_gps(al); break;
    case get_screenshot: do_get_screenshot(al); break;
    case no_action:
      cerr << "No action specified." << endl;
      usage();
      exit(-1);
      break;
    default:
      cerr << "logic error" << endl;
      assert(0);
    }  
  }
  catch (garmin::timeout &ex) {
    cerr << "Timeout before getting data from the GPS" << endl;
    if (verbose) 
      cerr << ex.m_msg << endl;
    exit(-1);
  }
  catch (garmin::not_possible &ex) {
    cerr << "Error getting data from the GPS" << endl;
    if (verbose) 
      cerr << ex.m_msg << endl;
    exit(-1);
  }
  catch (garmin::unsupported_protocol &ex) {
    cerr << "Operation was not possible in current configuration" << endl;
    if (verbose) 
      cerr << ex.m_msg << endl;
    exit(-1);
  }

  exit(0);
}
