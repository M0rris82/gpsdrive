/*
  #include <QWidget>
  #include <QImage>
  #include <QPixmap>
  #include <QPen>
*/
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
    
extern "C" void
init_mapnik ( ) {
    // register datasources (plug-ins) and a font
    // Both datasorce_cache and font_engine are 'singletons'.
    
    using namespace mapnik;
    datasource_cache::instance()->register_datasources("/usr/lib/mapnik/input/");


    freetype_engine::instance()->register_font("/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf");
    
}


extern "C" void
render_mapnik ( ) {
    Map map(1280,1024); 

    // This location has to be adapted in the future
    // for now it should work if gpsdrive is installed in the standard location
    string mapnik_config_file("./scripts/mapnik/osm.xml");
    if ( ! boost:: filesystem::exists(mapnik_config_file) ) 
	mapnik_config_file.assign("../scripts/mapnik/osm.xml");
    if ( ! boost:: filesystem::exists(mapnik_config_file) ) 
	mapnik_config_file.assign(DATADIR).append("/mapnik/osm.xml");

    cout << "Using Mapnik config-file: " << mapnik_config_file << endl;
    mapnik::load_map(map,mapnik_config_file);

    // World
    Envelope<double> box = Envelope<double>(-29785751.54497776,-19929239.11337915,
					    29723259.88776701,18379686.99645029);
    // Small area
    // Envelope<double> box = Envelope<double>(-16944.38844621579,6679978.34125,
    //                                         -16811.61155378421,6680061.65875);
    
    map.zoomToBox(box);
    
    
    Image32 buf(1280,1024);
    mapnik::agg_renderer<Image32> ren(map,buf);
    ren.apply();
	
    buf.saveToFile("/home/tweety/gpsdrive/gpsdrive-mapnik/test1.png", "PNG");

    //QImage image((uchar*)buf.raw_data(),1280,1024,QImage::Format_ARGB32);
    //image.save("/home/tweety/gpsdrive/gpsdrive-mapnik/test2.png", "PNG");

}

}
