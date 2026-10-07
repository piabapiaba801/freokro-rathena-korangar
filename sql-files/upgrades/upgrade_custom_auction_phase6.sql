-- FreaokRO Custom Auction Phase 6 - Server Special Auctions
ALTER TABLE `custom_auction`
  ADD COLUMN `server_creator_account_id` int unsigned NOT NULL DEFAULT 0 AFTER `is_server`,
  ADD COLUMN `server_creator_char_id` int unsigned NOT NULL DEFAULT 0 AFTER `server_creator_account_id`,
  ADD COLUMN `server_note` varchar(80) NOT NULL DEFAULT '' AFTER `server_creator_char_id`;

CREATE TABLE IF NOT EXISTS `custom_auction_admin_log` (
  `log_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `auction_id` bigint unsigned NOT NULL DEFAULT 0,
  `account_id` int unsigned NOT NULL DEFAULT 0,
  `char_id` int unsigned NOT NULL DEFAULT 0,
  `char_name` varchar(24) NOT NULL DEFAULT '',
  `action` varchar(16) NOT NULL,
  `details` varchar(255) NOT NULL DEFAULT '',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`),
  KEY `auction_time` (`auction_id`,`created_at`),
  KEY `admin_time` (`account_id`,`created_at`)
) ENGINE=InnoDB;
