// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom battle config initializations
 * Place the definition of custom battle configuration initializations in this file.
 *
 * Do not forget to add a comma at the end of the line, if you want to add another config
 *
 * Format:
 *	 { "name", &battle_config.<variable name>, <default value>, <minimum value>, <maximum value> },
 **/


{ "server_guard_enable", &battle_config.server_guard_enable, 1, 0, 1 },
{ "server_guard_log", &battle_config.server_guard_log, 1, 0, 1 },
{ "server_guard_drop_excess", &battle_config.server_guard_drop_excess, 1, 0, 1 },
{ "server_guard_skill_pps", &battle_config.server_guard_skill_pps, 30, 0, 1000 },
{ "server_guard_walk_pps", &battle_config.server_guard_walk_pps, 40, 0, 1000 },
{ "monster_catalog_exp_enable", &battle_config.monster_catalog_exp_enable, 1, 0, 1 },

{ "funk_master_enable", &battle_config.funk_master_enable, 1, 0, 1 },
{ "funk_market_clone", &battle_config.funk_market_clone, 1, 0, 1 },
{ "funk_random_options", &battle_config.funk_random_options, 1, 0, 1 },
{ "funk_preserve_toggle", &battle_config.funk_preserve_toggle, 1, 0, 1 },
{ "funk_gear_protect", &battle_config.funk_gear_protect, 1, 0, 1 },
{ "funk_party_synergy", &battle_config.funk_party_synergy, 1, 0, 1 },
{ "funk_mvp_tomb", &battle_config.funk_mvp_tomb, 1, 0, 1 },
{ "funk_regional_boss", &battle_config.funk_regional_boss, 1, 0, 1 },
{ "funk_basic_skills", &battle_config.funk_basic_skills, 1, 0, 1 },
{ "funk_champion", &battle_config.funk_champion, 1, 0, 1 },
{ "funk_server_guard", &battle_config.funk_server_guard, 1, 0, 1 },
{ "funk_voice_chat", &battle_config.funk_voice_chat, 1, 0, 1 },
{ "funk_card_log", &battle_config.funk_card_log, 1, 0, 1 },
{ "funk_skill_critical", &battle_config.funk_skill_critical, 1, 0, 1 },
{ "funk_rental_card_return", &battle_config.funk_rental_card_return, 1, 0, 1 },
{ "funk_welcome", &battle_config.funk_welcome, 1, 0, 1 },
{ "funk_autoattack", &battle_config.funk_autoattack, 1, 0, 1 },
{ "funk_durability", &battle_config.funk_durability, 1, 0, 1 },
{ "funk_monster_catalog", &battle_config.funk_monster_catalog, 1, 0, 1 },
{ "funk_extended_vending", &battle_config.funk_extended_vending, 1, 0, 1 },
{ "funk_auction", &battle_config.funk_auction, 1, 0, 1 },
{ "funk_area_loot", &battle_config.funk_area_loot, 1, 0, 1 },
{ "funk_monster_read", &battle_config.funk_monster_read, 1, 0, 1 },
{ "funk_universal_craft", &battle_config.funk_universal_craft, 1, 0, 1 },
{ "funk_return_save", &battle_config.funk_return_save, 1, 0, 1 },
{ "funk_tame_creature", &battle_config.funk_tame_creature, 1, 0, 1 },
{ "funk_ecommerce", &battle_config.funk_ecommerce, 1, 0, 1 },
{ "funk_leaf_wings", &battle_config.funk_leaf_wings, 1, 0, 1 },

{ "black_market_teleport_fee_type", &battle_config.black_market_teleport_fee_type, 0, 0, 2 },
{ "black_market_teleport_zeny", &battle_config.black_market_teleport_zeny, 0, 0, MAX_ZENY },
{ "black_market_teleport_item_id", &battle_config.black_market_teleport_item_id, 0, 0, INT32_MAX },
{ "black_market_teleport_item_amount", &battle_config.black_market_teleport_item_amount, 1, 1, MAX_AMOUNT },
