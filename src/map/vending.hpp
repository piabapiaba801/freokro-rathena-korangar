// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_VENDING_HPP_
#define	_VENDING_HPP_

#include <common/cbasetypes.hpp>
#include <common/db.hpp>
#include <common/mmo.hpp>

class map_session_data;
struct s_search_store_search;
struct s_autotrader;

static constexpr uint32 VENDING_CURRENCY_ZENY = 0;
static constexpr uint32 VENDING_CURRENCY_CASH = UINT32_MAX;
static constexpr uint32 VENDING_CURRENCY_KAFRA = UINT32_MAX - 1;

struct s_vending {
	int16 index; /// cart index (return item data)
	int16 amount; ///amout of the item for vending
	uint32 value; ///at which price
};

DBMap * vending_getdb();
void do_final_vending(void);
void do_init_vending(void);
void do_init_vending_autotrade( void );
 
void vending_reopen( map_session_data& sd );
void vending_closevending(map_session_data* sd);
int8 vending_openvending( map_session_data& sd, const char* message, const uint8* data, int32 count, struct s_autotrader *at );
void vending_vendinglistreq(map_session_data* sd, int32 id);
void vending_purchasereq(map_session_data* sd, int32 aid, int32 uid, const uint8* data, int32 count);
bool vending_search( const map_session_data* sd, t_itemid nameid );
bool vending_searchall( const map_session_data* sd, const s_search_store_search* s );
bool vending_clone_searchall( const s_search_store_search* s );
struct npc_data* vending_clone_by_npc_id(int32 npc_id);
void vending_update(map_session_data &sd);

void vending_create_clone(map_session_data* sd);
void vending_clone_purchase(map_session_data* sd, struct npc_data* nd, const uint8* data, int32 count);
void vending_clone_delete(struct npc_data* nd);
void vending_init_clones(void);
void vending_deliver_pending_point_payouts(map_session_data* sd);
bool vending_has_clone(uint32 char_id);
int8 vending_close_clone(map_session_data* sd);

#endif /* _VENDING_HPP_ */
