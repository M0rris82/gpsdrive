//
// Copyright (c) 2005 -- Daniel Wallner
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "../lib_map/map.h"

#include <stdio.h>
#ifdef USE_CAIRO
#include <cairo.h>
#endif
#include <Carbon/Carbon.h>

#include <unistd.h>
#include <stdlib.h>

void init_gps();
void free_gps();

#define PIXEL_SIZE 0.22E-3
#define SCALE_RATIO 10

typedef struct
{
  HIViewRef view;
  MapState *mapstate;
}
ViewData;

MapSettings mapsettings;
MapSet mapset;
MapState mapstates[16];
ViewData canvases[16];
int views = 1;
int render_mode = 0x100;
int cover = 0;
int draw_transparent = 0;
double pixel_size = PIXEL_SIZE;
int noZoom = FALSE;

static void expose(CGContextRef gc, int width, int height, MapState *mapstate)
{
#ifdef USE_CAIRO
#else
  MapGC mgc;

  memset((void*)&mgc, 0, sizeof(mgc));

  mgc.quartz_gc = gc;

  if (cover)
    coverifpossible(mapstate, width, height);

  drawmap(mapstate, &mgc, width, height, draw_transparent);

  drawmarkers(&mgc, width, height,
	      &mapsettings, mapstate, pixel_size,
	      0, 0);
#endif
}

static void button_press(int x, int y, MapState *mapstate)
{
  int i;

  mapsettings.posmode = TRUE;

  screen2wgs(mapstate, x, y,
	     &mapstate->req_lat, &mapstate->req_lon);

  fprintf(stderr, "new center: %lf;%lf\n",
	  mapstate->req_lat, mapstate->req_lon);

  for (i = 0; i < views; i++)
    {
      mapstates[i].req_lat = mapstate->req_lat;
      mapstates[i].req_lon = mapstate->req_lon;

      if (views != mapset.maps)
	{
	  selectbestmap(&mapset, &mapstates[i], render_mode, pixel_size);
	}
      else
	{
	  double pseudo_lat = (render_mode & 0x100) ? mapstate->req_lat : 0;

	  wgs2pixel(mapstates[i].dataset[0], mapstates[i].path[0],
		    mapstate->req_lat, mapstate->req_lon,
		    &mapstates[i].act_xPixel[0], &mapstates[i].act_yPixel[0]);

	  scale2zoom(mapstates[i].dataset[0], mapstates[i].path[0],
		     mapstates[i].req_scale, pixel_size, pseudo_lat,
		     &mapstates[i].act_xZoom[0], &mapstates[i].act_yZoom[0]);
	}

      if (noZoom)
	{
	  mapstates[i].act_xZoom[0] = 1;
	  mapstates[i].act_yZoom[0] = 1;
	}

      HIViewSetNeedsDisplay(canvases[i].view, TRUE);
    }
}

static void key_press(char key, MapState *mapstate)
{
  int i;

  int redraw = 0;

  double factor = 1;

  switch (key)
    {
    case '+':
      factor = 1 / 1.5;
      redraw = 1;
      break;

    case '-':
      factor = 1.5;
      redraw = 1;
      break;

    case 'm':
      render_mode = (render_mode & 0xfff0) + (((render_mode & 0x0f) + 1) % 3);
      redraw = 1;
      break;

    case 'f':
      render_mode = (render_mode & 0xff0f) +
	(((((render_mode & 0xf0) >> 4) + 1) % 3) << 4);
      redraw = 1;
      break;

    case 'a':
      render_mode = render_mode | 0x200;
      redraw = 1;
      break;

    case 'c':
      cover = !cover;
      redraw = 1;
      break;

    case 'p':
      mapsettings.havepos = !mapsettings.havepos;
      redraw = 1;
      break;

    case 's':
      render_mode ^= 0x100;
      redraw = 1;
      break;

    case 'g':
      mapsettings.drawgrid = !mapsettings.drawgrid;
      redraw = 1;
      break;

    case 't':
      draw_transparent = !draw_transparent;
      redraw = 1;
      break;

    case 'r':
      mapstate->req_rotation += 45;
      if (mapstate->req_rotation >= 360)
	mapstate->req_rotation -= 360;
      redraw = 1;
      break;
    }

  fprintf(stderr, "mode: %X\n", render_mode);

  if (redraw)
    for (i = 0; i < views; i++)
      {
	mapstates[i].req_scale *= factor;

	if (views != mapset.maps)
	  {
	    selectbestmap(&mapset, &mapstates[i], render_mode, pixel_size);
	  }
	else
	  {
	    double pseudo_lat = (render_mode & 0x100) ? mapstate->req_lat : 0;

	    wgs2pixel(mapstates[i].dataset[0], mapstates[i].path[0],
		      mapstate->req_lat, mapstate->req_lon,
		      &mapstates[i].act_xPixel[0],
		      &mapstates[i].act_yPixel[0]);

	    scale2zoom(mapstates[i].dataset[0], mapstates[i].path[0],
		       mapstates[i].req_scale, pixel_size, pseudo_lat,
		       &mapstates[i].act_xZoom[0], &mapstates[i].act_yZoom[0]);
	  }

	if (noZoom)
	  {
	    mapstates[i].act_xZoom[0] = 1;
	    mapstates[i].act_yZoom[0] = 1;
	  }

	HIViewSetNeedsDisplay(canvases[i].view, TRUE);
      }

  render_mode &= ~0x200;
}

