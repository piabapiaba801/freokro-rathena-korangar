-- Server-side persistence for custom project systems.
CREATE TABLE IF NOT EXISTS `custom_regional_boss_state` (
  `region_key` varchar(32) NOT NULL,
  `map_name` varchar(32) NOT NULL,
  `kill_count` int unsigned NOT NULL DEFAULT 0,
  `boss_id` int unsigned NOT NULL DEFAULT 0,
  `boss_alive` tinyint unsigned NOT NULL DEFAULT 0,
  `last_boss_id` int unsigned NOT NULL DEFAULT 0,
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`region_key`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `custom_black_market_stock` (
  `slot` int unsigned NOT NULL AUTO_INCREMENT,
  `item_id` int unsigned NOT NULL,
  `price` int unsigned NOT NULL,
  `stock` int unsigned NOT NULL DEFAULT 0,
  `vip_only` tinyint unsigned NOT NULL DEFAULT 0,
  `enabled` tinyint unsigned NOT NULL DEFAULT 1,
  PRIMARY KEY (`slot`),
  UNIQUE KEY `item_id` (`item_id`)
) ENGINE=InnoDB;

-- Persistent per-account Black Market quota.
-- reset_date is compared with CURDATE(); quotas renew at the daily server/DB reset
-- and survive map-server crashes, restarts and unexpected shutdowns.
CREATE TABLE IF NOT EXISTS `custom_black_market_quota` (
  `account_id` int unsigned NOT NULL,
  `reset_date` date NOT NULL,
  `free_count` smallint unsigned NOT NULL DEFAULT 0,
  `vip_count` smallint unsigned NOT NULL DEFAULT 0,
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`account_id`),
  KEY `reset_date` (`reset_date`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `custom_black_market_log` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int unsigned NOT NULL,
  `item_id` int unsigned NOT NULL,
  `price` int unsigned NOT NULL,
  `amount` int unsigned NOT NULL DEFAULT 1,
  `purchased_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `char_id` (`char_id`),
  KEY `item_id` (`item_id`)
) ENGINE=InnoDB;

-- Market Clone / Offline Vending (@autotrade2)
CREATE TABLE IF NOT EXISTS `vending_clones` (
  `clone_id` int unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int unsigned NOT NULL,
  `name` varchar(24) NOT NULL,
  `title` varchar(80) NOT NULL DEFAULT '',
  `map` varchar(32) NOT NULL,
  `x` smallint unsigned NOT NULL DEFAULT 0,
  `y` smallint unsigned NOT NULL DEFAULT 0,
  `sex` tinyint unsigned NOT NULL DEFAULT 0,
  `class` smallint unsigned NOT NULL DEFAULT 0,
  `hair` smallint unsigned NOT NULL DEFAULT 0,
  `hair_color` smallint unsigned NOT NULL DEFAULT 0,
  `head_top` int unsigned NOT NULL DEFAULT 0,
  `head_mid` int unsigned NOT NULL DEFAULT 0,
  `head_bottom` int unsigned NOT NULL DEFAULT 0,
  `robe` int unsigned NOT NULL DEFAULT 0,
  `weapon` int unsigned NOT NULL DEFAULT 0,
  `shield` int unsigned NOT NULL DEFAULT 0,
  `body` smallint unsigned NOT NULL DEFAULT 0,
  `currency` int unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`clone_id`),
  UNIQUE KEY `char_id` (`char_id`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `vending_clone_items` (
  `clone_id` int unsigned NOT NULL,
  `index` smallint unsigned NOT NULL,
  `nameid` int unsigned NOT NULL,
  `amount` smallint unsigned NOT NULL DEFAULT 0,
  `price` int unsigned NOT NULL DEFAULT 0,
  `refine` tinyint unsigned NOT NULL DEFAULT 0,
  `attribute` tinyint unsigned NOT NULL DEFAULT 0,
  `identify` tinyint unsigned NOT NULL DEFAULT 1,
  `expire_time` int unsigned NOT NULL DEFAULT 0,
  `bound` tinyint unsigned NOT NULL DEFAULT 0,
  `unique_id` bigint unsigned NOT NULL DEFAULT 0,
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT 0,
  `durability` smallint unsigned NOT NULL DEFAULT 10000,
  `card0` int unsigned NOT NULL DEFAULT 0,
  `card1` int unsigned NOT NULL DEFAULT 0,
  `card2` int unsigned NOT NULL DEFAULT 0,
  `card3` int unsigned NOT NULL DEFAULT 0,
  `opt_idx0` smallint unsigned NOT NULL DEFAULT 0, `opt_val0` smallint NOT NULL DEFAULT 0, `opt_parm0` tinyint unsigned NOT NULL DEFAULT 0,
  `opt_idx1` smallint unsigned NOT NULL DEFAULT 0, `opt_val1` smallint NOT NULL DEFAULT 0, `opt_parm1` tinyint unsigned NOT NULL DEFAULT 0,
  `opt_idx2` smallint unsigned NOT NULL DEFAULT 0, `opt_val2` smallint NOT NULL DEFAULT 0, `opt_parm2` tinyint unsigned NOT NULL DEFAULT 0,
  `opt_idx3` smallint unsigned NOT NULL DEFAULT 0, `opt_val3` smallint NOT NULL DEFAULT 0, `opt_parm3` tinyint unsigned NOT NULL DEFAULT 0,
  `opt_idx4` smallint unsigned NOT NULL DEFAULT 0, `opt_val4` smallint NOT NULL DEFAULT 0, `opt_parm4` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`clone_id`,`index`),
  KEY `nameid` (`nameid`)
) ENGINE=InnoDB;

-- Upgrade existing Market Clone tables created before durability metadata was added.
ALTER TABLE `vending_clone_items` ADD COLUMN IF NOT EXISTS `expire_time` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `identify`;
ALTER TABLE `vending_clone_items` ADD COLUMN IF NOT EXISTS `bound` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `expire_time`;
ALTER TABLE `vending_clone_items` ADD COLUMN IF NOT EXISTS `unique_id` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `bound`;
ALTER TABLE `vending_clone_items` ADD COLUMN IF NOT EXISTS `enchantgrade` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `unique_id`;
ALTER TABLE `vending_clone_items` ADD COLUMN IF NOT EXISTS `durability` SMALLINT UNSIGNED NOT NULL DEFAULT 10000 AFTER `enchantgrade`;
ALTER TABLE `vending_clone_items` ADD COLUMN IF NOT EXISTS `opt_parm0` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `opt_val0`;
ALTER TABLE `vending_clone_items` ADD COLUMN IF NOT EXISTS `opt_parm1` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `opt_val1`;
ALTER TABLE `vending_clone_items` ADD COLUMN IF NOT EXISTS `opt_parm2` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `opt_val2`;
ALTER TABLE `vending_clone_items` ADD COLUMN IF NOT EXISTS `opt_parm3` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `opt_val3`;
ALTER TABLE `vending_clone_items` ADD COLUMN IF NOT EXISTS `opt_parm4` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `opt_val4`;

-- Enhanced MVP Tomb: persistent kill and participation history
CREATE TABLE IF NOT EXISTS `custom_mvp_tomb_history` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `mob_id` int unsigned NOT NULL,
  `mob_name` varchar(64) NOT NULL,
  `map` varchar(32) NOT NULL,
  `x` smallint NOT NULL,
  `y` smallint NOT NULL,
  `killer_name` varchar(24) NOT NULL DEFAULT 'Unknown',
  `max_hp` bigint NOT NULL DEFAULT 0,
  `battle_duration_ms` int unsigned NOT NULL DEFAULT 0,
  `killed_at` datetime NOT NULL,
  PRIMARY KEY (`id`),
  KEY `mob_id` (`mob_id`),
  KEY `killed_at` (`killed_at`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `custom_mvp_tomb_participants` (
  `history_id` bigint unsigned NOT NULL,
  `char_id` int unsigned NOT NULL,
  `char_name` varchar(24) NOT NULL,
  `damage` bigint NOT NULL DEFAULT 0,
  `damage_tanked` bigint NOT NULL DEFAULT 0,
  `party_id` int unsigned NOT NULL DEFAULT 0,
  `guild_id` int unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`history_id`,`char_id`),
  KEY `char_id` (`char_id`),
  KEY `guild_id` (`guild_id`)
) ENGINE=InnoDB;
-- Card Drop Announcement & Audit Log (Normal / Champion / Boss-MVP)
CREATE TABLE IF NOT EXISTS `custom_card_drop_log` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` INT UNSIGNED NOT NULL DEFAULT 0,
  `char_id` INT UNSIGNED NOT NULL DEFAULT 0,
  `mob_id` INT UNSIGNED NOT NULL,
  `item_id` INT UNSIGNED NOT NULL,
  `category` ENUM('NORMAL','CHAMPION','BOSS_MVP') NOT NULL,
  `champion_type` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `map` VARCHAR(32) NOT NULL DEFAULT '',
  `x` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `y` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `dropped_at` DATETIME NOT NULL,
  PRIMARY KEY (`id`),
  KEY `idx_card_drop_char` (`char_id`,`dropped_at`),
  KEY `idx_card_drop_item` (`item_id`,`dropped_at`),
  KEY `idx_card_drop_category` (`category`,`dropped_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- FreokiRO equipment durability migration (10000 = 100.00%)
ALTER TABLE `inventory` ADD COLUMN IF NOT EXISTS `durability` SMALLINT UNSIGNED NOT NULL DEFAULT 10000 AFTER `enchantgrade`;
ALTER TABLE `cart_inventory` ADD COLUMN IF NOT EXISTS `durability` SMALLINT UNSIGNED NOT NULL DEFAULT 10000 AFTER `enchantgrade`;
ALTER TABLE `storage` ADD COLUMN IF NOT EXISTS `durability` SMALLINT UNSIGNED NOT NULL DEFAULT 10000 AFTER `enchantgrade`;
ALTER TABLE `guild_storage` ADD COLUMN IF NOT EXISTS `durability` SMALLINT UNSIGNED NOT NULL DEFAULT 10000 AFTER `enchantgrade`;
ALTER TABLE `mail_attachments` ADD COLUMN IF NOT EXISTS `durability` SMALLINT UNSIGNED NOT NULL DEFAULT 10000 AFTER `enchantgrade`;
ALTER TABLE `auction` ADD COLUMN IF NOT EXISTS `durability` SMALLINT UNSIGNED NOT NULL DEFAULT 10000 AFTER `enchantgrade`;


-- FreaokRO Market Clone 2.0 / Extended Vending
-- Currency encoding: 0=Zeny, 4294967295=Cash Points, 4294967294=Kafra Points, otherwise Item ID.
ALTER TABLE `vendings` ADD COLUMN IF NOT EXISTS `extended_vending_item` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `autotrade`;
CREATE TABLE IF NOT EXISTS `vending_clone_point_payouts` (
  `char_id` INT UNSIGNED NOT NULL,
  `currency` INT UNSIGNED NOT NULL,
  `amount` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`currency`)
) ENGINE=InnoDB;

-- FreaokRO V14 - Account-wide Monster Catalog / Olhar do Cacador
CREATE TABLE IF NOT EXISTS `monster_catalog` (
  `account_id` INT UNSIGNED NOT NULL,
  `mob_id` INT UNSIGNED NOT NULL,
  `discovered_char_id` INT UNSIGNED NOT NULL,
  `category` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0 normal, 1 miniboss, 2 MVP',
  `map` VARCHAR(16) NOT NULL DEFAULT '',
  `discovered_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `reward_state` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0 pending, 1 claimed/in-flight, 2 delivered',
  `reward_base` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `reward_job` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `rewarded_at` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`account_id`,`mob_id`),
  KEY `idx_monster_catalog_mob` (`mob_id`),
  KEY `idx_monster_catalog_reward` (`reward_state`)
) ENGINE=InnoDB;

-- FreaokRO Custom Auction House (Black Market / Leiloes)
-- Server-authoritative foundation. Item/bid state is persistent in InnoDB.
CREATE TABLE IF NOT EXISTS `custom_auction` (
  `auction_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `seller_account_id` int unsigned NOT NULL DEFAULT 0,
  `seller_char_id` int unsigned NOT NULL DEFAULT 0,
  `seller_name` varchar(24) NOT NULL DEFAULT '',
  `is_server` tinyint unsigned NOT NULL DEFAULT 0,
  `server_creator_account_id` int unsigned NOT NULL DEFAULT 0,
  `server_creator_char_id` int unsigned NOT NULL DEFAULT 0,
  `server_note` varchar(80) NOT NULL DEFAULT '',
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
  `ended_at` datetime NULL,
  `settlement_reason` varchar(16) NOT NULL DEFAULT '',
  `status` enum('ESCROW_PENDING','ACTIVE','SETTLEMENT_PENDING','CANCEL_PENDING','SOLD','EXPIRED','CANCELLED') NOT NULL DEFAULT 'ACTIVE',
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
  UNIQUE KEY `auction_once` (`auction_id`),
  KEY `item_sold` (`item_id`,`sold_at`),
  KEY `seller_sold` (`seller_char_id`,`sold_at`),
  KEY `buyer_sold` (`buyer_char_id`,`sold_at`)
) ENGINE=InnoDB;



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

CREATE TABLE IF NOT EXISTS `custom_auction_wallet` (
  `account_id` int unsigned NOT NULL,
  `balance` bigint unsigned NOT NULL DEFAULT 0,
  `refund_char_id` int unsigned NOT NULL DEFAULT 0,
  `refund_char_name` varchar(24) NOT NULL DEFAULT '',
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`account_id`)
) ENGINE=InnoDB;
