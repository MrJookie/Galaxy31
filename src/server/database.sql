/*
Navicat MySQL Data Transfer

Source Server         : Archlinux
Source Server Version : 50505
Source Host           : 89.177.76.215:3306
Source Database       : test

Target Server Type    : MYSQL
Target Server Version : 50505
File Encoding         : 65001

Date: 2016-07-26 03:08:34
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `abilities`
-- ----------------------------
DROP TABLE IF EXISTS `abilities`;
CREATE TABLE `abilities` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(40) DEFAULT NULL,
  `regeneration` int(11) DEFAULT NULL,
  `mining` int(11) DEFAULT NULL,
  `boost` int(11) DEFAULT NULL,
  `stealth` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of abilities
-- ----------------------------

-- ----------------------------
-- Table structure for `accounts`
-- ----------------------------
DROP TABLE IF EXISTS `accounts`;
CREATE TABLE `accounts` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `email` varchar(255) NOT NULL,
  `username` varchar(10) NOT NULL,
  `password` char(40) NOT NULL,
  `active` int(1) NOT NULL,
  `ip_addr` varbinary(16) NOT NULL,
  `datetime_registered` datetime NOT NULL,
  `datetime_last_login` datetime NOT NULL,
  `current_ship_id` int(11) NOT NULL,
  `money` int(11) NOT NULL,
  `resource_cobalt` int(11) NOT NULL,
  `resource_uranium` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of accounts
-- ----------------------------

-- ----------------------------
-- Table structure for `chassis`
-- ----------------------------
DROP TABLE IF EXISTS `chassis`;
CREATE TABLE `chassis` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(40) DEFAULT NULL,
  `texture_file` varchar(40) DEFAULT NULL,
  `price` int(11) DEFAULT NULL,
  `health` int(11) DEFAULT NULL,
  `armor` int(11) DEFAULT NULL,
  `weight` int(11) DEFAULT NULL,
  `speed` int(11) DEFAULT NULL,
  `mountable_slots` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of chassis
-- ----------------------------

-- ----------------------------
-- Table structure for `chat_log`
-- ----------------------------
DROP TABLE IF EXISTS `chat_log`;
CREATE TABLE `chat_log` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `src_account_id` int(11) DEFAULT NULL,
  `dst_account_id` int(11) DEFAULT NULL,
  `message` varchar(255) DEFAULT NULL,
  `datetime` datetime DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of chat_log
-- ----------------------------

-- ----------------------------
-- Table structure for `friendlist`
-- ----------------------------
DROP TABLE IF EXISTS `friendlist`;
CREATE TABLE `friendlist` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `src_account_id` int(11) DEFAULT NULL,
  `dst_account_id` int(11) DEFAULT NULL,
  `status` int(1) DEFAULT NULL,
  `datetime` datetime DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of friendlist
-- ----------------------------

-- ----------------------------
-- Table structure for `items`
-- ----------------------------
DROP TABLE IF EXISTS `items`;
CREATE TABLE `items` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(40) DEFAULT NULL,
  `price` int(11) DEFAULT NULL,
  `health` int(11) DEFAULT NULL,
  `weight` int(11) DEFAULT NULL,
  `armor` int(11) DEFAULT NULL,
  `damage` int(11) DEFAULT NULL,
  `slots_occupying` int(11) DEFAULT NULL,
  `primary_ability_id` int(11) DEFAULT NULL,
  `secondary_ability_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of items
-- ----------------------------

-- ----------------------------
-- Table structure for `mountables`
-- ----------------------------
DROP TABLE IF EXISTS `mountables`;
CREATE TABLE `mountables` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(40) DEFAULT NULL,
  `slot` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of mountables
-- ----------------------------

-- ----------------------------
-- Table structure for `nodes`
-- ----------------------------
DROP TABLE IF EXISTS `nodes`;
CREATE TABLE `nodes` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `ip_addr` varbinary(16) DEFAULT NULL,
  `status` int(1) DEFAULT NULL,
  `players_online` int(11) DEFAULT NULL,
  `cpu_usage` varchar(255) DEFAULT NULL,
  `mem_usage` varchar(255) DEFAULT NULL,
  `hdd_usage` varchar(255) DEFAULT NULL,
  `network_bandwidth` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of nodes
-- ----------------------------

-- ----------------------------
-- Table structure for `ships`
-- ----------------------------
DROP TABLE IF EXISTS `ships`;
CREATE TABLE `ships` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account_id` int(11) DEFAULT NULL,
  `name` varchar(40) DEFAULT NULL,
  `level` int(11) DEFAULT NULL,
  `chassi_id` int(11) DEFAULT NULL,
  `skin_file` varchar(40) DEFAULT NULL,
  `mountables_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of ships
-- ----------------------------

-- ----------------------------
-- Procedure structure for `AddGeometryColumn`
-- ----------------------------
DROP PROCEDURE IF EXISTS `AddGeometryColumn`;
DELIMITER ;;
CREATE DEFINER=`` PROCEDURE `AddGeometryColumn`(catalog varchar(64), t_schema varchar(64),
   t_name varchar(64), geometry_column varchar(64), t_srid int)
begin
  set @qwe= concat('ALTER TABLE ', t_schema, '.', t_name, ' ADD ', geometry_column,' GEOMETRY REF_SYSTEM_ID=', t_srid); PREPARE ls from @qwe; execute ls; deallocate prepare ls; end
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `DropGeometryColumn`
-- ----------------------------
DROP PROCEDURE IF EXISTS `DropGeometryColumn`;
DELIMITER ;;
CREATE DEFINER=`` PROCEDURE `DropGeometryColumn`(catalog varchar(64), t_schema varchar(64),
   t_name varchar(64), geometry_column varchar(64))
begin
  set @qwe= concat('ALTER TABLE ', t_schema, '.', t_name, ' DROP ', geometry_column); PREPARE ls from @qwe; execute ls; deallocate prepare ls; end
;;
DELIMITER ;
