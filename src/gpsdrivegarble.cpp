// $Id$
// garble.cp
// Douglas S. J. De Couto
// 17 February 2000

// Copyright (C) 2000 Douglas S. J. De Couto
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

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// $Id$

// $Log$
// Revision 1.1  2004/12/23 16:03:24  commiter
// Initial revision
//
// Revision 1.11  2003/04/28 15:42:38  ganter
// compiles now with gcc 3.3
//
// Revision 1.10  2002/11/06 05:29:15  ganter
// fixed most warnings
//
// Revision 1.9  2001/11/16 18:00:54  ganter
// tcpserver/client works
//
// Revision 1.8  2001/10/09 07:18:55  ganter
// updated spanish translation, radar works, fix for g++ 3.x compilers
// updated wpcvt added expedia.com mapserver, but not working yet
//
// Revision 1.7  2001/09/25 23:49:43  ganter
// v 0.27
//
// Revision 1.6  2001/09/23 22:31:13  ganter
// v0.26
//
// Revision 1.5  2001/08/29 23:13:44  ganter
// version 0.9
//
// Revision 1.4  2001/08/27 02:57:19  ganter
// Version 0.7
//
// Revision 1.3  2001/08/27 01:03:24  ganter
// Version 0.7 added experimental -f flag for direct use of GARMIN format
// no gpsd must be started!
//
// Revision 1.2  2001/08/26 23:41:55  ganter
// changed for gpsdrive
//
// This version is adapted for use in gpsdrive by ganter@ganter.at

//static const char *garble_version = "Garble v1.0.1 29 May 2000";

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
static const int default_timeout = 200;	// milliseconds

static bool verbose = false;

extern "C" double glang, gbreit;
char serialdev[80];

enum garble_action_t
{
  get_routes,
  get_waypoints,
  get_proximity_waypoints,
  get_tracks,
  get_product_info,
  turn_off_gps,
  get_time,
  get_position,
  get_pvt,
  no_action
};

std::string
action_to_name (garble_action_t act)
{
  switch (act)
    {
    case get_routes:
      return "download routes";
      break;
    case get_waypoints:
      return "download waypoints";
      break;
    case get_proximity_waypoints:
      return "download proximity waypoints";
      break;
    case get_tracks:
      return "download tracks";
      break;
    case get_product_info:
      return "get GPS product info";
      break;
    case turn_off_gps:
      return "turn off GPS";
      break;
    case get_time:
      return "get time";
      break;
    case get_position:
      return "get position";
      break;
    case get_pvt:
      return "get pvt";
      break;
    case no_action:
      return "none";
      break;
    default:
      ;				// punt
    }
  assert (0);
}

#ifdef THISISUSED
static struct option long_options[] = {
  {"echo", 0, 0, 'e'},
  {"device", 1, 0, 'd'},
  {"get-routes", 0, 0, 'r'},
  {"get-waypoints", 0, 0, 'w'},
  {"get-proximity-waypoints", 0, 0, 'x'},
  {"get-tracks", 0, 0, 't'},
  {"get-gps-info", 0, 0, 'i'},
  {"turn-off", 0, 0, 'o'},
  {"get-time", 0, 0, 'z'},
  {"get-pos", 0, 0, 'p'},
  {"version", 0, 0, 'v'},
  {"help", 0, 0, 'h'},
  {"timeout", 1, 0, 'm'},
  {"verbose", 0, 0, 'l'},
  {0, 0, 0, 0}
};

static const char *short_options = "rwxtiozpvhd:m:el";
#endif

