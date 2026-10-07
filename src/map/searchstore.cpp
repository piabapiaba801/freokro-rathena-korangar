// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "searchstore.hpp"  // struct s_search_store_info
#include <ctime>

#include <common/cbasetypes.hpp>
#include <common/malloc.hpp>  // aMalloc, aRealloc, aFree
#include <common/showmsg.hpp>  // ShowError, ShowWarning
#include <common/strlib.hpp>  // safestrncpy

#include "battle.hpp"  // battle_config.*
#include "clif.hpp"  // clif_open_search_store_info, clif_search_store_info_*
#include "pc.hpp"  // map_session_data
#include "npc.hpp" // FreokRO Market Clone search
#include "map.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "script.hpp" // Black Market travel confirmation variables

/// Type for shop search function
typedef bool (*searchstore_search_t)( const map_session_data* sd, t_itemid nameid );
typedef bool (*searchstore_searchall_t)( const map_session_data* sd, const struct s_search_store_search* s );

/**
 * Retrieves search function by type.
 * @param type : type of search to conduct
 * @return : search type
 */
static searchstore_search_t searchstore_getsearchfunc(e_searchstore_searchtype type)
{
	switch( type ) {
		case SEARCHTYPE_VENDING:      return &vending_search;
		case SEARCHTYPE_BUYING_STORE: return &buyingstore_search;
	}

	return nullptr;
}

/**
 * Retrieves search-all function by type.
 * @param type : type of search to conduct
 * @return : search type
 */
static searchstore_searchall_t searchstore_getsearchallfunc(e_searchstore_searchtype type)
{
	switch( type ) {
		case SEARCHTYPE_VENDING:      return &vending_searchall;
		case SEARCHTYPE_BUYING_STORE: return &buyingstore_searchall;
	}

	return nullptr;
}

/**
 * Checks if the player has a store by type.
 * @param sd : player requesting
 * @param type : type of search to conduct
 * @return : store type
 */
static bool searchstore_hasstore( const map_session_data& sd, e_searchstore_searchtype type )
{
	switch( type ) {
		case SEARCHTYPE_VENDING:      return sd.state.vending;
		case SEARCHTYPE_BUYING_STORE: return sd.state.buyingstore;
	}

	return false;
}

/**
 * Returns player's store ID by type.
 * @param sd : player requesting
 * @param type : type of search to conduct
 * @return : store ID
 */
// FreokRO Black Market: group level 1+ is VIP for remote purchases.
static bool blackmarket_remote_vip(const map_session_data& sd) {
	return pc_get_group_level(const_cast<map_session_data*>(&sd)) >= 1;
}

// FreokRO Black Market: persistent daily quota for VIP remote shop access.
// The quota is stored immediately in SQL and keyed by the database calendar day.
// This makes the reset deterministic (daily at 00:00 database/server time) and
// prevents map-server crashes/restarts from restoring a player's quota.
static bool blackmarket_consume_vip_access(map_session_data& sd) {
	char* data = nullptr;
	int64 count = 0;

	if (SQL_ERROR == Sql_Query(mmysql_handle,
		"SELECT IF(`reset_date`=CURDATE(),`vip_count`,0) FROM `custom_black_market_quota` WHERE `account_id`='%d' LIMIT 1",
		sd.status.account_id)) {
		Sql_ShowDebug(mmysql_handle);
		clif_displaymessage(sd.fd, "Mercado Negro: nao foi possivel validar sua cota. Tente novamente.");
		return false; // fail closed: SQL failure must never grant extra uses
	}
	if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
		Sql_GetData(mmysql_handle, 0, &data, nullptr);
		if (data != nullptr)
			count = strtoll(data, nullptr, 10);
	}
	Sql_FreeResult(mmysql_handle);

	if (count >= 50) {
		clif_displaymessage(sd.fd, "Mercado Negro: limite VIP de 50 acessos remotos neste reset diario atingido.");
		return false;
	}

	// Persist before granting the remote access. If the map-server dies immediately
	// afterwards, the use remains consumed and cannot be recovered by restarting.
	if (SQL_ERROR == Sql_Query(mmysql_handle,
		"INSERT INTO `custom_black_market_quota` (`account_id`,`reset_date`,`vip_count`) VALUES ('%d',CURDATE(),1) "
		"ON DUPLICATE KEY UPDATE `vip_count`=IF(`reset_date`=CURDATE(),`vip_count`+1,1), "
		"`free_count`=IF(`reset_date`=CURDATE(),`free_count`,0), `reset_date`=CURDATE(), `updated_at`=CURRENT_TIMESTAMP",
		sd.status.account_id)) {
		Sql_ShowDebug(mmysql_handle);
		clif_displaymessage(sd.fd, "Mercado Negro: nao foi possivel salvar sua cota. Acesso cancelado.");
		return false;
	}
	return true;
}