static pascal OSStatus OnViewEvent(EventHandlerCallRef handler,
				   EventRef event, ViewData *data)
{
  OSStatus err = CallNextEventHandler(handler, event);
  UInt32 kind = GetEventKind(event);

  if (err)
    return err;

  switch (kind)
    {
    case kEventControlBoundsChanged:
      // Force redraw
      HIViewSetNeedsDisplay(data->view, TRUE);
      return noErr;

    case kEventControlClick:
      {
	HIPoint where;
	err = GetEventParameter(event, kEventParamMouseLocation, typeHIPoint,
			NULL, sizeof( HIPoint ), NULL, &where);
	Point local;
	local.h = where.x;
	local.v = where.y;
	QDGlobalToLocalPoint(GetWindowPort(GetControlOwner(data->view)),
			     &local);
	button_press(local.h, local.v, data->mapstate);
      }
      return noErr;

    case kEventControlDraw:
      {
	// Get CGContextRef
	CGContextRef gc;
	err = GetEventParameter(event,
				kEventParamCGContextRef,
				typeCGContextRef,
				NULL,
				sizeof(CGContextRef),
				NULL,
				&gc);
	if (err)
	  return err;

	CGRect r;

	HIViewGetBounds(data->view, &r);

	expose(gc, r.size.width, r.size.height, data->mapstate);
      }
      return noErr;
    }

  return err;
}

static pascal OSStatus OnWindowEvent(EventHandlerCallRef handler,
				     EventRef event, MapState *mapstate)
{
  UInt32 kind = GetEventKind(event);

  switch (kind)
    {
    case kEventRawKeyDown:
      {
	char charCode;
	GetEventParameter(event, kEventParamKeyMacCharCodes, typeChar,
			  NULL, sizeof(char), NULL, &charCode);
	key_press(charCode, mapstate);
      }
      break;

    case kEventWindowClose:
      QuitApplicationEventLoop();
      break;

    default:
      return eventNotHandledErr;
    }

  return noErr;
}

WindowRef CreateWindow(int width, int height, MapState *mapstate)
{
  WindowRef window;

  Rect rect;
  SetRect(&rect, 0, 0, width, height);

  OSStatus err = CreateNewWindow(kDocumentWindowClass,
				 kWindowStandardDocumentAttributes |
				 kWindowStandardHandlerAttribute |
				 kWindowCompositingAttribute |
				 kWindowInWindowMenuAttribute,
				 &rect,
				 &window);

  // Setup window
  Str255 title = "\pmmapview";
  SetWTitle(window, title);
  RepositionWindow(window, NULL, kWindowCascadeOnMainScreen);

  // Change size box to transparent
  HIViewRef growThumb = NULL;
  HIViewFindByID(HIViewGetRoot(window), kHIViewWindowGrowBoxID, &growThumb);
  if (growThumb != NULL)
    {
      err = HIGrowBoxViewSetTransparent(growThumb, TRUE);
	}

  // Setup window events
  InstallStandardEventHandler(GetWindowEventTarget(window));

  EventTypeSpec eventList[] = {
			{kEventClassKeyboard, kEventRawKeyDown},
			{kEventClassWindow, kEventWindowClickContentRgn},
			{kEventClassWindow, kEventWindowClose}};

  InstallWindowEventHandler(window, NewEventHandlerUPP
			    ((EventHandlerProcPtr)OnWindowEvent),
			    GetEventTypeCount(eventList),
			    eventList, (void*)mapstate, NULL);

  return window;
}

