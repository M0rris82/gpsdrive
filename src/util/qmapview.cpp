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

// Qt API reference:
// http://doc.trolltech.com/

#include "../lib_map/map.h"

#include <qapplication.h>
#include <qmainwindow.h>
#include <qwidget.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <stdio.h>
#ifdef USE_CAIRO
#include <cairo.h>
#include <cairo-xlib.h>
#endif

#include <unistd.h>
#include <stdlib.h>

extern "C" {
  void init_gps();
  void free_gps();
}

#define PIXEL_SIZE 0.22E-3
#define SCALE_RATIO 10

class MapWidget : public QWidget
{
  Q_OBJECT
public:
  MapWidget(QWidget *parent, MapState *mapstate);
  ~MapWidget();

protected:
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

private:
#ifdef USE_CAIRO
  cairo_t *m_cr;
  cairo_surface_t *m_surf;
#endif
  MapState *m_mapstate;
};

MapSettings mapsettings;
MapSet mapset;
MapState mapstates[16];
MapWidget *canvases[16];
int views = 1;
int render_mode = 0x100;
int cover = 0;
int transparent = 0;
double pixel_size = PIXEL_SIZE;
int noZoom = 0;


MapWidget::MapWidget(QWidget *parent, MapState *mapstate):
  QWidget(parent, "MapWidget"),
  m_mapstate(mapstate)
{
#ifdef USE_CAIRO
  m_surf = cairo_xlib_surface_create((Display *)x11AppDisplay(),
				     (Drawable)handle(),
				     (Visual *)x11Visual(),
				     width(), height());
  m_cr = cairo_create(m_surf);
  cairo_surface_destroy(m_surf);
#endif

  setFocus();
  setFocusPolicy(QWidget::StrongFocus);
  setBackgroundMode(Qt::NoBackground);
}

MapWidget::~MapWidget()
{
#ifdef USE_CAIRO
  cairo_destroy(m_cr);
  m_cr = 0;
#endif
}

void MapWidget::paintEvent(QPaintEvent *)
{
#ifdef USE_CAIRO
  MapGC mgc;

  memset((void*)&mgc, 0, sizeof(mgc));

  m_surf = cairo_get_target(m_cr);
  cairo_xlib_surface_set_drawable(m_surf, handle(), width(), height());
  mgc.cairo_cr = m_cr;

  if (cover)
    coverifpossible(m_mapstate, width(), height());

  cairo_save(m_cr);

  drawmap(m_mapstate, &mgc, width(), height(), transparent);

/* where's the code??
  drawmarkers(&mgc, width(), height(),
	      &mapsettings, m_mapstate, pixel_size,
	      0, 0);
*/
  cairo_restore(m_cr);
#else
  QPainter painter(this);

  MapGC mgc;

  memset((void*)&mgc, 0, sizeof(mgc));

  mgc.qt_painter = &painter;

  if (cover)
    coverifpossible(m_mapstate, width(), height());

  drawmap(m_mapstate, &mgc, width(), height(), transparent);

/* where's the code??
  drawmarkers(&mgc, width(), height(),
	      &mapsettings, m_mapstate, pixel_size,
	      0, 0);
*/
#endif
}

void MapWidget::mousePressEvent(QMouseEvent *event)
{
  int i;

  mapsettings.posmode = TRUE;

  screen2wgs(m_mapstate, event->x(), event->y(),
	     &m_mapstate->req_lat, &m_mapstate->req_lon);

  fprintf(stderr, "new center: %lf;%lf\n",
	  m_mapstate->req_lat, m_mapstate->req_lon);

  for (i = 0; i < views; i++)
    {
      mapstates[i].req_lat = m_mapstate->req_lat;
      mapstates[i].req_lon = m_mapstate->req_lon;

      if (views != mapset.maps)
	{
	  selectbestmap(&mapset, &mapstates[i], render_mode, pixel_size);
	}
      else
	{
	  double pseudo_lat = (render_mode & 0x100) ? m_mapstate->req_lat : 0;

	  wgs2pixel(mapstates[i].dataset[0], mapstates[i].path[0],
		    m_mapstate->req_lat, m_mapstate->req_lon,
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

      canvases[i]->update();
    }
}

void MapWidget::keyPressEvent(QKeyEvent *event)
{
  int i;

  int redraw = 0;

  double factor = 1;

  switch (event->ascii())
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
      transparent = !transparent;
      redraw = 1;
      break;

    case 'r':
      m_mapstate->req_rotation += 45;
      if (m_mapstate->req_rotation >= 360)
	m_mapstate->req_rotation -= 360;
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
	    double pseudo_lat = (render_mode & 0x100) ?
	      m_mapstate->req_lat : 0;

	    wgs2pixel(mapstates[i].dataset[0], mapstates[i].path[0],
		      m_mapstate->req_lat, m_mapstate->req_lon,
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

	canvases[i]->update();
      }

  render_mode &= ~0x200;
}

void usage()
{
  fprintf(stderr, "libmap test and sample application\n\n");
  fprintf(stderr, " usage: qmapview [input map files or directories] [options and arguments]\n\n");
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
  fprintf(stderr, "\n  When qmapview is running it can be controlled with the following keys:\n");
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

  int latSet = FALSE;
  int lonSet = FALSE;
  int scaleSet = FALSE;
  int linear = FALSE;
  int windowed = FALSE;
  double ratio = SCALE_RATIO;

  QApplication theApp(argc, argv);

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

  // Create windows and views

  QMainWindow *windows[16];
  QSplitter *box1 = 0;
  QSplitter *box2 = 0;

  if (views == 1)
    windowed = TRUE;

  if (windowed)
    {
      for (i = 0; i < views; i++)
	{
	  windows[i] = new QMainWindow();
	}
    }
  else
    {
      windows[0] = new QMainWindow();

      QSplitter *split = new QSplitter(windows[0]);
      windows[0]->setCentralWidget(split);

      box1 = new QSplitter(split);
      box1->setOrientation(QSplitter::Vertical);
      box2 = new QSplitter(split);
      box2->setOrientation(QSplitter::Vertical);
    }

  for (i = 0; i < views; i++)
    {
      if (windowed)
	{
	  canvases[i] = new MapWidget(windows[i], &mapstates[i]);

	  windows[i]->setCentralWidget(canvases[i]);

	  windows[i]->show();
	}
      else
	{
	  if ((i & 1 && (!(views & 1) || i < 2)) ||
	      (!(i & 1) && (views & 1) && (i > 1)))
	    {
	      canvases[i] = new MapWidget(box2, &mapstates[i]);
	    }
	  else
	    {
	      canvases[i] = new MapWidget(box1, &mapstates[i]);
	    }
	}
    }

  theApp.setMainWidget(windows[0]);

  if (!windowed)
    windows[0]->show();

#ifdef HAVE_GPS
  init_gps();
#endif

  int ret = theApp.exec();

#ifdef HAVE_GPS
  free_gps();
#endif

  freemaps(&mapset);

  return ret;
}

// This line is necessary when there are no separate header files:
// http://doc.trolltech.com/3.1/moc.html
#include "qmapview.moc"
