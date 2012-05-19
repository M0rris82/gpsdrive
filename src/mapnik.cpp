#ifdef MAPNIK

#define BOOST_SPIRIT_THREADSAFE

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/version.hpp>
#include <fstream>
#include <sys/stat.h>

#include "mapnik.h"
#include "config.h"
#include "gpsdrive_config.h"


#if MAPNIK_VERSION < 200000
using mapnik::Image32;
using mapnik::Layer;
using mapnik::Envelope;
#else
using mapnik::image_32;
using mapnik::layer;
using mapnik::box2d;
#endif
using mapnik::Map;
using mapnik::coord2d;
using mapnik::feature_ptr;
using mapnik::geometry_ptr;
using mapnik::CoordTransform;
using mapnik::config_error;

extern int mydebug;
extern int borderlimit;

//TODO: get rid of this fixed projection, and use the one the database uses;
//      also adapt the xml-file...
//mapnik::projection Proj("+proj=merc +datum=WGS84");
mapnik::projection Proj("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over");

typedef struct {
    int NewMapYsn;
    int WidthInt;
    int HeightInt;
    double ScaleDbl;
    double NewScaleDbl;
    mapnik::coord2d CenterPt;
    mapnik::Map *MapPtr;
    unsigned char *ImageRawDataPtr;
    GdkPixbuf *GdkPixbufPtr;
    mapnik::coord2d NewCenterPt;
    int RenderYsn;
} MapnikMapStruct;

MapnikMapStruct MapnikMap;
int MapnikInitYsn = 0;

namespace mapnik {

  using namespace std;
  using namespace mapnik;

  void render_thread();

  /*
   * try reading map from cache
   */
  int try_read_tile_from_cache() {

    if (!local_config.mapnik_caching) return 0;

    mapnik::coord2d Pt = MapnikMap.NewCenterPt;
    // to lat and lon
    Proj.inverse(Pt.x, Pt.y);
    gchar mappath[2048];
    gchar filename[2048];

    // use scaleint not newscaleint, thread save, only the renderer can change scalint
    g_snprintf (mappath, sizeof (mappath), "%smapnik_cache/%.0f", local_config.dir_maps, MapnikMap.NewScaleDbl);

    g_snprintf (filename, sizeof (mappath), "%s/%f_%f.png", mappath, Pt.x, Pt.y);

    if(g_file_test (filename, G_FILE_TEST_EXISTS)) {
        MapnikMap.GdkPixbufPtr = gdk_pixbuf_new_from_file (filename, NULL);
        // ok activate new file for display
        MapnikMap.CenterPt = MapnikMap.NewCenterPt;
        MapnikMap.ScaleDbl = MapnikMap.NewScaleDbl;
        MapnikMap.NewMapYsn = true;
        if (mydebug > 3) cout << "mapnik map read from cache." << endl;
        return 1;
    }

    if (mydebug > 3) cout << "mapnik map not found in cache."  << endl;
    return 0;
  }

  /*
   * save to img cache
   */
  void add_current_tile_to_cache() {

    if (!local_config.mapnik_caching) return;

    mapnik::coord2d Pt = MapnikMap.CenterPt;
    // to lat and lon
    Proj.inverse(Pt.x, Pt.y);
    gchar mappath[2048];
    gchar filename[2048];

    // use scaleint not newscaleint, thread save, only the renderer can change scalint
    g_snprintf (mappath, sizeof (mappath), "%smapnik_cache/%.0f", local_config.dir_maps, MapnikMap.ScaleDbl);
	
    // create path if not exists
    if(!g_file_test (mappath, G_FILE_TEST_IS_DIR)) {
        if (!boost::filesystem::create_directories(mappath)) {
            cerr << "Mapnik: Could not create Mapnik temp dir!" << endl;
            return;
        }
    }

    g_snprintf (filename, sizeof (mappath), "%s/%f_%f.png", mappath, Pt.x, Pt.y);

    // save file
    GError **error = NULL; 
    gdk_pixbuf_save(MapnikMap.GdkPixbufPtr, filename, "png", error, NULL);


  }

  /* 
   * little replace function for strings
   */
  string ReplaceString(const string &SearchString, const string &ReplaceString, string StringToReplace)
  {          
    string::size_type pos = StringToReplace.find(SearchString, 0);
    int LengthSearch = SearchString.length();

    while(string::npos != pos )
    {      
        StringToReplace.replace(pos, LengthSearch, ReplaceString);
        pos = StringToReplace.find(SearchString, 0);
    }   
    return StringToReplace;
  }

