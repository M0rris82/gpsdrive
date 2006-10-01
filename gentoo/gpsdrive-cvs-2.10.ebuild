# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header$

DESCRIPTION="displays GPS position on a map"
HOMEPAGE="http://www.ostertag.name/gpsdrive/"
#SRC_URI="http://www.ostertag.name/gpsdrive/tar/${P}.tar.bz2"
ECVS_SERVER="cvs.gpsdrive.cc:/cvsroot"
ECVS_MODULE="gpsdrive"
ECVS_TOP_DIR="${DISTDIR}/cvs-src/${PN}"
S=${WORKDIR}/${ECVS_MODULE}

inherit cvs

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86 ppc amd64"

IUSE="nls"
DEPEND="sys-devel/gettext
	>=x11-libs/gtk+-2.0
	>=dev-libs/libpcre-4.2
	sci-libs/gdal
	dev-db/mysql"


src_compile() {
	econf `use_enable nls` || die "econf failed"
	emake || die "compile failed"
}

src_install() {
	make DESTDIR=${D} install || die
}