void
usage (void)
{
  cerr << "usage: " << endl
    <<
    "All data is sent to standard out.  Only the last GPS action option is used."
    << endl <<
    "-d, --device dev-name             use device specified by dev-name, defaults to "
    << default_dev << endl <<
    "-m, --timeout t                   wait up to t milliseconds to receive data, defaults to "
    << default_timeout << endl <<
    "-r, --get-routes                  download routes" << endl <<
    "-w, --get-waypoints               download waypoints" << endl <<
    "-x, --get-proximity-waypoints     download proximity waypoints" << endl
    << "-t, --get-tracks                  download tracks" << endl <<
    "-i, --get-gps-info                get GPS product info" << endl <<
    "-o, --turn-off                    turn off GPS" << endl <<
    "-z, --get-time                    get time from the GPS" << endl <<
    "-p, --get-pos                     get the current position" << endl <<
    "-v, --version                     current version of Garble" << endl <<
    "-e, --echo                        echo parameters and quit" << endl <<
    "-l, --verbose                     use verbose error messages" << endl <<
    "-h, --help                        print this message" << endl;
}


// not: the application layer object must be passed by reference, as
// we do not allow copying of the protocol layer object!
void
do_get_product_info (garmin::application_layer & al)
{
  garmin::product_data_type pd = al.get_product_data ();
  cout << "GPS product ID: " << pd.product_id << endl
    << "Software version: " << pd.software_version << endl
    << "Product description: " << pd.product_description << endl;
}

void
do_get_routes (garmin::application_layer & al)
{
  garmin::route_list_t * rl = al.get_routes ();
  garmin::route_list_t::const_iterator rli;
  for (rli = rl->begin (); rli != rl->end (); rli++)
    {
      garmin::route_t::const_iterator ri;
      for (ri = rli->begin (); ri != rli->end (); ri++)
	{
	  garmin::basic_waypt_type waypt = *ri;
	  garmin::degree_type pos = garmin::semicircle2degree (waypt.pos);
	  cout << waypt.id << " / " << pos.lat << ", " << pos.lon << " / "
	    << waypt.comment << endl;
	}
      cout << endl;
    }
  delete rl;
}

void
do_get_waypoints (garmin::application_layer & al)
{
  garmin::waypt_vec_t * wv = al.get_waypoints ();
  garmin::waypt_vec_t::const_iterator wvi;
  for (wvi = wv->begin (); wvi != wv->end (); wvi++)
    {
      garmin::basic_waypt_type waypt = *wvi;
      garmin::degree_type pos = garmin::semicircle2degree (waypt.pos);
      cout << waypt.id << " / " << pos.lat << ", " << pos.lon << " / "
	<< waypt.comment << endl;
    }
  delete wv;
}

void
do_get_proximity_waypoints (garmin::application_layer & al)
{
  garmin::prox_waypt_vec_t * wv = al.get_proximity_waypoints ();
  garmin::prox_waypt_vec_t::const_iterator wvi;
  for (wvi = wv->begin (); wvi != wv->end (); wvi++)
    {
      garmin::basic_waypt_type waypt = wvi->waypt;
      garmin::degree_type pos = garmin::semicircle2degree (waypt.pos);
      cout << waypt.id << " / " << wvi->dist << " / "
	<< pos.lat << ", " << pos.lon << " / " << waypt.comment << endl;
    }
  delete wv;
}

void
do_get_tracks (garmin::application_layer & al)
{
  garmin::track_list_t * tl = al.get_track_logs ();
  garmin::track_list_t::const_iterator tli;
  for (tli = tl->begin (); tli != tl->end (); tli++)
    {
      garmin::track_t::const_iterator ti;
      for (ti = tli->begin (); ti != tli->end (); ti++)
	{
	  garmin::track_point_type trackpt = *ti;
	  garmin::degree_type pos = garmin::semicircle2degree (trackpt.pos);
	  // ctime() includes "\n" at end of string, omit endl
	  cout << pos.lat << ", " << pos.lon << " / " << ctime (&trackpt.
								time);
	}
      cout << endl;
    }
  delete tl;
}

void
do_get_position (garmin::application_layer & al)
{
  garmin::degree_type pos = garmin::radian2degree (al.get_position ());
//  cout << pos.lat << ", " << pos.lon << endl;
  gbreit = pos.lat;
  glang = pos.lon;
}

void
do_get_pvt (garmin::application_layer & al)
{
  garmin::degree_type pos = garmin::radian2degree (al.get_pvt ());
//  cout << pos.lat << ", " << pos.lon << endl;
  gbreit = pos.lat;
  glang = pos.lon;
}

