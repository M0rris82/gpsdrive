# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/app-misc/gpsdrive/gpsdrive-2.10.ebuild,v 1.6 2005/01/01 15:05:04 eradicator Exp $

inherit flag-o-matic subversion

DESCRIPTION="displays GPS position on a map"
HOMEPAGE="http://www.gpsdrive.de"
#SRC_URI="http://www.gpsdrive.de/gpsdrive.tar/${P/_/}.tar.gz"

ESVN_REPO_URI="http://svn.gpsdrive.cc/gpsdrive/trunk"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86 ppc"

IUSE="nls mysql garmin"
DEPEND="sys-devel/gettext
	>=x11-libs/gtk+-2.0
	mysql? ( dev-db/mysql )
	>=dev-libs/libpcre-4.2
        sci-libs/gdal"

S="${WORKDIR}/${P/_/}"

src_compile() {
	local myconf
    
	use nls \
	    && myconf="${myconf} --enable-nls" \
	    || myconf="${myconf} --disable-nls"
    
	use garmin \
	    && myconf="${myconf} --enable-garmin" \
	    || myconf="${myconf} --disable-garmin"

	econf ${myconf} || die "econf failed"
	emake || die "compile failed"
}

src_install() {
	make DESTDIR=${D} install || die
}
