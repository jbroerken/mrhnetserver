-- ------------------------
-- MRH Net Server Database & Tables
--
-- This SQL file creates the database 
-- and table structure for the MRH Net 
-- Server MySQL database.
-- ------------------------

--
-- Database
--

DROP DATABASE IF EXISTS `mrhnetserver`;

CREATE DATABASE `mrhnetserver` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;


--
-- User Account Table
--

DROP TABLE IF EXISTS `user_account`;

CREATE TABLE `user_account` 
(
    `user_id` int unsigned NOT NULL UNIQUE AUTO_INCREMENT COMMENT 'Unique user identification',
    `mail_address` varchar(128) NOT NULL DEFAULT '' COMMENT 'User account mail address',
    `password` varchar(255) NOT NULL DEFAULT '' COMMENT 'User account password hash',
    PRIMARY KEY (`user_id`)
) 
DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='User accounts for server authentication';


--
-- User Device List
--

DROP TABLE IF EXISTS `user_device_list`;

CREATE TABLE `user_device_list` 
(
    `user_id` int unsigned NOT NULL COMMENT 'User identifier',
    `device_key` varchar(25) NOT NULL DEFAULT '' COMMENT 'User assigned device key',
    FOREIGN KEY (`user_id`) REFERENCES user_account(`user_id`)
) 
DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Known user devices';


--
-- Channel List
--

DROP TABLE IF EXISTS `channel_list`;

CREATE TABLE `channel_list` 
(
    `channel_id` int unsigned NOT NULL COMMENT 'Channel identifier',
    `name` varchar(64) NOT NULL DEFAULT '' COMMENT 'Channel identification name',
    `address` varchar(256) NOT NULL DEFAULT '127.0.0.1' COMMENT 'Channel network address',
    `port` int NOT NULL DEFAULT '-1' COMMENT 'Channel network port',
    `assistant_connections` int unsigned NOT NULL DEFAULT '0' COMMENT 'Channel assistant connection count',
    `last_update_s` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'Channel update time stamp',
    PRIMARY KEY (`channel_id`)
) 
DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Known channels for communication';


--
-- Channel Device Connections
--

DROP TABLE IF EXISTS `channel_device_connections`;

CREATE TABLE `channel_device_connections` 
(
    `channel_id` int unsigned NOT NULL COMMENT 'Channel list identifier',
    `user_id` int unsigned NOT NULL COMMENT 'User Account identifier',
    `device_key` varchar(255) NOT NULL DEFAULT '' COMMENT 'Connection device key',
    FOREIGN KEY (`channel_id`) REFERENCES channel_list(`channel_id`),
    FOREIGN KEY (`user_id`) REFERENCES user_account(`user_id`)
) 
DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Active connections for a active channel';