HIViewRef CreateMapView(HIViewRef parent, int left, int top,
			int right, int bottom, ViewData *data)
{
  HIViewRef view;

  HIRect bounds = CGRectMake(left, top, right, bottom);

  OSStatus err = HIObjectCreate(kHIViewClassID, nil, (HIObjectRef *)&view);
  data->view = view;

  err = HIViewChangeAttributes(view, kHIViewAllowsSubviews, 0);
  err = HIViewAddSubview(parent, view);
  err = HIViewSetFrame(view, &bounds);
  err = HIViewSetVisible(view, true);

  // Setup view events
  static EventTypeSpec viewEvents[] = {
			{kEventClassControl, kEventControlClick},
			{kEventClassControl, kEventControlBoundsChanged},
			{kEventClassControl, kEventControlDraw}};

  InstallControlEventHandler(view,
		      NewEventHandlerUPP((EventHandlerProcPtr)OnViewEvent),
		      GetEventTypeCount(viewEvents),
		      viewEvents,
		      (void*)data,
		      NULL);

  return view;
}

void usage()
{
  fprintf(stderr, "libmap test and sample application\n\n");
  fprintf(stderr, " usage: gmapview [input map files or directories] [options and arguments]\n\n");
  fprintf(stderr, "   -h                  print this message\n");
  fprintf(stderr, "   -a latitude         initial latitude\n");
  fprintf(stderr, "   -o longitude        initial longitude\n");
  fprintf(stderr, "   -s scale            initial scale\n");
  fprintf(stderr, "   -r rotation         initial rotation\n");
  fprintf(stderr, "   -p pixel size (mm)  pixel size\n");
  fprintf(stderr, "   -d dpi (1/inch)     pixel size\n");
  fprintf(stderr, "   -z                  no zoom\n");
  fprintf(stderr, "   -v n                number of views\n");
  fprintf(stderr, "   -w                  windowed views\n");
  fprintf(stderr, "   -l                  linear view scale\n");
  fprintf(stderr, "   -m ratio            view scale ratio\n\n");
  fprintf(stderr, "  If no input is given, gmapview uses the current directory.\n");
  fprintf(stderr, "  When the number of views equals the number of maps,\n");
  fprintf(stderr, "  each map is fixed to each view.\n");
  fprintf(stderr, "  You can combine short flags, so `-w -z' means the same as -wz or -zw.\n");
  fprintf(stderr, "\n  When gmapview is running it can be controlled with the following keys:\n");
  fprintf(stderr, "\n   +  Zoom in\n");
  fprintf(stderr, "   -  Zoom out\n");
  fprintf(stderr, "   m  Drawing mode (Single, overview, tiled)\n");
  fprintf(stderr, "   f  Filter maps (Street, topo, all)\n");
  fprintf(stderr, "   a  Show alternate map\n");
  fprintf(stderr, "   c  Cover if possible\n");
  fprintf(stderr, "   p  Show current position\n");
  fprintf(stderr, "   s  Pseudo projection\n");
  fprintf(stderr, "   g  Grid\n");
  fprintf(stderr, "   t  Transparent\n");
  fprintf(stderr, "   r  Rotate\n");
  fprintf(stderr, "\n  Left mouse click selects a new center point.\n");
}

