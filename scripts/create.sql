--- gpsdrive 1.29

CREATE DATABASE /*!32312 IF NOT EXISTS*/ geoinfo;

USE geoinfo;

--
-- Table structure for table 'waypoints'
--
CREATE TABLE waypoints (
  name char(40) default '',
  type char(40) default '',
  id int(11) NOT NULL auto_increment,
  lat double default '0',
  lon double default '0',
  comment char(160) default '',
  wep int(11) NOT NULL default '0',
  macaddr char(20) default '0',
  nettype int(11) NOT NULL default '0',
  typenr int(11) default NULL,
  PRIMARY KEY  (id),
  KEY macindex (macaddr),
  KEY descrid (type),
  KEY namekey (name),
  KEY typenr (typenr)
) TYPE=MyISAM;


grant select,insert,update,delete on geoinfo.* to gast@localhost identified by 'gast';
flush privileges;
INSERT INTO waypoints VALUES ('Lisa','Fritz',59,53.5626,9.9574,'',0,'0',0,NULL);
quit