// FreokRO Black Market: free users do not open a remote shop. Clicking a
// result prepares a destination and opens a confirmation dialog handled by
// CustomBlackMarket::OnTravelConfirm. The actual initial fee is 10,000 Zeny
// (configurable through battle_conf and passed to the script).
static bool blackmarket_travel_to_shop(map_session_data& sd, int16 map_id, int16 x, int16 y) {
	if (map_id < 0)
		return false;
	if ((map_getmapflag(map_id, MF_NOWARPTO) && !pc_has_permission(&sd, PC_PERM_WARP_ANYWHERE)) ||
		!pc_job_can_entermap(static_cast<enum e_job>(sd.status.class_), map_id, pc_get_group_level(&sd)) ||
		(sd.m >= 0 && map_getmapflag(sd.m, MF_NOWARP) && !pc_has_permission(&sd, PC_PERM_WARP_ANYWHERE))) {
		clif_displaymessage(sd.fd, "Mercado Negro: nao e permitido viajar para esta loja.");
		return false;
	}
	if (map_getcell(map_id, x, y, CELL_CHKNOPASS)) {
		clif_displaymessage(sd.fd, "Mercado Negro: a coordenada desta loja nao esta acessivel.");
		return false;
	}

	pc_setregstr(&sd, add_str("@BM_TRAVEL_MAP$"), map_mapid2mapname(map_id));
	pc_setreg(&sd, add_str("@BM_TRAVEL_X"), x);
	pc_setreg(&sd, add_str("@BM_TRAVEL_Y"), y);
	pc_setreg(&sd, add_str("@BM_TRAVEL_FEE_TYPE"), battle_config.black_market_teleport_fee_type);
	pc_setreg(&sd, add_str("@BM_TRAVEL_ZENY"), battle_config.black_market_teleport_zeny);
	pc_setreg(&sd, add_str("@BM_TRAVEL_ITEM"), battle_config.black_market_teleport_item_id);
	pc_setreg(&sd, add_str("@BM_TRAVEL_AMOUNT"), battle_config.black_market_teleport_item_amount);

	if (npc_event(&sd, "CustomBlackMarket::OnTravelConfirm", 0) != 0) {
		clif_displaymessage(sd.fd, "Mercado Negro: nao foi possivel abrir a confirmacao de viagem.");
		return false;
	}
	return true;
}

static int32 searchstore_getstoreid( const map_session_data& sd, e_searchstore_searchtype type )
{
	switch( type ) {
		case SEARCHTYPE_VENDING:      return sd.vender_id;
		case SEARCHTYPE_BUYING_STORE: return sd.buyer_id;
	}

	return 0;
}

/**
 * Send request to open Search Store.
 * @param sd : player requesting
 * @param uses : uses left to open
 * @param effect : shop type
 * @return : true : opened, false : failed to open
 */
bool searchstore_open(map_session_data& sd, uint16 uses, e_searchstore_effecttype effect, int16 mapid)
{
	if( sd.searchstore.open )
		return false;


	sd.searchstore.open   = true;
	sd.searchstore.uses   = uses;
	sd.searchstore.effect = effect;
	sd.searchstore.mapid  = mapid;

	clif_open_search_store_info(sd);

	return true;
}

/**
 * Query and present the results for the item.
 * @param sd : player requesting
 * @param type : shop type
 * @param min_price : minimum zeny price
 * @param max_price : maximum zeny price
 * @param itemlist : list with stored item results
 * @param item_count : amount of items in itemlist
 * @param cardlist : list with stored cards (cards attached to items)
 * @param card_count : amount of items in cardlist
 */
