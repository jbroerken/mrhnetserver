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
-- Client Messages
--

DROP TABLE IF EXISTS `message_data`;

CREATE TABLE `message_data` 
(
    `message_id` bigint unsigned NOT NULL UNIQUE AUTO_INCREMENT COMMENT 'Unique message identification',
    `user_id` int unsigned NOT NULL COMMENT 'User identifier',
    `device_key` varchar(25) NOT NULL DEFAULT '' COMMENT 'User assigned device key',
    `actor_type` tinyint unsigned NOT NULL DEFAULT '0' COMMENT 'Actor origin',
    `message_type` tinyint unsigned NOT NULL DEFAULT '0' COMMENT 'Message type',
    `message_data` varchar(2048) NOT NULL DEFAULT '' COMMENT 'Message data',
    PRIMARY KEY (`message_id`),
    FOREIGN KEY (`user_id`) REFERENCES user_account(`user_id`)
) 
DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Recieved and store currently held messages';