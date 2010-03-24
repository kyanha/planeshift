-- MySQL Administrator dump 1.4
--
-- ------------------------------------------------------
-- Server version	5.0.37-community-nt


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

--
-- Definition of table `sectors`
--

DROP TABLE IF EXISTS `sectors`;
CREATE TABLE `sectors` (
  `id` smallint(3) unsigned NOT NULL auto_increment,
  `name` varchar(30) NOT NULL default '',
  `rain_enabled` char(1) NOT NULL default 'N',
  `rain_min_gap` int(10) unsigned NOT NULL default '0',
  `rain_max_gap` int(10) unsigned NOT NULL default '0',
  `rain_min_duration` int(10) unsigned NOT NULL default '0',
  `rain_max_duration` int(10) unsigned NOT NULL default '0',
  `rain_min_drops` int(10) unsigned NOT NULL default '0',
  `rain_max_drops` int(10) unsigned default '0',
  `rain_min_fade_in` int(10) unsigned NOT NULL default '0',
  `rain_max_fade_in` int(10) unsigned NOT NULL default '0',
  `rain_min_fade_out` int(10) unsigned NOT NULL default '0',
  `rain_max_fade_out` int(10) unsigned NOT NULL default '0',
  `lightning_min_gap` int(10) unsigned default '0',
  `lightning_max_gap` int(10) unsigned NOT NULL default '0',
  `collide_objects` tinyint(1) NOT NULL default '0',
  `non_transient_objects` tinyint(1) NOT NULL default '0',
  `say_range` float(5,2) unsigned NOT NULL default '0.0' COMMENT 'Determines the range of say in the specific sector. Set 0.0 to use default.',
  `TeleportingSector` varchar(30) NOT NULL COMMENT 'Sector where the player will be teleported automatically when entering this sector',
  `TeleportingCords` varchar(30) NOT NULL COMMENT 'Cordinates where the player will be teleported automatically when entering this sector',
  `DeathSector` varchar(30) NOT NULL COMMENT 'Sector where the player will be teleported automatically when dieing in this sector',
  `DeathCords` varchar(30) NOT NULL COMMENT 'Cordinates where the player will be teleported automatically when dieing in this sector',
  `TeleportingSectorEnable` char(1) NOT NULL DEFAULT 'N' COMMENT 'When not N the sector will teleport when accessing it according to teleportingsector and teleportingcords',
  `TeleportingPenaltyEnable` char(1) NOT NULL DEFAULT 'N' COMMENT 'When not N when teleported when entering this sector it will apply the death penalty',
  `god_name` VARCHAR(45) NOT NULL DEFAULT 'Laanx',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=latin1;


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