  /*
   * get center for best tile
   */
  mapnik::coord2d get_best_tile_center(const mapnik::coord2d pPt, int pForceYsn) {
    double scale_denom = MapnikMap.NewScaleDbl;
    double res = scale_denom * 0.00028;
    int WidthInt = MapnikMap.WidthInt - 2 * 256;
    int HeightInt = MapnikMap.HeightInt - 2 * 256;
    
    
    mapnik::coord2d Pt = pPt;
	
    if ((MapnikMap.CenterPt.x + (0.5 * MapnikMap.WidthInt - borderlimit) * res) < Pt.x || 
        (MapnikMap.CenterPt.x - (0.5 * MapnikMap.WidthInt - borderlimit) * res) > Pt.x ||
        (MapnikMap.CenterPt.y + (0.5 * MapnikMap.HeightInt - borderlimit) * res) < Pt.y ||
        (MapnikMap.CenterPt.y - (0.5 * MapnikMap.HeightInt - borderlimit) * res) > Pt.y ||
        pForceYsn) {
        // pos
        
        Pt.x = WidthInt * res * int(Pt.x / (WidthInt * res)) + (Pt.x < 0 ? -1 : 1) * (WidthInt / 2 * res);
        Pt.y = HeightInt * res * int(Pt.y / (HeightInt * res)) + (Pt.y < 0 ? -1 : 1) * (HeightInt / 2 * res);

    } else {
        // take old
        Pt = MapnikMap.CenterPt;
    }

    // Proj.inverse(Pt.x, Pt.y);
    // cout << " -> " << Pt << endl;
    // Proj.forward(Pt.x, Pt.y);


    return Pt;
  }

  /*
   * initialize mapnik
   */
  extern "C"
  void init_mapnik (char *ConfigXML) {

    // register datasources (plug-ins) and fonts
    // Both datasorce_cache and font_engine are 'singletons'.

    std::string input_path(local_config.mapnik_input_path);

    if (mydebug > 10) cout << "datasource_cache::instance()->register_datasources(" << input_path << ")" << endl;
    datasource_cache::instance()->register_datasources(input_path);

    boost::filesystem::directory_iterator end_iter;
    for ( boost::filesystem::directory_iterator dir_itr( local_config.mapnik_font_path );
          dir_itr != end_iter;
          ++dir_itr ) {

        try {

            if ( boost::filesystem::is_regular( dir_itr->status() ) ) {
#if BOOST_VERSION < 103600
                if (mydebug > 10) cout << "freetype_engine::register_font(" << dir_itr->leaf() << ")" << endl;
#else
                if (mydebug > 10) cout << "freetype_engine::register_font(" << dir_itr->filename() << ")" << endl;
#endif
                freetype_engine::register_font( dir_itr->string() );
            }
        } catch ( const std::exception & ex ) {
#if BOOST_VERSION < 103600
            std::cout << dir_itr->leaf() << " " << ex.what() << std::endl;
#else
            std::cout << dir_itr->filename() << " " << ex.what() << std::endl;
#endif
        }

    }

    MapnikMap.WidthInt = 1280;
    MapnikMap.HeightInt = 1024;
    MapnikMap.ScaleDbl = -1; // <-- force creation of map if a map is set
    MapnikMap.MapPtr = new mapnik::Map(MapnikMap.WidthInt, MapnikMap.HeightInt);
    MapnikMap.CenterPt = mapnik::coord2d(0,0);
    MapnikMap.NewCenterPt = mapnik::coord2d(0,0);

    //load map
    try {
        
        if (!boost::filesystem::exists("/usr/share/mapnik/world_boundaries")) {
	    cout << "Missing '/usr/share/mapnik/world_boundaries' please install mapnik world boundaries, if error occurs!" << endl;
        MapnikInitYsn = 0;
	}
	else {
        std::string mapnik_config_file (ConfigXML);
        if (mydebug > 10) cout << "mapnik::load_map('" << mapnik_config_file <<"')" << endl;
        mapnik::load_map(*MapnikMap.MapPtr, mapnik_config_file);
        if (mydebug > 10) cout << "malloc map" << endl;
        MapnikMap.ImageRawDataPtr = (unsigned char *) malloc(MapnikMap.WidthInt * 3 * MapnikMap.HeightInt);
        boost::thread(boost::bind(&render_thread));
        MapnikInitYsn = -1;
	}
    }
    catch(const mapnik::config_error &ex) {
        cerr << "Cannot init mapnik. Mapnik support DISABLED: " << ex.what() << "\n" << endl;
        MapnikInitYsn = 0;
    }
  }

