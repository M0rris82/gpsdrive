/*******************************************************************************

Copyright (c) 2001-2005 Fritz Ganter <ganter@ganter.at>

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

*******************************************************************************/

/*
$Log$
Revision 1.1  2005/04/29 17:41:57  tweety
Moved the speech string to a seperate File



*******************************************************************************/

// ************************************************************************** //
// This file is used to contain all the spoken phrases in the given languages.
// **** NOTE **** Remember to include punctuation to make the phrases proper
// sentences, festival will adjust the sound to make it more natural if you do.
//
// **** TODO ****
//  Perhaps move these to the international strings system.

// ************************************************************************** //
// This enumeration is used to define all the currently supported languages.
// Add new languages to the end, you *MUST* then add new strings to *ALL*
// of the following arrays.

#include <gtk/gtk.h>

// ************************************************************************** //
// The actual phrases used to inform the user. NOTE the order of the items
// within each array is VERY important and must match the above enumeration.

gchar* speech_target_reached[] =
{
  "You reached the target %s.",
  "Sie haben das Ziel %s erreicht.",
  "usted ha llegado a %s."
};

gchar* speech_new_target[] =
{
  "New target is %s.",
  "Neues Ziel ist %s.",
  "DestinaciÃ³n definida: %s."
};


gchar* speech_danger_radar[] =
{
  "Danger, Danger, Radar in %d meters, Your speed is %d.",
  "Achtung, Achtung, Radar in %d metern, Ihre Geschwindigkeit ist %d.",
  "AtenciÃ³n AtenciÃ³n, control de  velocidad en %d metros, Su velocidad es %d."
};

// **** TODO ****
// German string needs corrected
gchar* speech_info_radar[] =
{
  "Information, Radar in %d meters.",
  "Information, Radarfalle in %d metern.",
  "InformaciÃ³n, control de velocidad en %d metros."
};

gchar* speech_arrival_hours_mins[] =
{
  "Arrival in approximatly %d hours and %d minutes.",
  "Ankunft in circa %d Stunden und %d minuten.",
  "Llegada en %d horas y %d minutos."
};

gchar* speech_arrival_mins[] =
{
  "Arrival in approximatly %d minutes.",
  "Ankunft in zirca %d minuten.",
  "Llegada en %d minutos."
};

// **** TODO ****
// Spanish string needs corrected
gchar* speech_arrival_one_hour_mins[] =
{
  "Arrival in approximatly one hour and %d minutes.",
  "Ankunft in circa einer Stunde und %d minuten.",
  "Arrival in approximatly one hour and %d minutes."
};

// **** TODO ****
// Spanish string needs corrected
gchar* speech_diff_gps_found[] =
{
  "Differential GPS signal found.",
  "Ein differenzielles GPS Signal wurde gefunden.",
  "Differential GPS signal found."
};

// **** TODO ****
// Spanish string needs corrected
gchar* speech_diff_gps_lost[] =
{
  "No differential GPS signal detected.",
  "Kein differenzielles GPS Signal vorhanden.",
  "No differential GPS signal detected."
};

// **** TODO ****
// Spanish string needs corrected
gchar* speech_gps_lost[] =
{
  "No GPS signal dectected.",
  "Kein ausreichendes GPS Signal vorhanden.",
  "No GPS signal dectected.",
};

gchar* speech_gps_good[] =
{
  "GPS signal good.",
  "Gutes GPS Signal vorhanden.",
  "GPS signal bueno."
};

// **** TODO ****
// Spanish string needs corrected
gchar* speech_kismet_found[] =
{
  "Found kismet. Happy wardriving.",
  "Kismet gefunden. Viel Spass beim wordreifing.",
  "Found kismet. Happy wardriving."
};

// **** TODO ****
// Spanish string needs corrected
gchar* speech_message_received[] =
{
  "You received a message from %s.",
  "Sie haben eine Nachricht von %s erhalten.",
  "You received a message from %s."
};

gchar* speech_morning[] =
{
  "Good Morning,",
  "Guten Morgen.",
  "Buenos dÃ­as."
};

gchar* speech_afternoon[] =
{
  "Good afternoon,",
  "Guten Tag.",
  "Buenos tardes."
};

gchar* speech_evening[] =
{
  "Good evening,",
  "Guten Abend.",
  "Buenas noches."
};

gchar* speech_time_mins[] =
{
  "It is one %d.",
  "Es ist ein Uhr %d.",
  "Es la una y %d minutos."
};

gchar* speech_time_hrs_mins[] =
{
  "It is %d %d.",
  "Es ist %d Uhr %d.",
  "Son las %d horas y %d minutos."
};

gchar* speech_too_few_satellites[] =
{
  "Not enough satellites in view.",
  "Zuwenig Satelliten in Sicht.",
  "El GPS Fix no estÃ¡ disponible."
};

// This is part of a sentence, so punctuation is given elsewhere.
gchar* speech_destination_is[] =
{
  "Destination is %s",
  "Das Ziel ist %s",
  "Su destinaciÃ³n estÃ¡ %s"
};

gchar* speech_front[] =
{
  "in front of you.",
  "vor ihnen.",
  "delante de usted."
};

gchar* speech_front_right[] =
{
  "ahead of you to the right.",
  "rechts vor ihnen.",
  "delante de usted a la derecha."
};

gchar* speech_right[] =
{
  "to your right.",
  "rechts.",
  "a la derecha."
};

gchar* speech_behind_right[] =
{
  "behind you to the right.",
  "rechts hinter ihnen.",
  "de tras de usted a la derecha."
};

gchar* speech_behind[] =
{
  "behind you.",
  "hinter ihnen.",
  "de tras de usted."
};

gchar* speech_behind_left[] =
{
  "behind you to the left.",
  "links hinter ihnen.",
  "de tras de usted a la izquierda."
};

gchar* speech_left[] =
{
  "to your left.",
  "links.",
  "a la izquierda."
};

gchar* speech_front_left[] =
{
  "ahead of you to the left.",
  "links vor ihnen.",
  "delante de usted a la izquierda."
};

gchar* speech_speed_mph[] =
{
  "The current speed is %d miles per hour.",
  "Die momentane Geschwindigkeit ist %d Meilen pro Stunde.",
  "La velocidad actual es %d milla por hora."
};

gchar* speech_speed_kph[] =
{
  "The current speed is %d kilometers per hour.",
  "Die momentane Geschwindigkeit ist %d kmh.",
  "La velocidad actual es %d kilometros por hora."
};

// This is part of a sentence, so punctuation is given elsewhere.
gchar* speech_distance_to[] =
{
  "Distance to %s is %s",
  "Die Entfernung bis %s ist %s",
  "La distancia a la %s es %s"
};

gchar* speech_yards[] =
{
  "%.0f yards.",
  "%.0f yard.",
  "%.0f yards."
};

gchar* speech_miles[] =
{
  "%.0f miles.",
  "%.0f Meilen.",
  "%.0f millas."
};

gchar* speech_meters[] =
{
  "%.0f meters.",
  "%.0f meter.",
  "%.0f metros."
};

gchar* speech_kilometers[] =
{
  "%.0f kilometers.",
  "%d kilometer.",
  "%.0f kilometros."
};

// **** TODO ****
// Spanish string needs corrected
gchar* speech_one_kilometer[] =
{
  "one kilometer.",
  "ein kilometer.",
  "one kilometer."
};

// ************************************************************************** //