void
do_get_time (garmin::application_layer & al)
{
  // ctime includes "\n", omit endl
  time_t t = al.get_date_time ();
  cout << ctime (&t);
}

void
do_turn_off_gps (garmin::application_layer & al)
{
  al.turn_off_gps ();
}


//////////////////////////////////////////

extern "C" int
garblemain (int argc, char **argv)
{
    int meinargc;
  string *dev_name = new string (default_dev);
  garble_action_t action = no_action;
  int timeout = default_timeout;
  bool echo_params = false;

  meinargc=argc;
//    while (true) {
//      int opt_index;
//      int opt = getopt_long(argc, argv, short_options, long_options, &opt_index);
//      if (opt == -1)
//        break;

//      switch (opt) {
//      case 0: // is a long option
//        cout << long_options[opt_index].name << endl;
//        break;
//      case 'd':
//        delete dev_name;
//        dev_name = new string(optarg);
//        break;
//      case 'm':
//        timeout = atoi(optarg);
//        break;
//      case 'r':
//        action = get_routes;
//        break;
//      case 'e':
//        echo_params = true;
//        break;
//      case 'w':
//        action = get_waypoints;
//        break;
//      case 'x':
//        action = get_proximity_waypoints;
//        break;
//      case 't':
//        action = get_tracks;
//        break;
//      case 'i':
//        action = get_product_info;
//        break;
//      case 'o':
//        action = turn_off_gps;
//        break;
//      case 'z':
//        action = get_time;
//        break;
//      case 'p':
//        action = get_position;
//        break;
//      case 'v':
//        cerr << garble_version << endl;
//        exit(0);
//        break;
//      case 'l':
//        verbose = true;
//        break;
//      case 'h':
//        // fall through to default
//      default:
//        usage();
//        exit(-1);
//      }
//    }

  if (meinargc == 1)
    action = get_position;
  if (meinargc == 2)
    action = get_pvt;

  *dev_name=serialdev;

  if (echo_params)
    {
      cout << "Using the following parameters:" << endl
	<< "Device name: " << *dev_name << endl
	<< "Timeout (milliseconds): " << timeout << endl
	<< "Action: " << action_to_name (action) << endl
	<< "Verbose: " << (verbose ? "yes" : "no") << endl;
      exit (0);
    }

  // setup the serial port etc.
  garmin_serial gs;
  try
  {
    gs.init (*dev_name, timeout);
  }
  catch (garmin::not_possible & ex)
  {
//    cerr << "Unable to open the serial port" << endl;
    if (verbose)
      cerr << ex.m_msg << endl;
    return (-2);
  }

  garmin::link_layer ll (&gs);
  garmin::application_layer al (&ll);

  try
  {
    switch (action)
      {
      case get_product_info:
	do_get_product_info (al);
	break;
      case get_routes:
	do_get_routes (al);
	break;
      case get_waypoints:
	do_get_waypoints (al);
	break;
      case get_proximity_waypoints:
	do_get_proximity_waypoints (al);
	break;
      case get_tracks:
	do_get_tracks (al);
	break;
      case get_position:
	do_get_position (al);
	break;
      case get_pvt:
	do_get_pvt (al);
	break;
      case get_time:
	do_get_time (al);
	break;
      case turn_off_gps:
	do_turn_off_gps (al);
	break;
      case no_action:
	cerr << "No action specified." << endl;
	usage ();
	exit (-1);
	break;
      default:
	cerr << "logic error" << endl;
	assert (0);
      }
  }
  catch (garmin::timeout & ex)
  {
//    cerr << "Timeout before getting data from the GPS" << endl;
    if (verbose)
      cerr << ex.m_msg << endl;
// set havepos to FALSE in gpsdrive.c
    return (-2);
  }
  catch (garmin::not_possible & ex)
  {
    cerr << "Error getting data from the GPS" << endl;
    if (verbose)
      cerr << ex.m_msg << endl;
    return (-1);
  }

  return 0;
}