int main(int argc, char **argv)
{
  extern char *optarg;
  extern int optind;
  extern int optopt;

  int i;

  WindowRef windows[16];
  //GtkWidget *paned;
  //GtkWidget *box1;
  //GtkWidget *box2;

  int windowed = FALSE;
  int latSet = FALSE;
  int lonSet = FALSE;
  int scaleSet = FALSE;
  int linear = FALSE;
  double ratio = SCALE_RATIO;

  mapinit(&mapsettings);

  resetmap(&mapstates[0]);

  while (optind < argc)
    {
      if (argv[optind][0] != '-' || strlen(argv[optind]) <= 1)
	{
	  // Load maps
	  if (!addmaps(argv[optind], &mapset))
	  {
	    fprintf(stderr,
		    "Unable to open input file: %s\n",
		    argv[optind]);
	    return 1;
	  }

	  optind++;
	}
      else
	{
	  // Option
	  int opt = getopt(argc, argv, ":ha:o:s:r:p:d:zv:wlm:");
	  if (opt != -1)
	    {
	      switch (opt)
		{
		case 'h':
		  usage();
		  return 1;

		case 'a':
		  mapstates[0].req_lat = atof(optarg);
		  latSet = TRUE;
		  break;

		case 'o':
		  mapstates[0].req_lon = atof(optarg);
		  lonSet = TRUE;
		  break;

		case 's':
		  mapstates[0].req_scale = atof(optarg);
		  scaleSet = TRUE;
		  break;

		case 'r':
		  mapstates[0].req_rotation = atof(optarg) * M_PI / 180;
		  break;

		case 'p':
		  pixel_size = atof(optarg) / 1000;
		  break;

		case 'd':
		  pixel_size = 0.0254 / atof(optarg);
		  break;

		case 'z':
		  noZoom = TRUE;
		  break;

		case 'v':
		  views = atoi(optarg);
		  break;

		case 'w':
		  windowed = TRUE;
		  break;

		case 'l':
		  linear = TRUE;
		  break;

		case 'm':
		  ratio = atof(optarg);
		  break;

		case ':':  // No argument
		  fprintf(stderr,
			  "Option -%c is missing an argument\n",
			  (char)optopt);
		  return 1;

		case '?':
		default:
		  fprintf(stderr,
			  "Unknown option -%c\n",
			  (char)optopt);
		  return 1;
                }
            }
        }
    }

  if (!mapset.maps && !addmaps(".", &mapset))
    {
      fprintf(stderr, "No maps in current directory\n");
      return 1;
    }

  if (views < 0 || views > 16)
    {
      fprintf(stderr, "Unsupported number of views\n");
      return 1;
    }

  {
    GInt32 xPixel;
    GInt32 yPixel;

    double lat;
    double lon;

    int smallest = -1;
    double smallestScale = 0;

    // Find smallest map
    for (i = 0; i < mapset.maps; i++)
      {
	double scale;

	scale2zoom(mapset.dataset[i], mapset.path[i],
		   1, pixel_size, 0, &scale, &scale);

	if (smallest == -1 || scale < smallestScale)
	  {
	    smallest = i;
	    smallestScale = scale;
	  }
      }

    // Center on smallest map
    center2pixel(mapset.dataset[smallest], &xPixel, &yPixel);
    pixel2wgs(mapset.dataset[smallest], mapset.path[smallest],
	      xPixel, yPixel, &lat, &lon);

    if (latSet)
      {
	lat = mapstates[0].req_lat;
      }
    if (lonSet)
      {
	lon = mapstates[0].req_lon;
      }

    // Init mapstates
    for (i = 0; i < views; i++)
      {
	if (i)
	  resetmap(&mapstates[i]);

	mapstates[i].req_lat = lat;
	mapstates[i].req_lon = lon;

	mapstates[i].req_rotation = mapstates[0].req_rotation;

	if (views == mapset.maps)
	  {
	    mapstates[i].dataset[0] = mapset.dataset[i];
	    mapstates[i].path[0] = mapset.path[i];
	    mapstates[i].dataset[1] = NULL;
	    mapstates[i].path[1] = NULL;

	    scale2zoom(mapstates[i].dataset[0], mapstates[i].path[0],
		       1, pixel_size, 0,
		       &mapstates[i].req_scale, &mapstates[i].req_scale);

	    mapstates[i].act_xZoom[0] = 1.0;
	    mapstates[i].act_yZoom[0] = 1.0;
	  }
	else
	  {
	    if (i == 0 && !scaleSet)
	      {
		mapstates[i].dataset[0] = mapset.dataset[smallest];
		mapstates[i].path[0] = mapset.path[smallest];

		scale2zoom(mapset.dataset[smallest], mapset.path[smallest],
			   1, pixel_size, 0,
			   &mapstates[0].req_scale, &mapstates[0].req_scale);
	      }

	    // Set scales relative to smallest map
	    if (i)
	      {
		if (linear)
		  mapstates[i].req_scale = ratio * i * mapstates[0].req_scale;
		else
		  mapstates[i].req_scale = ratio * mapstates[i - 1].req_scale;
	      }

	    selectbestmap(&mapset, &mapstates[i], render_mode, pixel_size);
	  }

	if (noZoom)
	  {
	    mapstates[i].act_xZoom[0] = 1;
	    mapstates[i].act_yZoom[0] = 1;
	  }

	wgs2pixel(mapstates[i].dataset[0], mapstates[i].path[0], lat, lon,
		  &mapstates[i].act_xPixel[0], &mapstates[i].act_yPixel[0]);
      }
  }

  if (views == 1)
    windowed = TRUE;

  CGRect screenRect = CGDisplayBounds(kCGDirectMainDisplay);

  int width = screenRect.size.width;
  int height = screenRect.size.height;

  width = width * 3 / 4;
  height = height * 5 / 8;

  if (windowed)
    {
      width /= 2;
      height /= 2;
    }

  if (windowed)
    {
      for (i = 0; i < views; i++)
	{
	  windows[i] = CreateWindow(width, height, &mapstates[i]);
	}
    }
  else
    {
      windows[0] = CreateWindow(width, height, &mapstates[0]);

      //HIViewRef rootView = HIViewGetRoot(windows[0]);
      Rect rect;

      HIViewRef separator;

      SetRect(&rect, width / 2 - 2, 0, width / 2 + 2, height);
      CreateSeparatorControl(windows[0], &rect, &separator);
      //HIViewAddSubview(rootView, separator);

  //err = HIViewChangeAttributes(view, kHIViewAllowsSubviews, 0);
  //err = HIViewAddSubview(parent, view);
  //err = HIViewSetFrame(view, &bounds);
  //err = HIViewSetVisible(view, true);

/*
      paned = gtk_hpaned_new();

      gtk_container_add(GTK_CONTAINER(windows[0]), paned);

      box1 = gtk_vbox_new(TRUE, 4);
      box2 = gtk_vbox_new(TRUE, 4);

      gtk_paned_pack1(GTK_PANED(paned), box1, FALSE, FALSE);
      gtk_paned_pack2(GTK_PANED(paned), box2, FALSE, FALSE);*/
    }

  for (i = 0; i < views; i++)
    {
      canvases[i].mapstate = &mapstates[i];

      if (windowed)
	{
	  HIViewRef root = NULL;
	  OSStatus err = HIViewFindByID(HIViewGetRoot(windows[i]), kHIViewWindowContentID, &root);

	  HIViewRef view = CreateMapView(root, 0, 0, width, height, &canvases[i]);

	  HILayoutInfo li;
	  li.version = kHILayoutInfoVersionZero;
	  err = HIViewGetLayoutInfo(view, &li);
	  li.binding.top.toView = root;
	  li.binding.top.kind = kHILayoutBindTop;
	  li.binding.bottom.toView = root;
	  li.binding.bottom.kind = kHILayoutBindBottom;
	  li.binding.left.toView = root;
	  li.binding.left.kind = kHILayoutBindLeft;
	  li.binding.right.toView = root;
	  li.binding.right.kind = kHILayoutBindRight;
	  err = HIViewSetLayoutInfo(view, &li);

	  ShowWindow(windows[i]);
	}
      else
	{
	  HIViewRef root = NULL;
	  OSStatus err = HIViewFindByID(HIViewGetRoot(windows[0]),
					kHIViewWindowContentID, &root);

	  int viewTop = height * (i / 2) / ((views + 1) / 2);
	  int viewBot = height * (i / 2 + 1) / ((views + 1) / 2);
	  if ((i & 1 && (!(views & 1) || i < 2)) ||
	      (!(i & 1) && (views & 1) && (i > 1)))
	    {
	  HIViewRef view = CreateMapView(root, 0, viewTop,
					 width / 2 - 2, viewBot, &canvases[i]);
	    }
	  else
	    {
	  HIViewRef view = CreateMapView(root, width / 2 + 2, viewTop,
					 width / 2, viewBot, &canvases[i]);
	    }
	}
    }

  if (!windowed)
    ShowWindow(windows[0]);

#ifdef HAVE_GPS
  init_gps();
#endif

  RunApplicationEventLoop();

#ifdef HAVE_GPS
  free_gps();
#endif

  freemaps(&mapset);

  return 0;
}
