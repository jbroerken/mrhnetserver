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
    `mail_address` varchar(255) NOT NULL DEFAULT '' COMMENT 'User account mail address',
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
    `device_key` varchar(255) NOT NULL DEFAULT '' COMMENT 'User platform device',
    FOREIGN KEY (`user_id`) REFERENCES user_account(`user_id`)
) 
DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Known user devices';


--
-- Channel List
--

DROP TABLE IF EXISTS `channel_list`;

CREATE TABLE `channel_list` 
(
    `channel_id` int unsigned NOT NULL UNIQUE AUTO_INCREMENT COMMENT 'Unique channel identification',
    `name` varchar(255) NOT NULL DEFAULT '' COMMENT 'Channel identification name',
    PRIMARY KEY (`channel_id`)
) 
DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Channel types available';


--
-- Active Channels
--

DROP TABLE IF EXISTS `active_channels`;

CREATE TABLE `active_channels` 
(
    `channel_id` int unsigned NOT NULL COMMENT 'Channel list identifier',
    `address` varchar(255) NOT NULL DEFAULT '127.0.0.1' COMMENT 'Channel network address',
    `port` int NOT NULL DEFAULT '-1' COMMENT 'Channel network port',
    `connections` int unsigned NOT NULL DEFAULT '0' COMMENT 'Channel connection count',
    `connection_list_id` int unsigned NOT NULL UNIQUE AUTO_INCREMENT COMMENT 'Channel connection list identifier',
    `last_update_s` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'UTC channel time stamp',
    FOREIGN KEY (`channel_id`) REFERENCES channel_list(`channel_id`)
) 
DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Active channels for the given channel types';


--
-- Channel Device Connections
--

DROP TABLE IF EXISTS `channel_device_connections`;

CREATE TABLE `channel_device_connections` 
(
    `connection_list_id` int unsigned NOT NULL COMMENT 'Channel connection list identifier',
    `channel_id` int unsigned NOT NULL COMMENT 'Channel list identifier',
    `device_key` varchar(255) NOT NULL DEFAULT '' COMMENT 'Connection device key',
    `connection_state` int unsigned NOT NULL DEFAULT '0' COMMENT 'Connection state (none, platform, app + platform)',
    FOREIGN KEY (`connection_list_id`) REFERENCES active_channels(`connection_list_id`),
    FOREIGN KEY (`channel_id`) REFERENCES channel_list(`channel_id`)
) 
DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Active connections for a active channel';