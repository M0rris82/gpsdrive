#!/usr/bin/python
"""GpsDrive Generate Mapnik Tiles

Generates 1280x1024 Mapniktiles for GpsDrive

Usage: python gpsdrive_mapnik_gentiles.py [options]

Options:
  -h, --help     show this help
  -b, --bbox     boundingbox (lon,lat,lon,lat) - Be carefull! Quote negative values!
  -s, --scale    scale single/range
                              1 = 147456000
                              2 = 73728000
                             ...
                             15 = 9000
                             16 = 4500
                             17 = 2250
  --test         testrun = generates Munich example
                             
Examples:
  
  Munich:
    gpsdrive_mapnik_gentiles.py -b  11.4,48.07,11.7,48.2 -s 10-16
  
  World:
    gpsdrive_mapnik_gentiles.py -b "-180.0,-90.0,180.0,90.0" -s 1-6
"""

from math import pi,cos,sin,log,exp,atan
from subprocess import call
import sys, os
import getopt
import string

zoom2scale = [728 * 576000,256 * 576000,128 * 576000,64 * 576000,32 * 576000,16 * 576000,8 * 576000,4 * 576000,2 * 576000,576000,288000,144000,72000,36000,18000,9000,4500,2250,1125]


DEG_TO_RAD = pi/180
RAD_TO_DEG = 180/pi

def minmax (a,b,c):
    a = max(a,b)
    a = min(a,c)
    return a

class GoogleProjection:
    def __init__(self,levels=18):
        self.Bc = []
        self.Cc = []
        self.zc = []
        self.Ac = []
        c = 256
        for d in range(0,levels):
            e = c/2;
            self.Bc.append(c/360.0)
            self.Cc.append(c/(2 * pi))
            self.zc.append((e,e))
            self.Ac.append(c)
            c *= 2
                
    def fromLLtoPixel(self,ll,zoom):
         d = self.zc[zoom]
         e = round(d[0] + ll[0] * self.Bc[zoom])
         f = minmax(sin(DEG_TO_RAD * ll[1]),-0.9999,0.9999)
         g = round(d[1] + 0.5*log((1+f)/(1-f))*-self.Cc[zoom])
         return (e,g)
     
    def fromPixelToLL(self,px,zoom):
         e = self.zc[zoom]
         f = (px[0] - e[0])/self.Bc[zoom]
         g = (px[1] - e[1])/-self.Cc[zoom]
         h = RAD_TO_DEG * ( 2 * atan(exp(g)) - 0.5 * pi)
         return (f,h)


import os
from PIL.Image import fromstring, new
from PIL.ImageDraw import Draw
from StringIO import StringIO
from mapnik import *

def render_tiles(bbox, mapfile, tile_dir, mapkoordfile, minZoom=1,maxZoom=18, name="unknown"):
    print "render_tiles(",bbox, mapfile, tile_dir, minZoom,maxZoom, name,")"

    fh_mapkoord = open(mapkoordfile, "a") 
    if fh_mapkoord == 0:
        sys.exit("Can not open map_koord.txt.")

    if not os.path.isdir(tile_dir):
         os.mkdir(tile_dir)

    gprj = GoogleProjection(maxZoom+1) 
    #m = Map(2 * 256,2 * 256)
    m = Map(1280,1024)
    load_map(m,mapfile)
    prj = Projection("+proj=merc +datum=WGS84")
    
    ll0 = (bbox[0],bbox[3])
    ll1 = (bbox[2],bbox[1])
    
    for z in range(minZoom,maxZoom + 1):
        px0 = gprj.fromLLtoPixel(ll0,z)
        px1 = gprj.fromLLtoPixel(ll1,z)
        for x in range(int(px0[0]/640.0),int(px1[0]/640.0)+1):
            for y in range(int(px0[1]/512.0),int(px1[1]/512.0)+1):
                p0 = gprj.fromPixelToLL((x * 640.0, (y+1) * 512.0),z)
                p1 = gprj.fromPixelToLL(((x+1) * 640.0, y * 512.0),z)

                # render a new tile and store it on filesystem
                c0 = prj.forward(Coord(p0[0],p0[1]))
                c1 = prj.forward(Coord(p1[0],p1[1]))
            
                bbox = Envelope(c0.x,c0.y,c1.x,c1.y)
                bbox.width(bbox.width() * 2)
                bbox.height(bbox.height() * 2)
                m.zoom_to_box(bbox)
                
                # check if we have directories in place
                zoom = "%s" % z
                str_x = "%s" % x
                str_y = "%s" % y

                if not os.path.isdir(tile_dir + zoom):
                    os.mkdir(tile_dir + zoom)
                if not os.path.isdir(tile_dir + zoom + '/' + str_x):
                    os.mkdir(tile_dir + zoom + '/' + str_x)

                tile_uri = tile_dir + zoom + '/' + str_x + '/' + str_y + '.png'
                tile_path = "mapnik/" + zoom + '/' + str_x + '/' + str_y + '.png'

		exists= ""
                if os.path.isfile(tile_uri):
                    exists= "exists"
                else:
                    im = Image(1280, 1024)
                    render(m, im)
                    im = fromstring('RGBA', (1280, 1024), im.tostring())
                    #im = im.crop((128,128,512-128,512-127))
                    fh = open(tile_uri,'w+b')
                    im.save(fh, 'PNG', quality=100)
                    command = "convert  -colors 255 %s %s" % (tile_uri,tile_uri)
                    call(command, shell=True)
   

                    fh_mapkoord.write(tile_path + " ")
                    fh_mapkoord.write(str((p0[1] + p1[1]) / 2) + " ")
                    fh_mapkoord.write(str((p0[0] + p1[0]) / 2) + " ")
                    fh_mapkoord.write(str(zoom2scale[z]))
                    fh_mapkoord.write(" " + str(p0[1]) + " " + str(p0[0]))
                    fh_mapkoord.write(" " + str(p1[1]) + " " + str(p1[0]))
                    fh_mapkoord.write("\n")


                bytes=os.stat(tile_uri)[6]
		empty= ''
                if bytes == 137:
                    empty = " Empty Tile "

                print name,"[",minZoom,"-",maxZoom,"]: " ,z,x,y,"p:",p0,p1,exists, empty
    fh_mapkoord.close()
    
