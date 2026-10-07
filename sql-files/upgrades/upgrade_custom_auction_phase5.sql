-- FreokRO Custom Auction - Phase 5 economy/audit upgrade.
ALTER TABLE `custom_auction`
  ADD COLUMN IF NOT EXISTS `listing_fee` bigint unsigned NOT NULL DEFAULT 0 AFTER `buy_now`;
ALTER TABLE `custom_auction_history`
  ADD COLUMN IF NOT EXISTS `sale_tax` bigint unsigned NOT NULL DEFAULT 0 AFTER `final_price`,
  ADD COLUMN IF NOT EXISTS `seller_net` bigint unsigned NOT NULL DEFAULT 0 AFTER `sale_tax`;
