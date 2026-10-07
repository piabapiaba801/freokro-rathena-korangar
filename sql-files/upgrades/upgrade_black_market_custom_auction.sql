-- FreaokRO Custom Auction House - initial persistent schema.

-- FreaokRO Custom Auction House (Black Market / Leiloes)
-- Server-authoritative foundation. Item/bid state is persistent in InnoDB.
CREATE TABLE IF NOT EXISTS `custom_auction` (
  `auction_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `seller_account_id` int unsigned NOT NULL DEFAULT 0,
  `seller_char_id` int unsigned NOT NULL DEFAULT 0,
  `seller_name` varchar(24) NOT NULL DEFAULT '',
  `is_server` tinyint unsigned NOT NULL DEFAULT 0,
  `item_id` int unsigned NOT NULL,
  `amount` int unsigned NOT NULL DEFAULT 1,
  `identify` tinyint unsigned NOT NULL DEFAULT 1,
  `refine` tinyint unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint unsigned NOT NULL DEFAULT 0,
  `bound` tinyint unsigned NOT NULL DEFAULT 0,
  `expire_time` int unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint unsigned NOT NULL DEFAULT 0,
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT 0,
  `durability` smallint unsigned NOT NULL DEFAULT 10000,
  `card0` int unsigned NOT NULL DEFAULT 0,
  `card1` int unsigned NOT NULL DEFAULT 0,
  `card2` int unsigned NOT NULL DEFAULT 0,
  `card3` int unsigned NOT NULL DEFAULT 0,
  `opt_id0` smallint unsigned NOT NULL DEFAULT 0, `opt_val0` smallint NOT NULL DEFAULT 0, `opt_par0` tinyint NOT NULL DEFAULT 0,
  `opt_id1` smallint unsigned NOT NULL DEFAULT 0, `opt_val1` smallint NOT NULL DEFAULT 0, `opt_par1` tinyint NOT NULL DEFAULT 0,
  `opt_id2` smallint unsigned NOT NULL DEFAULT 0, `opt_val2` smallint NOT NULL DEFAULT 0, `opt_par2` tinyint NOT NULL DEFAULT 0,
  `opt_id3` smallint unsigned NOT NULL DEFAULT 0, `opt_val3` smallint NOT NULL DEFAULT 0, `opt_par3` tinyint NOT NULL DEFAULT 0,
  `opt_id4` smallint unsigned NOT NULL DEFAULT 0, `opt_val4` smallint NOT NULL DEFAULT 0, `opt_par4` tinyint NOT NULL DEFAULT 0,
  `start_price` bigint unsigned NOT NULL,
  `current_bid` bigint unsigned NOT NULL DEFAULT 0,
  `buy_now` bigint unsigned NOT NULL DEFAULT 0,
  `highest_bidder_account_id` int unsigned NOT NULL DEFAULT 0,
  `highest_bidder_char_id` int unsigned NOT NULL DEFAULT 0,
  `highest_bidder_name` varchar(24) NOT NULL DEFAULT '',
  `bid_count` int unsigned NOT NULL DEFAULT 0,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `ends_at` datetime NOT NULL,
  `status` enum('ESCROW_PENDING','ACTIVE','SOLD','EXPIRED','CANCELLED') NOT NULL DEFAULT 'ACTIVE',
  PRIMARY KEY (`auction_id`),
  KEY `active_end` (`status`,`ends_at`),
  KEY `item_active` (`item_id`,`status`),
  KEY `seller_active` (`seller_account_id`,`status`),
  KEY `bidder_active` (`highest_bidder_account_id`,`status`),
  KEY `server_active` (`is_server`,`status`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `custom_auction_bid` (
  `bid_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `auction_id` bigint unsigned NOT NULL,
  `account_id` int unsigned NOT NULL,
  `char_id` int unsigned NOT NULL,
  `char_name` varchar(24) NOT NULL DEFAULT '',
  `amount` bigint unsigned NOT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `refunded` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`bid_id`),
  KEY `auction_time` (`auction_id`,`created_at`),
  KEY `account_time` (`account_id`,`created_at`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `custom_auction_history` (
  `history_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `auction_id` bigint unsigned NOT NULL,
  `item_id` int unsigned NOT NULL,
  `amount` int unsigned NOT NULL DEFAULT 1,
  `refine` tinyint unsigned NOT NULL DEFAULT 0,
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT 0,
  `seller_char_id` int unsigned NOT NULL DEFAULT 0,
  `buyer_char_id` int unsigned NOT NULL DEFAULT 0,
  `final_price` bigint unsigned NOT NULL DEFAULT 0,
  `is_server` tinyint unsigned NOT NULL DEFAULT 0,
  `sold_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`history_id`),
  KEY `item_sold` (`item_id`,`sold_at`),
  KEY `seller_sold` (`seller_char_id`,`sold_at`),
  KEY `buyer_sold` (`buyer_char_id`,`sold_at`)
) ENGINE=InnoDB;

-- Phase 2: escrow journal state for crash-safe item hand-off.
ALTER TABLE `custom_auction`
  MODIFY `status` enum('ESCROW_PENDING','ACTIVE','SOLD','EXPIRED','CANCELLED') NOT NULL DEFAULT 'ACTIVE';

-- Phase 3: persistent bid escrow / anti-sniping / Buy Now settlement.
CREATE TABLE IF NOT EXISTS `custom_auction_wallet` (
  `account_id` int unsigned NOT NULL,
  `balance` bigint unsigned NOT NULL DEFAULT 0,
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`account_id`)
) ENGINE=InnoDB;

ALTER TABLE `custom_auction`
  ADD COLUMN IF NOT EXISTS `ended_at` datetime NULL AFTER `ends_at`,
  ADD COLUMN IF NOT EXISTS `settlement_reason` varchar(16) NOT NULL DEFAULT '' AFTER `ended_at`;

ALTER TABLE `custom_auction`
  MODIFY `status` enum('ESCROW_PENDING','ACTIVE','SETTLEMENT_PENDING','CANCEL_PENDING','SOLD','EXPIRED','CANCELLED') NOT NULL DEFAULT 'ACTIVE';

-- Phase 4: crash-safe settlement / RodEx refund routing.
ALTER TABLE `custom_auction_wallet`
  ADD COLUMN IF NOT EXISTS `refund_char_id` int unsigned NOT NULL DEFAULT 0 AFTER `balance`,
  ADD COLUMN IF NOT EXISTS `refund_char_name` varchar(24) NOT NULL DEFAULT '' AFTER `refund_char_id`;

ALTER TABLE `custom_auction_history`
  ADD UNIQUE KEY IF NOT EXISTS `auction_once` (`auction_id`);


-- Phase 5: economy and auditable fees/taxes.
ALTER TABLE `custom_auction`
  ADD COLUMN IF NOT EXISTS `listing_fee` bigint unsigned NOT NULL DEFAULT 0 AFTER `buy_now`;
ALTER TABLE `custom_auction_history`
  ADD COLUMN IF NOT EXISTS `sale_tax` bigint unsigned NOT NULL DEFAULT 0 AFTER `final_price`,
  ADD COLUMN IF NOT EXISTS `seller_net` bigint unsigned NOT NULL DEFAULT 0 AFTER `sale_tax`;
