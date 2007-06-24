#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QPen>

#include <iostream>
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
#include "mapnik.h"
#include <string>
#include <fstream>

using mapnik::Image32;
using mapnik::Map;
using mapnik::Layer;
using mapnik::Envelope;
using mapnik::coord2d;
using mapnik::feature_ptr;
using mapnik::geometry_ptr;
using mapnik::CoordTransform;

namespace mapnik {

using namespace std;

typedef struct {
	int WidthInt;
	int HeightInt;
	double CenterLatDbl;
	double CenterLonDbl;
	int ScaleLevelInt;
	unsigned char *ImageRawDataPtr;
	Map map;
} MapnikMapStruct;

MapnikMapStruct MapnikMap;

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

extern "C"
void init_mapnik ( ) {
    // register datasources (plug-ins) and a font
    // Both datasorce_cache and font_engine are 'singletons'.
    
    using namespace mapnik;
    datasource_cache::instance()->register_datasources("/usr/lib/mapnik/input/");
    freetype_engine::instance()->register_font("/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf");
    
    MapnikMap.WidthInt = 1280;
    MapnikMap.HeightInt = 1024;
    MapnikMap.map = Map(MapnikMap.WidthInt, MapnikMap.HeightInt);
    
    // This location has to be adapted in the future
    // for now it should work if gpsdrive is installed in the standard location   
    string mapnik_config_file("./scripts/mapnik/osm.xml");
    if ( ! boost:: filesystem::exists(mapnik_config_file) ) 
	mapnik_config_file.assign("../scripts/mapnik/osm.xml");
    if ( ! boost:: filesystem::exists(mapnik_config_file) ) 
	mapnik_config_file.assign(DATADIR).append("/mapnik/osm.xml");
    cout << "Using Mapnik config-file: " << mapnik_config_file << endl;
    
    //load map
    mapnik::load_map(MapnikMap.map, mapnik_config_file);
}

extern "C"
void set_mapnik_map(double pCenterLatDbl, double pCenterLonDbl, int pScaleLevelInt) {
	MapnikMap.CenterLatDbl = pCenterLatDbl;
	MapnikMap.CenterLonDbl = pCenterLonDbl;
	MapnikMap.ScaleLevelInt = pScaleLevelInt;
	//Check level
	if (MapnikMap.ScaleLevelInt < MIN_LEVEL) MapnikMap.ScaleLevelInt = MIN_LEVEL;
	if (MapnikMap.ScaleLevelInt > MAX_LEVEL) MapnikMap.ScaleLevelInt = MAX_LEVEL;
}

extern "C"
void render_mapnik (void) {

    projection Proj("+proj=merc +datum=WGS84");
    
    double scale_denom = scales[MapnikMap.ScaleLevelInt];
    double res = scale_denom * 0.00028;
    
    mapnik::coord2d Pt;
    Pt.x = MapnikMap.CenterLonDbl;
    Pt.y = MapnikMap.CenterLatDbl;
    Proj.forward(Pt.x, Pt.y);
    
    Envelope<double> box = Envelope<double>(Pt.x - 0.5 * MapnikMap.WidthInt * res,
					    Pt.y - 0.5 * MapnikMap.HeightInt * res,
					    Pt.x + 0.5 * MapnikMap.WidthInt * res,
					    Pt.y + 0.5 * MapnikMap.HeightInt * res);
    
    // World
    //Envelope<double> box = Envelope<double>(-29785751.54497776,-19929239.11337915,
    //				               29723259.88776701,18379686.99645029);
    // Small area
    // Envelope<double> box = Envelope<double>(-16944.38844621579,6679978.34125,
    //                                         -16811.61155378421,6680061.65875);
    
    MapnikMap.map.zoomToBox(box);
    
    
    Image32 buf(MapnikMap.WidthInt,MapnikMap.HeightInt);
    mapnik::agg_renderer<Image32> ren(MapnikMap.map,buf);
    ren.apply();
    
    std::cout << MapnikMap.map.getCurrentExtent() << "\n";
    
    MapnikMap.ImageRawDataPtr = (unsigned char *) buf.raw_data();
	
    // Not working yet
    // buf.saveToFile("test1.png", "PNG");

    
    QImage image((uchar*)buf.raw_data(),1280,1024,QImage::Format_ARGB32);
    image.save("test.png", "PNG");
    

}

extern "C"
unsigned char *get_mapnik_imagedata() {
	return MapnikMap.ImageRawDataPtr;
}

}