  /*
   * mapnik initialized?
   */
  extern "C"
  int active_mapnik_ysn() {
    if (MapnikInitYsn)
      return -1;
    else
      return 0;
	
  }

  /*
   * Generate the local mapnik config xml
   */
  extern "C"
  int gen_mapnik_config_xml_ysn(char *Dest, char *Username, int night_color_replace) {
	
    if (mydebug > 10) cout << "Check Mapnik config-template: " << local_config.mapnik_xml_template << endl;
    if (mydebug > 10) cout << "Check Mapnik config-file: " << Dest << endl;

    bool have_template = boost::filesystem::exists(local_config.mapnik_xml_template);
    bool have_dest = boost::filesystem::exists(Dest);

    if ( ! have_template)
    {
        have_template = boost::filesystem::exists("./scripts/mapnik/osm-template.xml");
        if (mydebug > 10) cout << "Check Mapnik config-file:  ./scripts/mapnik/osm-template.xml" << endl;
    }
    if ( ! have_template)
    {
        have_template = boost::filesystem::exists("../scripts/mapnik/osm-template.xml");
        if (mydebug > 10) cout << "Check Mapnik config-file:  ../scripts/mapnik/osm-template.xml" << endl;
    }
    if ( ! have_template)
    {
        have_template = boost::filesystem::exists("/usr/local/share/gpsdrive/osm-template.xml");
        if (mydebug > 10) cout << "Check Mapnik config-file:  /usr/local/share/gpsdrive/osm-template.xml" << endl;
    }
    if ( ! have_template)
    {
        have_template = boost::filesystem::exists("/usr/share/gpsdrive/osm-template.xml");
        if (mydebug > 10) cout << "Check Mapnik config-file:  /usr/share/gpsdrive/osm-template.xml" << endl;
    }

    if ( ! have_template && ! have_dest ) {
        // file not found return
        cout << "Cannot find Mapnik config-file: " << Dest << endl;
        cout << "Cannot find Mapnik config-template: " << local_config.mapnik_xml_template << endl;
        return 0;
    }
   
    // load files

    if ( ! have_dest && have_template ) {
        cout << "Generating Mapnik config-file (" << Dest << ") from config-template (" << local_config.mapnik_xml_template << ")" << endl;
    }

    if ( have_dest && have_template ) {
        struct stat dbuf;
        struct stat tbuf;

        if ( !stat(Dest, &dbuf) && !stat(local_config.mapnik_xml_template, &tbuf) ) {
            if ( dbuf.st_mtime > tbuf.st_mtime ) {
                if (mydebug > 0) cout << "Mapnik config-file newer than config-template, NOT regenerating" << endl;
                return -1;
            }
        }
    }
 
    ifstream InputXML (local_config.mapnik_xml_template);
    ofstream DestXML (Dest);
    
    if (mydebug > 0) cout << "Mapnik convert: '" << local_config.mapnik_xml_template << "' ----> '" << Dest << "'" << endl;
    if (mydebug > 0) cout << "Mapnik convert Username: " << Username << endl;

    if (InputXML && DestXML) {
        if (InputXML.is_open()) {
            string s ;
            while (getline(InputXML, s)) {
                s.assign(ReplaceString("@USER@", Username, s));
                if ( night_color_replace ) {
                    s.assign(ReplaceString("bgcolor=\"#b5d0d0\"" , "bgcolor=\"#330033\"", s));
                }
                else {
                    s.assign(ReplaceString("bgcolor=\"#b5d0d0\"" , "bgcolor=\"#ffeed4\"", s));
                }
                DestXML << s  << endl;
            }
        }
    }
    InputXML.close();
    DestXML.close();
    if (mydebug > 0) cout << "Mapnik convert to '"  << Dest << "' done." << endl;
	
    return -1;
  }

