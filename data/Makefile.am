# Makefile.am for data

cfg_filesdir   = $(datadir)/gpsdrive/
cfg_files_DATA = icons.txt map_koord.txt

pixmapdir   = $(datadir)/gpsdrive/pixmaps
pixmap_DATA = pixmaps/*.png pixmaps/*.gif

iconsdir   = $(datadir)/gpsdrive/icons
icons_DATA = icons/index.html

icons_square.big_DATA =	icons/square.big/*.png
icons_square.bigdir= $(datadir)/gpsdrive/icons/square.big

icons_square.big_food_DATA = icons/square.big/food/*.png
icons_square.big_fooddir= $(datadir)/gpsdrive/icons/square.big/food

icons_square.big_restaurant_DATA = icons/square.big/food/restaurant/*.png
icons_square.big_restaurantdir= $(datadir)/gpsdrive/icons/square.big/restaurant

icons_square.big_geocache_DATA = icons/square.big/geocache/*.png
icons_square.big_geocachedir= $(datadir)/gpsdrive/icons/square.big

icons_square.big_people_DATA = icons/square.big/people/*.png
icons_square.big_peopledir= $(datadir)/gpsdrive/icons/square.big/people

icons_square.big_sightseeing_DATA = icons/square.big/sightseeing/*.png 
icons_square.big_sightseeingdir= $(datadir)/gpsdrive/icons/square.big/sightseeing


icons_square.small_DATA = icons/square.small/*.png
icons_square.smalldir= $(datadir)/gpsdrive/icons/square.small

icons_square.small_accomodation_DATA = icons/square.small/accomodation/*.png
icons_square.small_accomodationdir= $(datadir)/gpsdrive/icons/square.small/accomodation

icons_square.small_people_DATA = icons/square.small/people/*.png
icons_square.small_peopledir= $(datadir)/gpsdrive/icons/square.small/people


icons_classic_DATA = icons/classic/*.png
icons_classicdir= $(datadir)/gpsdrive/icons/classic

icons_classic_accomodation_DATA = icons/classic/accomodation/*.png
icons_classic_accomodationdir= $(datadir)/gpsdrive/icons/classic/accomodation

icons_classic_food_DATA = icons/classic/food/*.png
icons_classic_fooddir= $(datadir)/gpsdrive/icons/classic

icons_classic_health_DATA = icons/classic/health/*.png
icons_classic_healthdir= $(datadir)/gpsdrive/icons/classic/health

icons_classic_money_DATA = icons/classic/money/*.png
icons_classic_moneydir= $(datadir)/gpsdrive/icons/classic/money

icons_classic_nautical_DATA = icons/classic/nautical/*.png
icons_classic_nauticaldir= $(datadir)/gpsdrive/icons/classic/nautical

icons_classic_other_DATA = icons/classic/other/*.png
icons_classic_otherdir= $(datadir)/gpsdrive/icons/classic/other

icons_classic_people_DATA = icons/classic/people/*.png
icons_classic_peopledir= $(datadir)/gpsdrive/icons/classic/people

icons_classic_places_DATA = icons/classic/places/*.png
icons_classic_placesdir= $(datadir)/gpsdrive/icons/classic/places

icons_classic_recreation_DATA = icons/classic/recreation/*.png
icons_classic_recreationdir= $(datadir)/gpsdrive/icons/classic/recreation

icons_classic_religion_DATA = icons/classic/religion/*.png
icons_classic_religiondir= $(datadir)/gpsdrive/icons/classic/religion

icons_classic_shopping_DATA = icons/classic/shopping/*.png
icons_classic_shoppingdir= $(datadir)/gpsdrive/icons/classic/shopping

icons_classic_sports_DATA = icons/classic/sports/*.png
icons_classic_sportsdir= $(datadir)/gpsdrive/icons/classic/sports

icons_classic_transport_DATA = icons/classic/transport/*.png
icons_classic_transportdir= $(datadir)/gpsdrive/icons/classic/transport

icons_classic_vehicle_DATA = icons/classic/vehicle/*.png
icons_classic_vehicledir= $(datadir)/gpsdrive/icons/classic/vehicle

icons_classic_wlan_DATA = icons/classic/wlan/*.png 
icons_classic_wlandir= $(datadir)/gpsdrive/icons/classic/wlan

streetsdir   = $(datadir)/gpsdrive/streets
streets_DATA = streets/Autobahnen.txt \
	  streets/Streets_au.txt  streets/Streets.txt

poidir   = $(datadir)/gpsdrive/poi
poi_DATA = \
	poi/cities_germany.txt  poi/cities.txt  poi/germany.txt\
	poi/cities_au.txt poi/shops_au.txt


mapsdir   = $(datadir)/gpsdrive/maps
maps_DATA = \
           maps/*.png \
           maps/*.jpg


appsdir = $(datadir)/applications
apps_DATA = gpsdrive.desktop

EXTRA_DIST=        \
        $(cfg_files_DATA) \
        $(pixmap_DATA) $(icons_DATA) \
	$(icons_square.big_DATA) \
	$(icons_square.big_food_DATA) \
	$(icons_square.big_restaurant_DATA) \
	$(icons_square.big_geocache_DATA) \
	$(icons_square.big_people_DATA) \
	$(icons_square.big_sightseeing_DATA) \
	$(icons_square.small_DATA) \
	$(icons_square.small_accomodation_DATA) \
	$(icons_square.small_people_DATA) \
	$(icons_classic_DATA) \
	$(icons_classic_accomodation_DATA) \
	$(icons_classic_food_DATA) \
	$(icons_classic_health_DATA) \
	$(icons_classic_money_DATA) \
	$(icons_classic_nautical_DATA) \
	$(icons_classic_other_DATA) \
	$(icons_classic_people_DATA) \
	$(icons_classic_places_DATA) \
	$(icons_classic_recreation_DATA) \
	$(icons_classic_religion_DATA) \
	$(icons_classic_shopping_DATA) \
	$(icons_classic_sports_DATA) \
	$(icons_classic_transport_DATA) \
	$(icons_classic_vehicle_DATA) \
	$(icons_classic_wlan_DATA) \
        $(maps_DATA) $(poi_DATA) $(streets_DATA) \
        $(apps_DATA)

rebuild-icons:
	sh ./rebuild-icons.txt.sh
