/***********************************************************************

Copyright (c) 2001,2002 Fritz Ganter <ganter@ganter.at>

Website: www.kraftvoll.at/software

Disclaimer: Please do not use for navigation.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*********************************************************************/
/*
$Log$
Revision 1.3  2005/02/06 21:18:05  tweety
more cleanup: extracted more functionality to functions

Revision 1.2  2005/02/06 17:52:44  tweety
extract icon handling to icons.c

Revision 1.0  2005/02/02 18:11:02  tweety
*/

int drawicon (gint posxdest, gint posydest, char *ic);
void load_friends_icon (void);
void load_icons(void);

typedef struct
{
    GdkPixbuf *icon;
    char name[40];
}
auxiconsstruct;

//extern gint maxauxicons, lastauxicon;
//extern auxiconsstruct *auxicons;
//extern GdkPixbuf *friendsimage, *friendspixbuf;
