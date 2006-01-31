-- MySQL dump 10.10
--
-- Host: localhost    Database: geoinfo
-- ------------------------------------------------------
-- Server version	5.0.16-Debian_1-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `address`
--

CREATE TABLE `address` (
  `address_id` int(11) NOT NULL auto_increment,
  `country` varchar(40) NOT NULL default '',
  `state` varchar(80) NOT NULL default '',
  `zip` varchar(5) NOT NULL default '',
  `city` varchar(80) NOT NULL default '',
  `city_part` varchar(80) NOT NULL default '',
  `streets_name` varchar(80) NOT NULL default '',
  `streets_number` varchar(5) NOT NULL default '',
  `phone` varchar(160) NOT NULL default '',
  `comment` varchar(160) NOT NULL default '',
  PRIMARY KEY  (`address_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `poi`
--

CREATE TABLE `poi` (
  `poi_id` int(11) NOT NULL auto_increment,
  `name` varchar(80) default NULL,
  `poi_type_id` int(11) NOT NULL default '0',
  `lat` double default '0',
  `lon` double default '0',
  `alt` double default '0',
  `proximity` float default '0',
  `comment` varchar(255) default NULL,
  `scale_min` int(12) NOT NULL default '0',
  `scale_max` int(12) NOT NULL default '0',
  `last_modified` date NOT NULL default '0000-00-00',
  `url` varchar(160) default NULL,
  `address_id` int(11) default '0',
  `source_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`poi_id`),
  KEY `last_modified` (`last_modified`),
  KEY `name` (`name`),
  KEY `lat` (`lat`),
  KEY `lon` (`lon`),
  KEY `combi1` (`lat`,`lon`,`scale_min`,`scale_max`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `poi_type`
--

CREATE TABLE `poi_type` (
  `poi_type_id` int(11) NOT NULL auto_increment,
  `name` varchar(80) NOT NULL default '',
  `name_de` varchar(80) default 'en',
  `symbol` varchar(160) default '',
  `description` varchar(160) default '',
  `description_de` varchar(160) default '',
  PRIMARY KEY  (`poi_type_id`),
  KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `source`
--

CREATE TABLE `source` (
  `source_id` int(11) NOT NULL auto_increment,
  `name` varchar(80) NOT NULL default '',
  `comment` varchar(160) NOT NULL default '',
  `last_update` date NOT NULL default '0000-00-00',
  `url` varchar(160) NOT NULL default '',
  `licence` varchar(160) NOT NULL default '',
  PRIMARY KEY  (`source_id`),
  KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `streets`
--

CREATE TABLE `streets` (
  `streets_id` int(11) NOT NULL auto_increment,
  `name` varchar(80) default NULL,
  `streets_type_id` int(11) NOT NULL default '0',
  `lat1` double default '0',
  `lon1` double default '0',
  `alt1` double default '0',
  `lat2` double default '0',
  `lon2` double default '0',
  `alt2` double default '0',
  `proximity` float default '0',
  `comment` varchar(255) default NULL,
  `scale_min` int(12) NOT NULL default '0',
  `scale_max` int(12) NOT NULL default '0',
  `last_modified` date NOT NULL default '0000-00-00',
  `source_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`streets_id`),
  KEY `last_modified` (`last_modified`),
  KEY `name` (`name`),
  KEY `lat1` (`lat1`),
  KEY `lon1` (`lon1`),
  KEY `lat2` (`lat2`),
  KEY `lon2` (`lon2`),
  KEY `combi1` (`lat1`,`lon1`,`lat2`,`lon2`,`scale_min`,`scale_max`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `streets_type`
--

CREATE TABLE `streets_type` (
  `streets_type_id` int(11) NOT NULL auto_increment,
  `name` varchar(80) NOT NULL default '',
  `name_de` varchar(80) default 'en',
  `description` varchar(160) default '',
  `description_de` varchar(160) default '',
  `color` varchar(80) default '',
  `linetype` varchar(80) default '',
  PRIMARY KEY  (`streets_type_id`),
  KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `tracks`
--

CREATE TABLE `tracks` (
  `track_id` int(11) NOT NULL auto_increment,
  `name` varchar(80) default NULL,
  `track_type_id` int(11) NOT NULL default '0',
  `lat1` double default '0',
  `lon1` double default '0',
  `alt1` double default '0',
  `time1` date default '0000-00-00',
  `lat2` double default '0',
  `lon2` double default '0',
  `alt2` double default '0',
  `time2` date default '0000-00-00',
  `speed` double default '0',
  `direction` double default '0',
  `acuracy` float default '0',
  `time_delta` float default '0',
  `source_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`track_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `tracks_type`
--

CREATE TABLE `tracks_type` (
  `track_type_id` int(11) NOT NULL auto_increment,
  `name` varchar(80) NOT NULL default '',
  `name_de` varchar(80) default 'en',
  `symbol` varchar(160) default '',
  `description` varchar(160) default '',
  `description_de` varchar(160) default '',
  `color` varchar(80) default '',
  `linetype` varchar(80) default '',
  PRIMARY KEY  (`track_type_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `traffic`
--

CREATE TABLE `traffic` (
  `id` int(11) NOT NULL auto_increment,
  `status` int(11) default NULL,
  `street` varchar(40) default NULL,
  `descshort` varchar(100) default NULL,
  `desclong` text NOT NULL,
  `future` int(11) NOT NULL default '0',
  `time` time default '00:00:00',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `waypoints`
--

CREATE TABLE `waypoints` (
  `name` char(40) default '',
  `type` char(40) default '',
  `id` int(11) NOT NULL auto_increment,
  `lat` double default '0',
  `lon` double default '0',
  `comment` char(160) default '',
  `wep` int(11) NOT NULL default '0',
  `macaddr` char(20) default '0',
  `nettype` int(11) NOT NULL default '0',
  `typenr` int(11) default NULL,
  PRIMARY KEY  (`id`),
  KEY `macaddr` (`macaddr`),
  KEY `type` (`type`),
  KEY `name` (`name`),
  KEY `typenr` (`typenr`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;


grant select,insert,update,delete on geoinfo.* to gast@localhost identified by 'gast';
flush privileges;
INSERT INTO waypoints VALUES ('Lisa','Fritz',59,53.5626,9.9574,'',0,'0',0,NULL);
quit