  /*
   *  set new map values
   * center lat/lon
   * pForceNewCenterYsn = force maprendering with new center
   * pScaleDbl = gpsdrive scale wanted
   * returing yes/no if a new map should be rendered
   */
  extern "C"
  int set_mapnik_map_ysn(const double pPosLatDbl, const double pPosLonDbl, int pForceNewCenterYsn, const int pScaleDbl) {
    int PanCntInt = 0;
    int OnMapYsn = 0;
    double scale_denom = MapnikMap.ScaleDbl;
    double res = scale_denom * 0.00028;
    int RenderMapYsn = 0;

    if (MapnikMap.RenderYsn) return 1;


    if (pScaleDbl != MapnikMap.ScaleDbl) {
        /* new  scale */
        MapnikMap.NewScaleDbl = pScaleDbl;
        pForceNewCenterYsn = 1; /* we always force the center */
    }
	
    mapnik::coord2d Pt = mapnik::coord2d(pPosLonDbl, pPosLatDbl);
    Proj.forward(Pt.x, Pt.y);

    mapnik::coord2d BestPt = get_best_tile_center(Pt, pForceNewCenterYsn);

    if (BestPt.x != MapnikMap.CenterPt.x || BestPt.y != MapnikMap.CenterPt.y || pForceNewCenterYsn) {
        MapnikMap.NewCenterPt = BestPt;
        if (!try_read_tile_from_cache()) {
            MapnikMap.RenderYsn = true;
        }
    }

    return 1;
	
    /* force new center */
    if (pForceNewCenterYsn) {
        MapnikMap.NewCenterPt = mapnik::coord2d(pPosLonDbl, pPosLatDbl);
        Proj.forward(MapnikMap.NewCenterPt.x, MapnikMap.NewCenterPt.y);
        RenderMapYsn = 1;
    }
    /* if a new map should be rendered, 
     * then caculate new center pix coord
     * else out of allowed map aerea? pan!*/
    if (!RenderMapYsn) {
        /* out of allowed map area? pan! if more then 10 times to pan center to map*/
        mapnik::coord2d Pt = mapnik::coord2d(pPosLonDbl, pPosLatDbl);
        Proj.forward(Pt.x, Pt.y);
        while (!OnMapYsn && PanCntInt < 10) {
            OnMapYsn = 1;
            /* pan right or left? */
            if ((MapnikMap.NewCenterPt.x + (0.5 * MapnikMap.WidthInt - borderlimit) * res) < Pt.x) {
                /* pan right */
                if (mydebug > 30) cout << "pan right\n";
                MapnikMap.NewCenterPt.x = MapnikMap.NewCenterPt.x + (MapnikMap.WidthInt - borderlimit * 2) * res;
                PanCntInt += 1;
                OnMapYsn = 0;
            } else if ((MapnikMap.NewCenterPt.x - (0.5 * MapnikMap.WidthInt - borderlimit) * res) > Pt.x) {
                /* pan left */
                if (mydebug > 30) cout << "pan left\n";
                MapnikMap.NewCenterPt.x = MapnikMap.NewCenterPt.x - (MapnikMap.WidthInt - borderlimit * 2) * res;
                PanCntInt += 1;
                OnMapYsn = 0;
            }
            /* pan up or down? */
            if ((MapnikMap.NewCenterPt.y + (0.5 * MapnikMap.HeightInt - borderlimit) * res) < Pt.y) {
                /* pan up */
                if (mydebug > 30) cout << "pan up\n";
                MapnikMap.NewCenterPt.y = MapnikMap.NewCenterPt.y + (MapnikMap.HeightInt - borderlimit * 2) * res;
                PanCntInt += 1;
                OnMapYsn = 0;
            } else if ((MapnikMap.NewCenterPt.y - (0.5 * MapnikMap.HeightInt - borderlimit) * res) > Pt.y) {
                /* pan down */
                if (mydebug > 30) cout << "pan down\n";
                MapnikMap.NewCenterPt.y = MapnikMap.NewCenterPt.y - (MapnikMap.HeightInt - borderlimit * 2) * res;
                PanCntInt += 1;
                OnMapYsn = 0;
            }
        }
		
        if (PanCntInt > 0 && OnMapYsn) {
            /* render map */
            RenderMapYsn = 1;
        } else if (PanCntInt) {
            RenderMapYsn = 1;
            MapnikMap.NewCenterPt.x = pPosLonDbl;
            MapnikMap.NewCenterPt.y = pPosLatDbl;
            Proj.forward(MapnikMap.NewCenterPt.x, MapnikMap.NewCenterPt.y);
        }
    }
    cout << "New: " << MapnikMap.NewCenterPt << endl;

    if (RenderMapYsn) MapnikMap.RenderYsn = true;
  }

  /*
   * convert the color channel
   */
  inline unsigned char
  convert_color_channel (unsigned char Source, unsigned char Alpha) {
    return Alpha ? ((Source << 8) - Source) / Alpha : 0;
  }

