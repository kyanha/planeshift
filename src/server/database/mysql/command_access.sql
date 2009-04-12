# HeidiSQL Dump 
#
# --------------------------------------------------------
# Host:                 127.0.0.1
# Database:             planeshift
# Server version:       5.0.67-community-nt
# Server OS:            Win32
# Target-Compatibility: MySQL 5.0
# max_allowed_packet:   1048576
# HeidiSQL version:     3.2 Revision: 1129
# --------------------------------------------------------

/*!40100 SET CHARACTER SET latin1*/;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0*/;


DROP TABLE IF EXISTS `command_group_assignment`;

#
# Table structure for table 'command_group_assignment'
#

CREATE TABLE `command_group_assignment` (
  `command_name` varchar(40) NOT NULL default '',
  `group_member` int(8) NOT NULL,
  UNIQUE KEY `UNIQUE1` (`command_name`,`group_member`)
) ENGINE=InnoDB /*!40100 DEFAULT CHARSET=utf8*/;



#
# Dumping data for table 'command_group_assignment'
#

LOCK TABLES `command_group_assignment` WRITE;
/*!40000 ALTER TABLE `command_group_assignment` DISABLE KEYS*/;
INSERT INTO `command_group_assignment` (`command_name`, `group_member`) VALUES
	('/action',30),
	('/assignfaction',24),
	('/assignfaction',25),
	('/assignfaction',30),
	('/awardexp',24),
	('/awardexp',25),
	('/awardexp',30),
	('/badtext',30),
	('/ban',24),
	('/ban',25),
	('/ban',30),
	('/banadvisor',22),
	('/banadvisor',23),
	('/banadvisor',24),
	('/banadvisor',25),
	('/banadvisor',30),
	('/banname',22),
	('/banname',23),
	('/banname',24),
	('/banname',25),
	('/banname',30),
	('/changeguildleader',22),
	('/changeguildleader',23),
	('/changeguildleader',24),
	('/changeguildleader',25),
	('/changeguildleader',30),
	('/changeguildname',22),
	('/changeguildname',23),
	('/changeguildname',24),
	('/changeguildname',25),
	('/changeguildname',30),
	('/changename',22),
	('/changename',23),
	('/changename',24),
	('/changename',25),
	('/changename',30),
	('/charlist',21),
	('/charlist',22),
	('/charlist',23),
	('/charlist',24),
	('/charlist',25),
	('/charlist',30),
	('/checkitem',22),
	('/checkitem',23),
	('/checkitem',24),
	('/checkitem',25),
	('/checkitem',26),
	('/checkitem',30),
	('/crystal',25),
	('/crystal',30),
	('/death',25),
	('/death',30),
	('/deletechar',30),
	('/deputize',24),
	('/deputize',25),
	('/deputize',30),
	('/disablequest',25),
	('/disablequest',30),
	('/divorce',22),
	('/divorce',23),
	('/divorce',24),
	('/divorce',25),
	('/divorce',30),
	('/event',24),
	('/event',25),
	('/event',30),
	('/fog',24),
	('/fog',25),
	('/fog',30),
	('/freeze',22),
	('/freeze',23),
	('/freeze',24),
	('/freeze',25),
	('/freeze',30),
	('/giveitem',23),
	('/giveitem',24),
	('/giveitem',25),
	('/giveitem',30),
	('/impersonate',23),
	('/impersonate',24),
	('/impersonate',25),
	('/impersonate',30),
	('/info',21),
	('/info',22),
	('/info',23),
	('/info',24),
	('/info',25),
	('/info',30),
	('/inspect',22),
	('/inspect',23),
	('/inspect',24),
	('/inspect',25),
	('/inspect',30),
	('/item',24),
	('/item',25),
	('/item',30),
	('/key',25),
	('/key',30),
	('/kick',22),
	('/kick',23),
	('/kick',24),
	('/kick',25),
	('/kick',30),
	('/killnpc',22),
	('/killnpc',23),
	('/killnpc',24),
	('/killnpc',25),
	('/killnpc',30),
	('/listwarnings',21),
	('/listwarnings',22),
	('/listwarnings',23),
	('/listwarnings',24),
	('/listwarnings',25),
	('/listwarnings',30),
	('/loadquest',30),
	('/location',30),
	('/marriageinfo',22),
	('/marriageinfo',23),
	('/marriageinfo',24),
	('/marriageinfo',25),
	('/marriageinfo',30),
	('/modify',25),
	('/modify',30),
	('/money',24),
	('/money',25),
	('/money',30),
	('/morph',23),
	('/morph',24),
	('/morph',25),
	('/morph',30),
	('/mute',21),
	('/mute',22),
	('/mute',23),
	('/mute',24),
	('/mute',25),
	('/mute',30),
	('/npc',30),
	('/path',30),
	('/petition',0),
	('/petition',10),
	('/petition',21),
	('/petition',22),
	('/petition',23),
	('/petition',24),
	('/petition',25),
	('/petition',30),
	('/quest',21),
	('/quest',22),
	('/quest',23),
	('/quest',24),
	('/quest',25),
	('/quest',30),
	('/rain',24),
	('/rain',25),
	('/rain',30),
	('/reload',30),
	('/rndmsgtest',30),
	('/runscript',30),
	('/set',10),
	('/set',21),
	('/set',22),
	('/set',23),
	('/set',24),
	('/set',25),
	('/set',30),
	('/setitemname',24),
	('/setitemname',25),
	('/setitemname',30),
	('/setkillexp',23),
	('/setkillexp',24),
	('/setkillexp',25),
	('/setkillexp',30),
	('/setlabelcolor',22),
	('/setlabelcolor',23),
	('/setlabelcolor',24),
	('/setlabelcolor',25),
	('/setlabelcolor',30),
	('/setquality',24),
	('/setquality',25),
	('/setquality',30),
	('/setskill',22),
	('/setskill',23),
	('/setskill',24),
	('/setskill',25),
	('/setskill',30),
	('/setstackable',24),
	('/setstackable',25),
	('/setstackable',30),
	('/settrait',23),
	('/settrait',24),
	('/settrait',25),
	('/settrait',30),
	('/show_gm',21),
	('/show_gm',22),
	('/show_gm',23),
	('/show_gm',24),
	('/show_gm',25),
	('/show_gm',30),
	('/slide',10),
	('/slide',21),
	('/slide',22),
	('/slide',23),
	('/slide',24),
	('/slide',25),
	('/slide',30),
	('/snow',24),
	('/snow',25),
	('/snow',30),
	('/takeitem',23),
	('/takeitem',24),
	('/takeitem',25),
	('/takeitem',30),
	('/targetname',21),
	('/targetname',22),
	('/targetname',23),
	('/targetname',24),
	('/targetname',25),
	('/targetname',30),
	('/teleport',10),
	('/teleport',21),
	('/teleport',22),
	('/teleport',23),
	('/teleport',24),
	('/teleport',25),
	('/teleport',30),
	('/thaw',22),
	('/thaw',23),
	('/thaw',24),
	('/thaw',25),
	('/thaw',30),
	('/thunder',24),
	('/thunder',25),
	('/thunder',30),
	('/unban',24),
	('/unban',25),
	('/unban',30),
	('/unbanadvisor',22),
	('/unbanadvisor',23),
	('/unbanadvisor',24),
	('/unbanadvisor',25),
	('/unbanadvisor',30),
	('/unbanname',22),
	('/unbanname',23),
	('/unbanname',24),
	('/unbanname',25),
	('/unbanname',30),
	('/unmute',21),
	('/unmute',22),
	('/unmute',23),
	('/unmute',24),
	('/unmute',25),
	('/unmute',30),
	('/updaterespawn',30),
	('/warn',21),
	('/warn',22),
	('/warn',23),
	('/warn',24),
	('/warn',25),
	('/warn',30),
	('/weather',24),
	('/weather',25),
	('/weather',30),
	('always login',22),
	('always login',23),
	('always login',24),
	('always login',25),
	('always login',30),
	('cast all spells',22),
	('cast all spells',23),
	('cast all spells',24),
	('cast all spells',25),
	('cast all spells',30),
	('change NPC names',30),
	('changenameall',23),
	('changenameall',24),
	('changenameall',25),
	('changenameall',30),
	('command area',22),
	('command area',23),
	('command area',24),
	('command area',25),
	('command area',30),
	('default advisor',21),
	('default advisor',22),
	('default advisor',23),
	('default advisor',24),
	('default advisor',25),
	('default infinite inventory',21),
	('default infinite inventory',22),
	('default infinite inventory',23),
	('default infinite inventory',24),
	('default infinite inventory',25),
	('default infinite inventory',30),
	('default invincible',21),
	('default invincible',22),
	('default invincible',23),
	('default invincible',24),
	('default invincible',25),
	('default invincible',30),
	('default invisible',21),
	('default invisible',22),
	('default invisible',23),
	('default invisible',24),
	('default invisible',25),
	('default invisible',30),
	('morph others',24),
	('morph others',25),
	('morph others',30),
	('move others',22),
	('move others',23),
	('move others',24),
	('move others',25),
	('move others',30),
	('move unpickupables/spawns',25),
	('move unpickupables/spawns',30),
	('pickup override',22),
	('pickup override',23),
	('pickup override',24),
	('pickup override',25),
	('pickup override',30),
	('pos extras',10),
	('pos extras',21),
	('pos extras',22),
	('pos extras',23),
	('pos extras',24),
	('pos extras',25),
	('pos extras',30),
	('quest change others',24),
	('quest change others',25),
	('quest change others',30),
	('quest list others',22),
	('quest list others',23),
	('quest list others',24),
	('quest list others',25),
	('quest list others',30),
	('quest notify',10),
	('quest notify',21),
	('quest notify',22),
	('quest notify',23),
	('quest notify',24),
	('quest notify',25),
	('quest notify',30),
	('rotate all',22),
	('rotate all',23),
	('rotate all',24),
	('rotate all',25),
	('rotate all',30),
	('save quest disable',30),
	('setskill others',24),
	('setskill others',25),
	('setskill others',30),
	('view stats',21),
	('view stats',22),
	('view stats',23),
	('view stats',24),
	('view stats',25),
	('view stats',30);
/*!40000 ALTER TABLE `command_group_assignment` ENABLE KEYS*/;
UNLOCK TABLES;


DROP TABLE IF EXISTS `command_groups`;

#
# Table structure for table 'command_groups'
#

CREATE TABLE `command_groups` (
  `id` int(8) unsigned NOT NULL,
  `group_name` varchar(40) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB /*!40100 DEFAULT CHARSET=utf8*/;



#
# Dumping data for table 'command_groups'
#

LOCK TABLES `command_groups` WRITE;
/*!40000 ALTER TABLE `command_groups` DISABLE KEYS*/;
INSERT INTO `command_groups` (`id`, `group_name`) VALUES
	('0','Player'),
	('10','Tester'),
	('21','Initiate'),
	('22','Moderator'),
	('23','Advanced'),
	('24','Senior'),
	('25','Leader'),
	('30','Developers');
/*!40000 ALTER TABLE `command_groups` ENABLE KEYS*/;
UNLOCK TABLES;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS*/;