void searchstore_query(map_session_data& sd, e_searchstore_searchtype type, uint32 min_price, uint32 max_price, const struct PACKET_CZ_SEARCH_STORE_INFO_item* itemlist, uint32 item_count, const struct PACKET_CZ_SEARCH_STORE_INFO_item* cardlist, uint32 card_count)
{
	uint32 i;
	map_session_data* pl_sd;
	struct DBIterator *iter;
	struct s_search_store_search s;
	searchstore_searchall_t store_searchall;
	time_t querytime;

	if( !sd.searchstore.open )
		return;

	if( ( store_searchall = searchstore_getsearchallfunc(type) ) == nullptr ) {
		ShowError("searchstore_query: Unknown search type %u (account_id=%d).\n", type, sd.id);
		return;
	}

	time(&querytime);

	if( sd.searchstore.nextquerytime > querytime ) {
		clif_search_store_info_failed(sd, SSI_FAILED_LIMIT_SEARCH_TIME);
		return;
	}

	if( !sd.searchstore.uses ) {
		clif_search_store_info_failed(sd, SSI_FAILED_SEARCH_CNT);
		return;
	}

	// uses counter must be updated before validating the next search
	sd.searchstore.uses--;
	sd.searchstore.type = type;
	sd.searchstore.nextquerytime = querytime + battle_config.searchstore_querydelay;

	// drop previous results
	searchstore_clear(sd);

	// validate lists
	for( i = 0; i < item_count; i++ ) {
		if( !item_db.exists(itemlist[i].itemId) ) {
			ShowWarning("searchstore_query: Client resolved item %u is not known.\n", itemlist[i].itemId);
			clif_search_store_info_failed(sd, SSI_FAILED_NOTHING_SEARCH_ITEM);

			// update uses
			clif_search_store_info_ack(sd);
			return;
		}
	}
	for( i = 0; i < card_count; i++ ) {
		if( !item_db.exists(cardlist[i].itemId) ) {
			ShowWarning("searchstore_query: Client resolved card %u is not known.\n", cardlist[i].itemId);
			clif_search_store_info_failed(sd, SSI_FAILED_NOTHING_SEARCH_ITEM);

			// update uses
			clif_search_store_info_ack(sd);
			return;
		}
	}

	if( max_price < min_price )
		std::swap(min_price, max_price);

	// search
	s.search_sd  = &sd;
	s.itemlist   = itemlist;
	s.cardlist   = cardlist;
	s.item_count = item_count;
	s.card_count = card_count;
	s.min_price  = min_price;
	s.max_price  = max_price;
	iter         = db_iterator((type == SEARCHTYPE_VENDING) ? vending_getdb() : buyingstore_getdb());

	for( pl_sd = (map_session_data*)dbi_first(iter); dbi_exists(iter);  pl_sd = (map_session_data*)dbi_next(iter) ) {
		if( &sd == pl_sd ) // skip own shop, if any
			continue;

		// Skip stores that are not in the map defined by the search
		if (sd.searchstore.mapid != 0 && pl_sd->m != sd.searchstore.mapid) {
			continue;
		}

		if( !store_searchall(pl_sd, &s) ) { // exceeded result size
			clif_search_store_info_failed(sd, SSI_FAILED_OVER_MAXCOUNT);
			break;
		}
	}

	dbi_destroy(iter);

	// FreokRO Market Clone: persistent @autotrade2 shops are NPC-backed and
	// therefore are not members of vending_db. Include them explicitly when
	// searching vending stores so the Black Market finder sees every player shop.
	if (type == SEARCHTYPE_VENDING && !vending_clone_searchall(&s))
		clif_search_store_info_failed(sd, SSI_FAILED_OVER_MAXCOUNT);

	if( !sd.searchstore.items.empty() ) {
		// present results
		clif_search_store_info_ack( sd );

		// one page displayed
		sd.searchstore.pages++;
	} else {
		// cleanup
		searchstore_clear(sd);

		// notify of failure (must go before updating uses)
		clif_search_store_info_failed(sd, SSI_FAILED_NOTHING_SEARCH_ITEM);

		// update uses
		clif_search_store_info_ack( sd );
	}
}

/**
 * Checks whether or not more results are available for the client.
 * @param sd : player requesting
 * @return : true : more items to search, false : no more items
 */
bool searchstore_querynext( const map_session_data& sd )
{
	if( !sd.searchstore.items.empty() && ( sd.searchstore.items.size()-1 )/SEARCHSTORE_RESULTS_PER_PAGE > sd.searchstore.pages )
		return true;

	return false;
}

/**
 * Get and display the results for the next page.
 * @param sd : player requesting
 */
void searchstore_next(map_session_data& sd)
{
	if( !sd.searchstore.open || sd.searchstore.items.size() <= sd.searchstore.pages*SEARCHSTORE_RESULTS_PER_PAGE ) // nothing (more) to display
		return;

	// present results
	clif_search_store_info_ack( sd );

	// one more page displayed
	sd.searchstore.pages++;
}

/**
 * Prepare to clear information for closing of window.
 * @param sd : player requesting
 */
void searchstore_clear(map_session_data& sd)
{
	searchstore_clearremote(sd);

	sd.searchstore.items.clear();
	sd.searchstore.pages = 0;
}

/**
 * Close the Search Store window.
 * @param sd : player requesting
 */
void searchstore_close(map_session_data& sd)
{
	if( sd.searchstore.open ) {
		searchstore_clear(sd);

		sd.searchstore.uses = 0;
		sd.searchstore.open = false;
	}
}

/**
 * Process the actions (click) for the Search Store window.
 * @param sd : player requesting
 * @param account_id : account ID of owner's shop
 * @param store_id : store ID created by client
 * @param nameid : item being searched
 */
