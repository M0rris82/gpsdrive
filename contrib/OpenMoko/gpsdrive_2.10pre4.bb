inherit autotools pkgconfig

PR = "r1"
DEFAULT_PREFERENCE="1"

PACKAGES += "gpsdrive-maps gpsdrive-osm"
DESCRIPTION = "GPS navigation/map display software"
DEPENDS = "virtual/libc gtk+ libpcre gpsd libwww-perl-native mysql"
RDEPENDS_${PN} = "gdk-pixbuf-loader-gif gpsd libwww-perl"
SECTION = "x11"
PRIORITY = "optional"
LICENSE = "GPL"

SRC_URI = "http://www.gpsdrive.de/packages/${PN}-${PV}.tar.gz \
           file://gpsdrive.desktop \
           file://gpsdrive-remove-hardcoded-paths.patch;patch=1 \
           file://gpsdrive-ifdef-mapnik.patch;patch=1 \
           file://gpsdrive-fix-desktop-file.patch;patch=1"

CFLAGS += "-D_GNU_SOURCE"

FILES_${PN} = "${bindir} ${datadir}/applications ${datadir}/map-icons"
FILES_${PN} += "${datadir}/${PN}/pixmaps ${datadir}/${PN}/poi ${datadir}/${PN}/gpsdrive.desktop ${datadir}/${PN}/map_koord.txt"

FILES_${PN}-maps = "${datadir}/${PN}/maps"

FILES_${PN}-osm = "${libdir}/perl"

FILES_${PN}-doc = "${datadir}/man ${datadir}/${PN}/Documentation"

EXTRA_OECONF = "--disable-garmin"

do_install_append () {
        mkdir -p  ${D}${datadir}/applications
        install -m 0644 ${WORKDIR}/gpsdrive.desktop ${D}${datadir}/applications/gpsdrive.desktop
}
