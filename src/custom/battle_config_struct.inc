// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom battle config structure
 * Place the definition of custom battle configuration structure members in this file.
 *
 * Make sure that the name you want is not already taken by looking into battle.hpp
 * Do not forget to add initilization logic to battle_config_init.inc
 *
 * Format:
 *	 <datatype> name;
 **/


// ServerGuard - conservative server-side anti-cheat / flood controls.
int32 server_guard_enable;
int32 server_guard_log;
int32 server_guard_drop_excess;
int32 server_guard_skill_pps;
int32 server_guard_walk_pps;

// FreaokRO Monster Catalog - runtime reward switch. Discovery/catalog remain active.
int32 monster_catalog_exp_enable;

// FreaokRO FUNK emergency feature switches. Runtime mutable through @funk.
int32 funk_master_enable;
int32 funk_market_clone;
int32 funk_random_options;
int32 funk_preserve_toggle;
int32 funk_gear_protect;
int32 funk_party_synergy;
int32 funk_mvp_tomb;
int32 funk_regional_boss;
int32 funk_basic_skills;
int32 funk_champion;
int32 funk_server_guard;
int32 funk_voice_chat;
int32 funk_card_log;
int32 funk_skill_critical;
int32 funk_rental_card_return;
int32 funk_welcome;
int32 funk_autoattack;
int32 funk_durability;
int32 funk_monster_catalog;
int32 funk_extended_vending;
int32 funk_auction;
int32 funk_area_loot;
int32 funk_monster_read;
int32 funk_universal_craft;
int32 funk_return_save;
int32 funk_tame_creature;
int32 funk_ecommerce;
int32 funk_leaf_wings;

// FreokRO Black Market shop finder.
// Group level >= 1 may open player shops remotely. Lower groups are sent to the shop location.
int32 black_market_teleport_fee_type;   // 0=free, 1=Zeny, 2=item
int32 black_market_teleport_zeny;
int32 black_market_teleport_item_id;
int32 black_market_teleport_item_amount;