  /*
   * converting argb32 to gdkpixbuf
   */
  void
  convert_argb32_to_gdkpixbuf_data (unsigned char const *Source, unsigned char *Dest) {
    unsigned char const *SourcePixel = Source;
    unsigned char *DestPixel = Dest;
    for (int y = 0; y < MapnikMap.HeightInt; y++) {
        for (int x = 0; x < MapnikMap.WidthInt; x++) {
            DestPixel[0] = convert_color_channel(SourcePixel[0], SourcePixel[3]);
            DestPixel[1] = convert_color_channel(SourcePixel[1], SourcePixel[3]);
            DestPixel[2] = convert_color_channel(SourcePixel[2], SourcePixel[3]);
            DestPixel += 3;
            SourcePixel += 4;
        }
    }
  }

  void render_thread() {

    //loop infinite
    while (1==1) {

        // TODO: better blocking system - wait mutex needed
        // wait for jobs
        sleep(1);
        if (MapnikMap.RenderYsn) {
            double scale_denom = MapnikMap.NewScaleDbl;
            double res = scale_denom * 0.00028;
		
            /* render image */
#if MAPNIK_VERSION < 200000
            Envelope<double> box = Envelope<double>(MapnikMap.NewCenterPt.x - 0.5 * MapnikMap.WidthInt * res,
                                                    MapnikMap.NewCenterPt.y - 0.5 * MapnikMap.HeightInt * res,
                                                    MapnikMap.NewCenterPt.x + 0.5 * MapnikMap.WidthInt * res,
                                                    MapnikMap.NewCenterPt.y + 0.5 * MapnikMap.HeightInt * res);

            MapnikMap.MapPtr->zoomToBox(box);

            Image32 buf(MapnikMap.WidthInt, MapnikMap.HeightInt);
            mapnik::agg_renderer<Image32> ren(*MapnikMap.MapPtr,buf);
            ren.apply();

            if (mydebug > 0) std::cout << MapnikMap.MapPtr->getCurrentExtent() << "\n";
#else
	    box2d<double> box = box2d<double>(MapnikMap.NewCenterPt.x - 0.5 * MapnikMap.WidthInt * res,
                                              MapnikMap.NewCenterPt.y - 0.5 * MapnikMap.HeightInt * res,
                                              MapnikMap.NewCenterPt.x + 0.5 * MapnikMap.WidthInt * res,
                                              MapnikMap.NewCenterPt.y + 0.5 * MapnikMap.HeightInt * res);

            MapnikMap.MapPtr->zoom_to_box(box);

            image_32 buf(MapnikMap.WidthInt, MapnikMap.HeightInt);
            mapnik::agg_renderer<image_32> ren(*MapnikMap.MapPtr,buf);
            ren.apply();

            if (mydebug > 0) std::cout << MapnikMap.MapPtr->get_current_extent() << "\n";
#endif
            /* get raw data for gpsdrives pixbuf */
            convert_argb32_to_gdkpixbuf_data(buf.raw_data(), MapnikMap.ImageRawDataPtr);

            /* create pixbuf data before check to unref */
            if (MapnikMap.GdkPixbufPtr)
		gdk_pixbuf_unref(MapnikMap.GdkPixbufPtr);

            MapnikMap.GdkPixbufPtr = gdk_pixbuf_new_from_data(MapnikMap.ImageRawDataPtr,
				GDK_COLORSPACE_RGB, FALSE, 8, 1280, 1024, 1280 * 3, NULL, NULL);

            /* ok we have a map set default values */
#if MAPNIK_VERSION < 200000
            mapnik::Envelope<double> ext = MapnikMap.MapPtr->getCurrentExtent();
#else
            mapnik::box2d<double> ext = MapnikMap.MapPtr->get_current_extent();
#endif
            mapnik::coord2d pt = ext.center();
            MapnikMap.CenterPt.x = pt.x;
            MapnikMap.CenterPt.y = pt.y;
            Proj.inverse(pt.x, pt.y);
            MapnikMap.ScaleDbl = MapnikMap.NewScaleDbl;

            // inform that there is a new map
            MapnikMap.NewMapYsn = 1;

            //add to cache
            add_current_tile_to_cache();

            MapnikMap.RenderYsn = 0;
        }	
    }
  }


  /*
   * return pointer to imagedata for gpsdrive
   */
  extern "C"
  GdkPixbuf *get_mapnik_gdk_pixbuf() {
    return MapnikMap.GdkPixbufPtr;
  }

  /*
   * return selected mapscale
   */
  extern "C"
  double get_mapnik_mapscale() {
    return MapnikMap.ScaleDbl;
  }

