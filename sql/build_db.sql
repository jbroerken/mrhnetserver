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
    `password` varchar(256) NOT NULL DEFAULT '' COMMENT 'User account password hash',
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
-- Server List
--

DROP TABLE IF EXISTS `server_list`;

CREATE TABLE `server_list` 
(
    `server_id` int unsigned NOT NULL COMMENT 'Server identifier',
    `channel` varchar(64) NOT NULL DEFAULT '' COMMENT 'Channel identification name',
    `address` varchar(256) NOT NULL DEFAULT '127.0.0.1' COMMENT 'Server network address',
    `port` int NOT NULL DEFAULT '-1' COMMENT 'Server network port',
    `assistant_connections` int unsigned NOT NULL DEFAULT '0' COMMENT 'Server assistant connection count',
    `max_connections` int unsigned NOT NULL DEFAULT '0' COMMENT 'Server total maximum connection count',
    `last_update_s` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'Server update time stamp',
    PRIMARY KEY (`server_id`)
) 
DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Known servers for communication';


--
-- Server Device Connections
--

DROP TABLE IF EXISTS `server_device_connections`;

CREATE TABLE `server_device_connections` 
(
    `server_id` int unsigned NOT NULL COMMENT 'Server list identifier',
    `user_id` int unsigned NOT NULL COMMENT 'User Account identifier',
    `device_key` varchar(25) NOT NULL DEFAULT '' COMMENT 'Connection device key',
    FOREIGN KEY (`server_id`) REFERENCES server_list(`server_id`),
    FOREIGN KEY (`user_id`) REFERENCES user_account(`user_id`)
) 
DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Active connections for a server';