#ifdef MAPNIK

/*
#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QPen>
*/

#include <iostream>
#include <fstream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp> 
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <string>
#include <fstream>

#include "mapnik.h"
#include "config.h"

using mapnik::Image32;
using mapnik::Map;
using mapnik::Layer;
using mapnik::Envelope;
using mapnik::coord2d;
using mapnik::feature_ptr;
using mapnik::geometry_ptr;
using mapnik::CoordTransform;

extern int mydebug;
extern int borderlimit;
extern int SCREEN_X_2;
extern int SCREEN_Y_2;

mapnik::projection Proj("+proj=merc +datum=WGS84");

typedef struct {
	int WidthInt;
	int HeightInt;
	int BorderlimitInt;
	double CenterLatDbl;
	double CenterLonDbl;
	mapnik::coord2d CenterPt;
	int RenderMapYsn;
	double ScaleInt;
	unsigned char *ImageRawDataPtr;
	mapnik::Map *MapPtr;
	int NewMapYsn;
} MapnikMapStruct;

MapnikMapStruct MapnikMap;
int MapnikInitYsn = 0;

namespace mapnik {

using namespace std;
using namespace mapnik;

/*
double scales [] = {279541132.014,
                    139770566.007,
                    69885283.0036,
                    34942641.5018,
                    17471320.7509,
                    8735660.37545,
                    4367830.18772,
                    2183915.09386,
                    1091957.54693,
                    545978.773466,
                    272989.386733,
                    136494.693366,
                    68247.3466832,
                    34123.6733416,
                    17061.8366708,
                    8530.9183354,
                    4265.4591677,
                    2132.72958385,
                    1066.36479192,
                    533.182395962};
const int MIN_LEVEL = 0;
const int MAX_LEVEL = 18;
*/

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
 * initialize mapnik
 */
extern "C"
void init_mapnik (char *ConfigXML) {

	// register datasources (plug-ins) and a font
    // Both datasorce_cache and font_engine are 'singletons'.
   
    datasource_cache::instance()->register_datasources("/usr/lib/mapnik/input/");
    freetype_engine::instance()->register_font("/usr/lib/mapnik/fonts/DejaVuSans.ttf");
    
    MapnikMap.WidthInt = 1280;
    MapnikMap.HeightInt = 1024;
    MapnikMap.BorderlimitInt = borderlimit;
    MapnikMap.ScaleInt = -1; // <-- force creation of map if a map is set
    MapnikMap.MapPtr = new mapnik::Map(MapnikMap.WidthInt, MapnikMap.HeightInt);
    
    //load map
    std::string mapnik_config_file (ConfigXML);
    mapnik::load_map(*MapnikMap.MapPtr, mapnik_config_file);
    MapnikInitYsn = -1;
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
int gen_mapnik_config_xml_ysn(char *Dest, char *Username) {
	
    // This location has to be adapted in the future
    // for now it should work if gpsdrive is installed in the standard location   
    string mapnik_config_file("./scripts/mapnik/osm.xml");
    if ( ! boost:: filesystem::exists(mapnik_config_file) ) 
	mapnik_config_file.assign("../scripts/mapnik/osm.xml");
    if ( ! boost:: filesystem::exists(mapnik_config_file) ) 
	mapnik_config_file.assign(DATADIR).append("/mapnik/osm.xml");
    cout << "Using Mapnik config-file: " << mapnik_config_file << endl;
    
    if ( ! boost:: filesystem::exists(mapnik_config_file) ) {
    	// file not found return
    	return 0;
    }
    
    // load files
 
	ifstream InputXML (mapnik_config_file.c_str());
	ofstream DestXML (Dest);
	
	if (InputXML && DestXML) {
		if (InputXML.is_open()) {
			string s ;
			while (getline(InputXML, s)) {
				DestXML << ReplaceString("@USER@", Username, s) << endl;
			}
		}
	}
	InputXML.close();
	DestXML.close();
	
	return -1;
}

/***
 *  set new map values
 * center lat/lon
 * pForceNewCenterYsn = force maprendering with new center
 * pScaleInt = gpsdrive scale wanted
 ***/
extern "C"
void set_mapnik_map(double pPosLatDbl, double pPosLonDbl, int pForceNewCenterYsn, int pScaleInt) {
	int PanCntInt = 0;
	int OnMapYsn = 0;
	double scale_denom = MapnikMap.ScaleInt;
	double res = scale_denom * 0.00028;
	/* first we disable the map rendering 
	 * and test if we need to render a new map */
	MapnikMap.RenderMapYsn = 0;
	
	
	if (pScaleInt != MapnikMap.ScaleInt) {
		/* new  scale */
		MapnikMap.ScaleInt = pScaleInt;
		pForceNewCenterYsn = 1; /* we always force the center */
	}
	
	/* force new center */
	if (pForceNewCenterYsn) {
		MapnikMap.CenterLatDbl = pPosLatDbl;
		MapnikMap.CenterLonDbl = pPosLonDbl;
		MapnikMap.RenderMapYsn = 1;
	}
	/* if a new map should be rendered, 
	 * then caculate new center pix coord
	 * else out of allowed map aerea? pan!*/
	if (MapnikMap.RenderMapYsn) {
		/* calc new center pix */
		MapnikMap.CenterPt.x = MapnikMap.CenterLonDbl;
		MapnikMap.CenterPt.y = MapnikMap.CenterLatDbl;
		Proj.forward(MapnikMap.CenterPt.x, MapnikMap.CenterPt.y);
	} else {
		/* out of allowed map area? pan! if more then 10 times to pan center to map*/
		while (!OnMapYsn && PanCntInt < 10) {
			OnMapYsn = 1;
			mapnik::coord2d Pt = mapnik::coord2d(pPosLonDbl, pPosLatDbl);
			Proj.forward(Pt.x, Pt.y);
			/* pan right or left? */
			if ((MapnikMap.CenterPt.x + (0.5 * MapnikMap.WidthInt - MapnikMap.BorderlimitInt) * res) < Pt.x) {
				cout << "pan right\n";
				/* pan right */
				MapnikMap.CenterPt.x = MapnikMap.CenterPt.x + (MapnikMap.WidthInt - MapnikMap.BorderlimitInt * 2) * res;
				PanCntInt += 1;
				OnMapYsn = 0;
			} else if ((MapnikMap.CenterPt.x - (0.5 * MapnikMap.WidthInt - MapnikMap.BorderlimitInt) * res) > Pt.x) {
				/* pan left */
				cout << "pan left\n";
				MapnikMap.CenterPt.x = MapnikMap.CenterPt.x - (MapnikMap.WidthInt - MapnikMap.BorderlimitInt * 2) * res;
				PanCntInt += 1;
				OnMapYsn = 0;
			}
			/* pan up or down? */
			if ((MapnikMap.CenterPt.y + (0.5 * MapnikMap.HeightInt - MapnikMap.BorderlimitInt) * res) < Pt.y) {
				cout << "pan up\n";
				/* pan up */
				MapnikMap.CenterPt.y = MapnikMap.CenterPt.y + (MapnikMap.HeightInt - MapnikMap.BorderlimitInt * 2) * res;
				PanCntInt += 1;
				OnMapYsn = 0;
			} else if ((MapnikMap.CenterPt.y - (0.5 * MapnikMap.HeightInt - MapnikMap.BorderlimitInt) * res) > Pt.y) {
				/* pan down */
				cout << "pan down\n";
				MapnikMap.CenterPt.y = MapnikMap.CenterPt.y - (MapnikMap.HeightInt - MapnikMap.BorderlimitInt * 2) * res;
				PanCntInt += 1;
				OnMapYsn = 0;
			}
		}
		
		if (PanCntInt > 0 && OnMapYsn) {
			/* render map */
			MapnikMap.RenderMapYsn = 1;
			/* calc new lat/lon */
			MapnikMap.CenterLonDbl = MapnikMap.CenterPt.x;
			MapnikMap.CenterLatDbl = MapnikMap.CenterPt.y;
			Proj.inverse(MapnikMap.CenterLonDbl, MapnikMap.CenterLatDbl);
		} else if (PanCntInt) {
			MapnikMap.CenterLatDbl = pPosLatDbl;
			MapnikMap.CenterLonDbl = pPosLonDbl;
			MapnikMap.RenderMapYsn = 1;
			MapnikMap.CenterPt.x = MapnikMap.CenterLonDbl;
			MapnikMap.CenterPt.y = MapnikMap.CenterLatDbl;
			Proj.forward(MapnikMap.CenterPt.x, MapnikMap.CenterPt.y);
		}
	}
	

	//Check level
	/*if (MapnikMap.ScaleInt < MIN_LEVEL) MapnikMap.ScaleInt = MIN_LEVEL;
	if (MapnikMap.ScaleInt > MAX_LEVEL) MapnikMap.ScaleInt = MAX_LEVEL;
*/
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

/* 
 * is there a new map to render?
 */
extern "C"
void render_mapnik () {

	MapnikMap.NewMapYsn = false;
	if (!MapnikMap.RenderMapYsn) return;
	

    
    //double scale_denom = scales[MapnikMap.ScaleInt];
    double scale_denom = MapnikMap.ScaleInt;
    double res = scale_denom * 0.00028;
    
   /* render image */
    Envelope<double> box = Envelope<double>(MapnikMap.CenterPt.x - 0.5 * MapnikMap.WidthInt * res,
    					MapnikMap.CenterPt.y - 0.5 * MapnikMap.HeightInt * res,
    					MapnikMap.CenterPt.x + 0.5 * MapnikMap.WidthInt * res,
    					MapnikMap.CenterPt.y + 0.5 * MapnikMap.HeightInt * res);
    
    MapnikMap.MapPtr->zoomToBox(box);
    
    Image32 buf(MapnikMap.WidthInt, MapnikMap.HeightInt);
    mapnik::agg_renderer<Image32> ren(*MapnikMap.MapPtr,buf);
    ren.apply();
    
    if (mydebug > 0) std::cout << MapnikMap.MapPtr->getCurrentExtent() << "\n";
    
    /* get raw data for gpsdrives pixbuf */
    if (!MapnikMap.ImageRawDataPtr) {
    	MapnikMap.ImageRawDataPtr = (unsigned char *) malloc(MapnikMap.WidthInt * 3 * MapnikMap.HeightInt);
    }
    convert_argb32_to_gdkpixbuf_data(buf.raw_data(), MapnikMap.ImageRawDataPtr);
    
    /* ok we have a map set default values */
    MapnikMap.NewMapYsn = true;
    mapnik::Envelope<double> ext = MapnikMap.MapPtr->getCurrentExtent();
    mapnik::coord2d pt = ext.center();
    MapnikMap.CenterPt.x = pt.x;
    MapnikMap.CenterPt.y = pt.y;
    Proj.inverse(pt.x, pt.y);
    MapnikMap.CenterLonDbl = pt.x;
    MapnikMap.CenterLatDbl = pt.y;

}

/*
 * return pointer to imagedata for gpsdrive
 */
extern "C"
unsigned char *get_mapnik_imagedata() {
	return MapnikMap.ImageRawDataPtr;
}

/*
 * return selected mapscale
 */
extern "C"
double get_mapnik_mapscale() {
	return MapnikMap.ScaleInt;
}

/* 
 * return pixelfactor
 */
extern "C"
double get_mapnik_pixelfactor() {
	return MapnikMap.ScaleInt * 0.00028;
}

/*
 * return if a new map was rendered
 */
extern "C"
int get_mapnik_newmapysn() {
	return MapnikMap.NewMapYsn;
}

/*
 * return mapcenter of actual rendered map
 */
extern "C"
void get_mapnik_center(double *pLatDbl, double *pLonDbl) {
	*pLatDbl = MapnikMap.CenterLatDbl;
	*pLonDbl = MapnikMap.CenterLonDbl;
}

/*
 * wraper function for gpsdrive
 */
extern "C"
void get_mapnik_clacxytopos(double *pLatDbl, double *pLonDbl, int pXInt, int pYInt, int pXOffInt, int pYOffInt, int pZoom) {
	double XDbl = (SCREEN_X_2 - pXInt - pXOffInt) * MapnikMap.ScaleInt * 0.00028 / pZoom;
	double YDbl = (SCREEN_Y_2 - pYInt - pYOffInt) * MapnikMap.ScaleInt * 0.00028 / pZoom;
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
void get_mapnik_clacxy(double *pXDbl, double *pYDbl, double pLatDbl, double pLonDbl, int pXOffInt, int pYOffInt, int pZoom) {
	
	double X = pLonDbl;
	double Y = pLatDbl;
	Proj.forward(X, Y);
	X = X - MapnikMap.CenterPt.x;
	Y = Y - MapnikMap.CenterPt.y;
	
	 *pXDbl = (SCREEN_X_2 + X * pZoom / (MapnikMap.ScaleInt * 0.00028)) - pXOffInt;
	 *pYDbl = (SCREEN_Y_2 - Y * pZoom / (MapnikMap.ScaleInt * 0.00028)) - pYOffInt;

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
void get_mapnik_miniclacxy(double *pXDbl, double *pYDbl, double pLatDbl, double pLonDbl, int pZoom) {
	double X = pLonDbl;
	double Y = pLatDbl;
	Proj.forward(X, Y);
	X = X - MapnikMap.CenterPt.x;
	Y = Y - MapnikMap.CenterPt.y;
	
	 *pXDbl = (64 + X * pZoom / (MapnikMap.ScaleInt * 0.00028 * 10));
	 *pYDbl = (51 - Y * pZoom / (MapnikMap.ScaleInt * 0.00028 * 10));

}


} //end namespace mapnik

#endif