  /* 
   * return pixelfactor
   */
  extern "C"
  double get_mapnik_pixelfactor() {
    return MapnikMap.ScaleDbl * 0.00028;
  }

  /*
   * return if a new map was rendered
   */
  extern "C"
  int get_mapnik_newmapysn() {
    return MapnikMap.NewMapYsn;
  }

  /* 
   * set new map
   */
  extern "C"
  int set_mapnik_newmapysn(const int pInt) {
    MapnikMap.NewMapYsn = pInt;
    return 1;
  }

  /*
   * return mapcenter of actual rendered map
   */
  extern "C"
  void get_mapnik_center(double *pLatDbl, double *pLonDbl) {
    *pLatDbl = MapnikMap.CenterPt.y;
    *pLonDbl = MapnikMap.CenterPt.x;
    Proj.inverse(*pLonDbl, *pLatDbl);
  }

  /*
   * wraper function for gpsdrive
   */
  extern "C"
  void get_mapnik_calcxytopos(double *pLatDbl, double *pLonDbl, int pXInt, int pYInt, int pXOffInt, int pYOffInt, int pZoom, int mapx2Int, int mapy2Int) {
    double XDbl = (mapx2Int - pXInt - pXOffInt) * MapnikMap.ScaleDbl * 0.00028 / pZoom;
    double YDbl = (mapy2Int - pYInt - pYOffInt) * MapnikMap.ScaleDbl * 0.00028 / pZoom;
    double LonDbl = MapnikMap.CenterPt.x - XDbl;
    double LatDbl = MapnikMap.CenterPt.y + YDbl;
    Proj.inverse(LonDbl, LatDbl);
    *pLonDbl = LonDbl;
    *pLatDbl = LatDbl;
  }

  /*
   * wraper function for gpsdrive
   */
  extern "C"
  void get_mapnik_calcxy(int *pXInt, int *pYInt, double pLatDbl, double pLonDbl, int pXOffInt, int pYOffInt, int pZoom, int mapx2Int, int mapy2Int) {
	
    double X = pLonDbl;
    double Y = pLatDbl;
    Proj.forward(X, Y);
    X = X - MapnikMap.CenterPt.x;
    Y = Y - MapnikMap.CenterPt.y;
	
    *pXInt = int( double( 0.5 + (mapx2Int + X * pZoom / (MapnikMap.ScaleDbl * 0.00028)) - pXOffInt) );
    *pYInt = int( double(0.5 + (mapy2Int - Y * pZoom / (MapnikMap.ScaleDbl * 0.00028)) - pYOffInt) );

  }

  /*
   * wraper function for gpsdrive
   */
  extern "C"
  void get_mapnik_minixy2latlon(int pXInt, int pYInt, double *pLatDbl, double *pLonDbl) {
    double XDbl = pXInt;
    double YDbl = pYInt;
    double LonDbl = MapnikMap.CenterPt.x - XDbl;
    double LatDbl = MapnikMap.CenterPt.y - YDbl;
    Proj.inverse(LonDbl, LatDbl);
    *pLonDbl = LonDbl;
    *pLatDbl = LatDbl;
  }

  /*
   * wraper function for gpsdrive
   */
  extern "C"
  void get_mapnik_minicalcxy(int *pXDbl, int *pYDbl, double pLatDbl, double pLonDbl, int pZoom) {
    double X = pLonDbl;
    double Y = pLatDbl;
    Proj.forward(X, Y);
    X = X - MapnikMap.CenterPt.x;
    Y = Y - MapnikMap.CenterPt.y;

    *pXDbl = int(double(64 + X * pZoom / (MapnikMap.ScaleDbl * 0.00028 * 10)));
    *pYDbl = int(double(51 - Y * pZoom / (MapnikMap.ScaleDbl * 0.00028 * 10)));

  }

  /*
   * wrapper function for gpsdrive
   */
  extern "C"
  void convert_mapnik_coords(double *pXDbl, double *pYDbl, double pLonDbl, double pLatDbl, int mode)
  {
    double X = pLonDbl;
    double Y = pLatDbl;

    if (mode == 0)
    {
        // convert lat/lon coords to mercator projection used in postgis
        Proj.forward(X, Y);
    }
    else
    {
        // convert mercator projection used in postgis to lat/lon coords
        Proj.inverse(X, Y);
    }

    *pXDbl = X;
    *pYDbl = Y;
  }


} //end namespace mapnik

#endif