void searchstore_click(map_session_data& sd, uint32 account_id, int32 store_id, t_itemid nameid)
{
	uint32 i;
	map_session_data* pl_sd;
	searchstore_search_t store_search;

	if( !sd.searchstore.open || sd.searchstore.items.empty() )
		return;

	searchstore_clearremote(sd);

	ARR_FIND( 0, sd.searchstore.items.size(), i, sd.searchstore.items[i]->store_id == store_id && sd.searchstore.items[i]->account_id == account_id && sd.searchstore.items[i]->nameid == nameid );
	if( i == sd.searchstore.items.size() ) { // no such result, crafted
		ShowWarning("searchstore_click: Received request with item %u of account %d, which is not part of current result set (account_id=%d, char_id=%d).\n", nameid, account_id, sd.id, sd.status.char_id);
		clif_search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	// FreokRO Market Clone results use the clone NPC runtime id. They do not
	// have a map_session_data, so route the click directly to the clone shop.
	if (sd.searchstore.type == SEARCHTYPE_VENDING) {
		struct npc_data* clone = vending_clone_by_npc_id(static_cast<int32>(account_id));
		if (clone != nullptr && clone->id == store_id && clone->u.vending_clone.items != nullptr) {
			bool still_selling = false;
			for (const struct s_vending_clone_item& vci : *clone->u.vending_clone.items) {
				if (vci.amount > 0 && vci.item_data.nameid == nameid) {
					still_selling = true;
					break;
				}
			}
			if (!still_selling) {
				clif_search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
				return;
			}

			if (sd.searchstore.effect == SEARCHSTORE_EFFECT_NORMAL) {
				if (sd.m != clone->m) clif_search_store_info_click_ack(sd, -1, -1);
				else clif_search_store_info_click_ack(sd, clone->x, clone->y);
			} else if (sd.searchstore.effect == SEARCHSTORE_EFFECT_REMOTE) {
				if (blackmarket_remote_vip(sd)) {
					if (blackmarket_consume_vip_access(sd))
						clif_vending_clone_list(sd, *clone);
				}
				else
					blackmarket_travel_to_shop(sd, clone->m, clone->x, clone->y);
			}
			return;
		}
	}

	if( ( pl_sd = map_id2sd(account_id) ) == nullptr ) { // no longer online
		clif_search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	if( !searchstore_hasstore(*pl_sd, sd.searchstore.type) || searchstore_getstoreid(*pl_sd, sd.searchstore.type) != store_id ) { // no longer vending/buying or not same shop
		clif_search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	store_search = searchstore_getsearchfunc(sd.searchstore.type);

	if( !store_search(pl_sd, nameid) ) {// item no longer being sold/bought
		clif_search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	switch( sd.searchstore.effect ) {
		case SEARCHSTORE_EFFECT_NORMAL:
			// display coords
			if( sd.m != pl_sd->m ) // not on same map, wipe previous marker
				clif_search_store_info_click_ack(sd, -1, -1);
			else
				clif_search_store_info_click_ack(sd, pl_sd->x, pl_sd->y);
			break;
		case SEARCHSTORE_EFFECT_REMOTE:
			// FreokRO Black Market: only VIP/group level 1+ receives remote access.
			// Other users use the same result as a travel link to the physical shop.
			if (blackmarket_remote_vip(sd)) {
				if (!blackmarket_consume_vip_access(sd))
					break;
				sd.searchstore.remote_id = account_id;
				switch( sd.searchstore.type ) {
					case SEARCHTYPE_VENDING:      vending_vendinglistreq(&sd, account_id); break;
					case SEARCHTYPE_BUYING_STORE: buyingstore_open(&sd, account_id);       break;
				}
			} else {
				blackmarket_travel_to_shop(sd, pl_sd->m, pl_sd->x, pl_sd->y);
			}
			break;
		default:
			// unknown
			ShowError("searchstore_click: Unknown search store effect %u (account_id=%d).\n", sd.searchstore.effect, sd.id);
	}
}

/**
 * Checks whether or not sd has opened account_id's shop remotely.
 * @param sd : player requesting
 * @param account_id : account ID of owner's shop
 * @return : true : shop opened, false : shop not opened
 */
bool searchstore_queryremote( const map_session_data& sd, uint32 account_id )
{
	return (bool)( sd.searchstore.open && !sd.searchstore.items.empty() && sd.searchstore.remote_id == account_id );
}

/**
 * Removes range-check bypassing for remotely opened stores.
 * @param sd : player requesting
 */
void searchstore_clearremote(map_session_data& sd)
{
	sd.searchstore.remote_id = 0;
}
