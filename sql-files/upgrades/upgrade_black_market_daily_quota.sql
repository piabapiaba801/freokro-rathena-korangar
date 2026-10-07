-- FreokRO Black Market: persistent daily quotas.
-- Safe to run repeatedly.
CREATE TABLE IF NOT EXISTS `custom_black_market_quota` (
  `account_id` int unsigned NOT NULL,
  `reset_date` date NOT NULL,
  `free_count` smallint unsigned NOT NULL DEFAULT 0,
  `vip_count` smallint unsigned NOT NULL DEFAULT 0,
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`account_id`),
  KEY `reset_date` (`reset_date`)
) ENGINE=InnoDB;