def usage():
    print __doc__

def main(argv):

    home = os.environ['HOME']
    user = os.environ['USER']
    data = home + "/.gpsdrive"
    mapfile = data + "/osm.xml"
    tile_dir = home + "/.gpsdrive/maps/mapnik/"
    mapkoordfile = home + "/.gpsdrive/maps/map_koord.txt"

    if not os.path.isfile(mapfile):
	command = "sed 's,\@DATA_DIR\@,@DATA_DIR@/mapnik,g;s,\@USER\@," + user + ",g;' <@DATA_DIR@/mapnik/osm.xml >" + mapfile
	print "Creating " + mapfile
	print "Command " + command
	call(command, shell=True)
    
    
    minZoom = 0
    maxZoom = 0
    bboxset = 0
    
    try:
        opts, args = getopt.getopt(argv, "hb:s:", ["help", "bbox=", "scale=", "test"])
    except getopt.GetoptError:
        sys.exit("Invalid option!")
    
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()
        elif opt in ("-b", "--bbox"):
            bboxset = 1
            bboxs = string.split(arg, ',')
        elif opt in ("-s" , "--scale"):
            zooms = string.split(arg, '-')
            if len(zooms) == 2:
                minZoom = eval(zooms[0])
                if str(minZoom) <> zooms[0]: minZoom = 0
                maxZoom = eval(zooms[1])
                if str(maxZoom) <> zooms[1]: maxZoom = 0
            elif len(zooms) == 1:
                minZoom = eval(zooms[0])
                if str(minZoom) <> zooms[0]: minZoom = 0
                maxZoom = minZoom
                
        elif opt in ("--test"):
            bbox = (11.4,48.07,11.7,48.2)
            minZoom = 10
            maxZoom = 16
            render_tiles(bbox, mapfile, tile_dir, minZoom, maxZoom, "Test")
            sys.exit()
            
    if bboxset == 0:
       sys.exit("No boundingbox set!")
            
    if len(bboxs) < 4:
        sys.exit("Boundingbox invalid!")
    
    # check for correct values
    if str(eval(bboxs[0])) != bboxs[0] or str(eval(bboxs[1])) != bboxs[1] or str(eval(bboxs[2])) != bboxs[2] or str(eval(bboxs[3])) != bboxs[3]:
        sys.exit("Boundingbox invalid!")
        
    if minZoom < 1 or minZoom > 17 or maxZoom < 1 and maxZoom > 17 or minZoom > maxZoom or int(minZoom) <> minZoom or int(maxZoom) <> maxZoom:
        sys.exit("Invalid scale!")
        
    #ok transform bboxs to a bbox with float
    bbox = (eval(bboxs[0]), eval(bboxs[1]), eval(bboxs[2]), eval(bboxs[3]))
    
    #start rendering
    render_tiles(bbox, mapfile, tile_dir, mapkoordfile, minZoom, maxZoom, "Generate:")
    
    #return info
    print "\n", "Finished.\n"


if __name__ == "__main__":
    main(sys.argv[1:])
