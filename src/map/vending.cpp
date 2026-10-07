// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "vending.hpp"
#include <cstdlib> // atoi
#include <unordered_map>
#include <vector>

#include <common/malloc.hpp> // aMalloc, aFree
#include <common/nullpo.hpp>
#include <common/showmsg.hpp> // ShowInfo
#include <common/strlib.hpp>
#include <common/timer.hpp>  // DIFF_TICK
#include <common/sql.hpp>

#include "achievement.hpp"
#include "atcommand.hpp"
#include "battle.hpp"
#include "buyingstore.hpp" // struct s_autotrade_entry, struct s_autotrader
#include "chrif.hpp"
#include "clif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "mail.hpp"
#include "map.hpp"
#include "intif.hpp"
#include "npc.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"

static uint32 vending_nextid = 0; ///Vending_id counter
static DBMap *vending_db; ///DB holder the vender : charid -> map_session_data

//Autotrader
static DBMap *vending_autotrader_db; /// Holds autotrader info: char_id -> struct s_autotrader
static void vending_autotrader_remove(struct s_autotrader *at, bool remove);
static int32 vending_autotrader_free(DBKey key, DBData *data, va_list ap);

/**
 * Lookup to get the vending_db outside module
 * @return the vending_db
 */
DBMap * vending_getdb()
{
	return vending_db;
}

/**
 * Create an unique vending shop id.
 * @return the next vending_id
 */
static int32 vending_getuid(void)
{
	return ++vending_nextid;
}

/**
 * Make a player close his shop
 * @param sd : player session
 */
void vending_closevending(map_session_data* sd)
{
	nullpo_retv(sd);

	if( sd->state.vending ) {
		if( Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE vending_id = %d;", vending_items_table, sd->vender_id ) != SQL_SUCCESS ||
			Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE `id` = %d;", vendings_table, sd->vender_id ) != SQL_SUCCESS ) {
				Sql_ShowDebug(mmysql_handle);
		}

		sd->state.vending = false;
		sd->vender_id = 0;
		clif_closevendingboard( *sd, AREA_WOS, nullptr );
		idb_remove(vending_db, sd->status.char_id);
	}
}

/**
 * Player request a shop's item list (a player shop)
 * @param sd : player requestion the list
 * @param id : vender account id (gid)
 */
void vending_vendinglistreq(map_session_data* sd, int32 id)
{
	map_session_data* vsd;
	nullpo_retv(sd);

	if( (vsd = map_id2sd(id)) == nullptr )
		return;
	if( !vsd->state.vending )
		return; // not vending

	if (!pc_can_give_items(sd) || !pc_can_give_items(vsd)) { //check if both GMs are allowed to trade
		clif_displaymessage( sd->fd, msg_txt( sd, 246 ) ); // Your GM level doesn't authorize you to perform this action.
		return;
	}

	sd->vended_id = vsd->vender_id;  // register vending uid

	clif_vendinglist( *sd, *vsd );
}

/**
 * Calculates taxes for vending
 * @param sd: Vender
 * @param zeny: Total amount to tax
 * @return Total amount after taxes
 */
static double vending_calc_tax(map_session_data *sd, double zeny)
{
	if (battle_config.vending_tax && zeny >= battle_config.vending_tax_min)
		zeny -= zeny * (battle_config.vending_tax / 10000.);

	return zeny;
}

/**
 * Purchase item(s) from a shop
 * @param sd : buyer player session
 * @param aid : account id of vender
 * @param uid : shop unique id
 * @param data : items data who would like to purchase \n
 *	data := {<index>.w <amount>.w }[count]
 * @param count : number of different items he's trying to buy
 */
void vending_purchasereq(map_session_data* sd, int32 aid, int32 uid, const uint8* data, int32 count)
{
	int32 i, j, cursor, w, new_ = 0, blank, vend_list[MAX_VENDING];
	double z;
	struct s_vending vending[MAX_VENDING]; // against duplicate packets
	map_session_data* vsd = map_id2sd(aid);

	nullpo_retv(sd);
	if( vsd == nullptr || !vsd->state.vending || vsd->id == sd->id )
		return; // invalid shop

	if( vsd->vender_id != uid ) { // shop has changed
		clif_buyvending( *sd, 0, 0, PURCHASEMC_STORE_INCORRECT );  // store information was incorrect
		return;
	}

	if( !searchstore_queryremote(*sd, aid) && ( sd->m != vsd->m || !check_distance_bl(sd, vsd, AREA_SIZE) ) )
		return; // shop too far away

	searchstore_clearremote(*sd);

	if( count < 1 || count > MAX_VENDING || count > vsd->vend_num )
		return; // invalid amount of purchased items

	blank = pc_inventoryblank(sd); //number of free cells in the buyer's inventory

	// duplicate item in vending to check hacker with multiple packets
	memcpy(&vending, &vsd->vending, sizeof(vsd->vending)); // copy vending list

	// some checks
	z = 0.; // zeny counter
	w = 0;  // weight counter
	for( i = 0; i < count; i++ ) {
		int16 amount = *(uint16*)(data + 4*i + 0);
		int16 idx    = *(uint16*)(data + 4*i + 2);
		idx -= 2;

		if( amount <= 0 )
			return;

		// check of item index in the cart
		if( idx < 0 || idx >= MAX_CART )
			return;

		ARR_FIND( 0, vsd->vend_num, j, vsd->vending[j].index == idx );
		if( j == vsd->vend_num )
			return; //picked non-existing item
		else
			vend_list[i] = j;

		z += ((double)vsd->vending[j].value * (double)amount);
		if (z < 0. || z > (double)MAX_ZENY) return;
		const uint32 currency = vsd->vending_currency;
		if (currency == VENDING_CURRENCY_ZENY) {
			if (z > (double)sd->status.zeny) { clif_buyvending(*sd, idx, amount, PURCHASEMC_NO_ZENY); return; }
			if (!battle_config.vending_over_max && z + (double)vsd->status.zeny > (double)MAX_ZENY) { clif_buyvending(*sd, idx, vsd->vending[j].amount, PURCHASEMC_OUT_OF_STOCK); return; }
		} else if (currency == VENDING_CURRENCY_CASH) {
			if (z > sd->cashPoints) { clif_displaymessage(sd->fd, "Not enough Cash Points."); return; }
		} else if (currency == VENDING_CURRENCY_KAFRA) {
			if (z > sd->kafraPoints) { clif_displaymessage(sd->fd, "Not enough Kafra Points."); return; }
		} else {
			int64 have = 0;
			for (int32 k=0; k<MAX_INVENTORY; ++k) if (sd->inventory.u.items_inventory[k].nameid == currency && !sd->inventory.u.items_inventory[k].bound) have += sd->inventory.u.items_inventory[k].amount;
			if (z > have) { clif_displaymessage(sd->fd, "Not enough vending currency items."); return; }
			if (pc_checkadditem(vsd, currency, (int32)z) == CHKADDITEM_OVERAMOUNT) { clif_displaymessage(sd->fd, "Seller cannot receive that amount of currency."); return; }
			if (pc_inventoryblank(vsd) < 1 && pc_search_inventory(vsd, currency) < 0) { clif_displaymessage(sd->fd, "Seller has no inventory space for the currency."); return; }
		}
		w += itemdb_weight(vsd->cart.u.items_cart[idx].nameid) * amount;
		if( w + sd->weight > sd->max_weight ) {
			clif_buyvending( *sd, idx, amount, PURCHASEMC_OVERWEIGHT );
			return;
		}

		//Check to see if cart/vend info is in sync.
		if( vending[j].amount > vsd->cart.u.items_cart[idx].amount )
			vending[j].amount = vsd->cart.u.items_cart[idx].amount;

		// if they try to add packets (example: get twice or more 2 apples if marchand has only 3 apples).
		// here, we check cumulative amounts
		if( vending[j].amount < amount ) {
			// send more quantity is not a hack (an other player can have buy items just before)
			clif_buyvending( *sd, idx, vsd->vending[j].amount, PURCHASEMC_OUT_OF_STOCK );
			return;
		}

		vending[j].amount -= amount;

		switch( pc_checkadditem(sd, vsd->cart.u.items_cart[idx].nameid, amount) ) {
		case CHKADDITEM_EXIST:
			break;	//We'd add this item to the existing one (in buyers inventory)
		case CHKADDITEM_NEW:
			new_++;
			if (new_ > blank)
				return; //Buyer has no space in his inventory
			break;
		case CHKADDITEM_OVERAMOUNT:
			return; //too many items
		}
	}

	const uint32 currency = vsd->vending_currency;
	const int32 total_cost = (int32)z;
	if (currency == VENDING_CURRENCY_ZENY) {
		pc_payzeny(sd, total_cost, LOG_TYPE_VENDING, vsd->status.char_id);
		achievement_update_objective(sd, AG_SPEND_ZENY, 1, total_cost);
		z = vending_calc_tax(sd, z);
		pc_getzeny(vsd, (int32)z, LOG_TYPE_VENDING, sd->status.char_id);
	} else if (currency == VENDING_CURRENCY_CASH) {
		if (pc_paycash(sd, total_cost, 0, LOG_TYPE_VENDING) < 0) return;
		pc_getcash(vsd, total_cost, 0, LOG_TYPE_VENDING);
	} else if (currency == VENDING_CURRENCY_KAFRA) {
		if (pc_paycash(sd, total_cost, total_cost, LOG_TYPE_VENDING) < 0) return;
		pc_getcash(vsd, 0, total_cost, LOG_TYPE_VENDING);
	} else {
		int32 remaining = total_cost;
		for (int32 k = 0; k < MAX_INVENTORY && remaining > 0; ++k) {
			item& cur = sd->inventory.u.items_inventory[k];
			if (cur.nameid != currency || cur.bound) continue;
			int32 take = std::min<int32>(cur.amount, remaining);
			pc_delitem(sd, k, take, 0, 6, LOG_TYPE_VENDING);
			remaining -= take;
		}
		item payment{}; payment.nameid = currency; payment.identify = 1;
		if (remaining > 0 || pc_additem(vsd, &payment, total_cost, LOG_TYPE_VENDING) != ADDITEM_SUCCESS) return;
	}

	for( i = 0; i < count; i++ ) {
		int16 amount = *(uint16*)(data + 4*i + 0);
		int16 idx    = *(uint16*)(data + 4*i + 2);
		idx -= 2;
		z = 0.; // zeny counter

		// vending item
		pc_additem(sd, &vsd->cart.u.items_cart[idx], amount, LOG_TYPE_VENDING);
		vsd->vending[vend_list[i]].amount -= amount;
		z += ((double)vsd->vending[vend_list[i]].value * (double)amount);

		if( vsd->vending[vend_list[i]].amount ) {
			if( Sql_Query( mmysql_handle, "UPDATE `%s` SET `amount` = %d WHERE `vending_id` = %d and `cartinventory_id` = %d", vending_items_table, vsd->vending[vend_list[i]].amount, vsd->vender_id, vsd->cart.u.items_cart[idx].id ) != SQL_SUCCESS ) {
				Sql_ShowDebug( mmysql_handle );
			}
		} else {
			if( Sql_Query( mmysql_handle, "DELETE FROM `%s` WHERE `vending_id` = %d and `cartinventory_id` = %d", vending_items_table, vsd->vender_id, vsd->cart.u.items_cart[idx].id ) != SQL_SUCCESS ) {
				Sql_ShowDebug( mmysql_handle );
			}
		}

		pc_cart_delitem(vsd, idx, amount, 0, LOG_TYPE_VENDING);
		if (vsd->vending_currency == VENDING_CURRENCY_ZENY) z = vending_calc_tax(sd, z);
		clif_vendingreport( *vsd, idx, amount, sd->status.char_id, (int32)z );

		//print buyer's name
		if( battle_config.buyer_name ) {
			char temp[256];
			sprintf(temp, msg_txt(sd,265), sd->status.name);
			clif_messagecolor(vsd, color_table[COLOR_LIGHT_GREEN], temp, false, SELF);
		}
	}

	// compact the vending list
	for( i = 0, cursor = 0; i < vsd->vend_num; i++ ) {
		if( vsd->vending[i].amount == 0 )
			continue;

		if( cursor != i ) { // speedup
			vsd->vending[cursor].index = vsd->vending[i].index;
			vsd->vending[cursor].amount = vsd->vending[i].amount;
			vsd->vending[cursor].value = vsd->vending[i].value;
		}

		cursor++;
	}

	vsd->vend_num = cursor;

	//Always save BOTH: customer (buyer) and vender
	if( save_settings&CHARSAVE_VENDING ) {
		chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART);
		chrif_save(vsd, CSAVE_INVENTORY|CSAVE_CART);
	}

	//check for @AUTOTRADE users [durf]
	if( vsd->state.autotrade ) {
		//see if there is anything left in the shop
		ARR_FIND( 0, vsd->vend_num, i, vsd->vending[i].amount > 0 );
		if( i == vsd->vend_num ) {
			//Close Vending (this was automatically done by the client, we have to do it manually for autovenders) [Skotlex]
			vending_closevending(vsd);
			map_quit(vsd);	//They have no reason to stay around anymore, do they?
		}
	}
}

/**
 * Player setup a new shop
 * @param sd : player opening the shop
 * @param message : shop title
 * @param data : itemlist data
 *	data := {<index>.w <amount>.w <value>.l}[count]
 * @param count : number of different items
 * @param at Autotrader info, or nullptr if requetsed not from autotrade persistance
 * @return 0 If success, 1 - Cannot open (die, not state.prevend, trading), 2 - No cart, 3 - Count issue, 4 - Cart data isn't saved yet, 5 - No valid item found
 */
int8 vending_openvending( map_session_data& sd, const char* message, const uint8* data, int32 count, struct s_autotrader *at ){
	int32 i, j;
	int32 vending_skill_lvl;
	char message_sql[MESSAGE_SIZE*2];
	StringBuf buf;

	if ( pc_isdead(&sd) || !sd.state.prevend || pc_istrading(&sd)) {
		return 1; // can't open vendings lying dead || didn't use via the skill (wpe/hack) || can't have 2 shops at once
	}

	vending_skill_lvl = pc_checkskill(&sd, MC_VENDING);
	
	// skill level and cart check
	if( !vending_skill_lvl || !pc_iscarton(&sd) ) {
		clif_skill_fail( sd, MC_VENDING );
		sd.state.prevend = 0;
		sd.state.workinprogress = WIP_DISABLE_NONE;
		clif_openvending_ack( sd, OPENSTORE2_FAILED );
		return 2;
	}

	// check number of items in shop
	if( count < 1 || count > MAX_VENDING || count > 2 + vending_skill_lvl ) { // invalid item count
		clif_skill_fail( sd, MC_VENDING );
		sd.state.prevend = 0;
		sd.state.workinprogress = WIP_DISABLE_NONE;
		clif_openvending_ack( sd, OPENSTORE2_FAILED );
		return 3;
	}

	if (save_settings&CHARSAVE_VENDING) // Avoid invalid data from saving
		chrif_save(&sd, CSAVE_INVENTORY|CSAVE_CART);

	// filter out invalid items
	i = 0;
	int64 total = 0;
	for( j = 0; j < count; j++ ) {
		int16 index        = *(uint16*)(data + 8*j + 0);
		int16 amount       = *(uint16*)(data + 8*j + 2);
		uint32 value       = *(uint32*)(data + 8*j + 4);

		index -= 2; // offset adjustment (client says that the first cart position is 2)

		if( index < 0 || index >= MAX_CART // invalid position
		||  pc_cartitem_amount(&sd, index, amount) < 0 // invalid item or insufficient quantity
		//NOTE: official server does not do any of the following checks!
		||  !sd.cart.u.items_cart[index].identify // unidentified item
		||  sd.cart.u.items_cart[index].attribute == 1 // broken item
		||  (pc_durability_eligible(itemdb_search(sd.cart.u.items_cart[index].nameid)) && !pc_durability_full(sd.cart.u.items_cart[index])) // damaged durability
		||  sd.cart.u.items_cart[index].expire_time // It should not be in the cart but just in case
		||  (sd.cart.u.items_cart[index].bound && !pc_can_give_bounded_items(&sd)) // can't trade account bound items and has no permission
		||  !itemdb_cantrade(&sd.cart.u.items_cart[index], pc_get_group_level(&sd), pc_get_group_level(&sd)) ) // untradeable item
			continue;

		sd.vending[i].index = index;
		sd.vending[i].amount = amount;
		sd.vending[i].value = min(value, (uint32)battle_config.vending_max_value);
		total += static_cast<int64>(sd.vending[i].value) * amount;
		i++; // item successfully added
	}

	// check if the total value of the items plus the current zeny is over the limit
	if ( !battle_config.vending_over_max && (static_cast<int64>(sd.status.zeny) + total) > MAX_ZENY ) {
#if PACKETVER >= 20200819
		clif_msg_color( sd, MSI_MERCHANTSHOP_TOTA_LOVER_ZENY_ERR, color_table[COLOR_RED] );
#endif
		clif_skill_fail( sd, MC_VENDING );
		sd.state.prevend = 0;
		sd.state.workinprogress = WIP_DISABLE_NONE;
		clif_openvending_ack( sd, OPENSTORE2_FAILED );
		return 1;
	}

	if (i != j) {
		clif_displaymessage(sd.fd, msg_txt(&sd, 266)); //"Some of your items cannot be vended and were removed from the shop."
		clif_skill_fail( sd, MC_VENDING ); // custom reply packet
		sd.state.prevend = 0;
		sd.state.workinprogress = WIP_DISABLE_NONE;
		clif_openvending_ack( sd, OPENSTORE2_FAILED );
		return 5;
	}

	if( i == 0 ) { // no valid item found
		clif_skill_fail( sd, MC_VENDING ); // custom reply packet
		sd.state.prevend = 0;
		sd.state.workinprogress = WIP_DISABLE_NONE;
		clif_openvending_ack( sd, OPENSTORE2_FAILED );
		return 5;
	}

	sd.state.prevend = 0;
	sd.state.vending = true;
	sd.state.workinprogress = WIP_DISABLE_NONE;
	sd.vender_id = vending_getuid();
	sd.vend_num = i;
	safestrncpy(sd.message, message, MESSAGE_SIZE);
	
	Sql_EscapeString( mmysql_handle, message_sql, sd.message );

	if( Sql_Query( mmysql_handle, "INSERT INTO `%s`(`id`, `account_id`, `char_id`, `sex`, `map`, `x`, `y`, `title`, `autotrade`, `body_direction`, `head_direction`, `sit`, `extended_vending_item`) "
		"VALUES( %d, %d, %d, '%c', '%s', %d, %d, '%s', %d, '%d', '%d', '%d', %u );",
		vendings_table, sd.vender_id, sd.status.account_id, sd.status.char_id, sd.status.sex == SEX_FEMALE ? 'F' : 'M', map_getmapdata(sd.m)->name, sd.x, sd.y, message_sql, sd.state.autotrade, at ? at->dir : sd.ud.dir, at ? at->head_dir : sd.head_dir, at ? at->sit : pc_issit(&sd), at ? at->vending_currency : sd.vending_currency ) != SQL_SUCCESS ) {
		Sql_ShowDebug(mmysql_handle);
	}

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s`(`vending_id`,`index`,`cartinventory_id`,`amount`,`price`) VALUES", vending_items_table);
	for (j = 0; j < i; j++) {
		StringBuf_Printf(&buf, "(%d,%d,%d,%d,%d)", sd.vender_id, j, sd.cart.u.items_cart[sd.vending[j].index].id, sd.vending[j].amount, sd.vending[j].value);
		if (j < i-1)
			StringBuf_AppendStr(&buf, ",");
	}
	if (SQL_ERROR == Sql_QueryStr(mmysql_handle, StringBuf_Value(&buf)))
		Sql_ShowDebug(mmysql_handle);

	clif_openvending( sd );
	clif_showvendingboard( sd );

	idb_put(vending_db, sd.status.char_id, &sd);

	return 0;
}

/**
 * Checks if an item is being sold in given player's vending.
 * @param sd : vender session (player)
 * @param nameid : item id
 * @return 0:not selling it, 1: yes
 */
bool vending_search( const map_session_data* sd, t_itemid nameid )
{
	int32 i;

	if( !sd->state.vending ) { // not vending
		return false;
	}

	ARR_FIND( 0, sd->vend_num, i, sd->cart.u.items_cart[sd->vending[i].index].nameid == nameid );
	if( i == sd->vend_num ) { // not found
		return false;
	}

	return true;
}

/**
 * Searches for all items in a vending, that match given ids, price and possible cards.
 * @param sd : The vender session to search into
 * @param s : parameter of the search (see s_search_store_search)
 * @return Whether or not the search should be continued.
 */
bool vending_searchall( const map_session_data* sd, const struct s_search_store_search* s )
{
	int32 i, c, slot;
	uint32 idx, cidx;
	const item* it;

	if( !sd->state.vending ) // not vending
		return true;

	for( idx = 0; idx < s->item_count; idx++ ) {
		ARR_FIND( 0, sd->vend_num, i, sd->cart.u.items_cart[sd->vending[i].index].nameid == s->itemlist[idx].itemId );
		if( i == sd->vend_num ) { // not found
			continue;
		}
		it = &sd->cart.u.items_cart[sd->vending[i].index];

		if( s->min_price && s->min_price > sd->vending[i].value ) { // too low price
			continue;
		}

		if( s->max_price && s->max_price < sd->vending[i].value ) { // too high price
			continue;
		}

		if( s->card_count ) { // check cards
			if( itemdb_isspecial(it->card[0]) ) { // something, that is not a carded
				continue;
			}
			slot = itemdb_slots(it->nameid);

			for( c = 0; c < slot && it->card[c]; c ++ ) {
				ARR_FIND( 0, s->card_count, cidx, s->cardlist[cidx].itemId == it->card[c] );
				if( cidx != s->card_count ) { // found
					break;
				}
			}

			if( c == slot || !it->card[c] ) { // no card match
				continue;
			}
		}

		// Check if the result set is full
		if( s->search_sd->searchstore.items.size() >= (uint32)battle_config.searchstore_maxresults ){
			return false;
		}

		std::shared_ptr<s_search_store_info_item> ssitem = std::make_shared<s_search_store_info_item>();

		ssitem->store_id = sd->vender_id;
		ssitem->account_id = sd->status.account_id;
		// FreokRO Black Market: expose the physical shop location in every result.
		snprintf(ssitem->store_name, sizeof(ssitem->store_name), "%s [%s %d,%d]",
			sd->message, map_getmapdata(sd->m)->name, sd->x, sd->y);
		ssitem->nameid = it->nameid;
		ssitem->amount = sd->vending[i].amount;
		ssitem->price = sd->vending[i].value;
		for( int32 j = 0; j < MAX_SLOTS; j++ ){
			ssitem->card[j] = it->card[j];
		}
		ssitem->refine = it->refine;
		ssitem->enchantgrade = it->enchantgrade;

		s->search_sd->searchstore.items.push_back( ssitem );
	}

	return true;
}

/**
* Open vending for Autotrader
* @param sd Player as autotrader
*/
void vending_reopen( map_session_data& sd )
{
	struct s_autotrader *at = nullptr;
	int8 fail = -1;

	// Open vending for this autotrader
	if ((at = (struct s_autotrader *)uidb_get(vending_autotrader_db, sd.status.char_id)) && at->count && at->entries) {
		uint8 *data, *p;
		uint16 j, count;

		// Init vending data for autotrader
		CREATE(data, uint8, at->count * 8);

		for (j = 0, p = data, count = at->count; j < at->count; j++) {
			struct s_autotrade_entry *entry = at->entries[j];
			uint16 *index = (uint16*)(p + 0);
			uint16 *amount = (uint16*)(p + 2);
			uint32 *value = (uint32*)(p + 4);

			// Find item position in cart
			ARR_FIND(0, MAX_CART, entry->index, sd.cart.u.items_cart[entry->index].id == entry->cartinventory_id);

			if (entry->index == MAX_CART) {
				count--;
				continue;
			}

			*index = entry->index + 2;
			*amount = itemdb_isstackable(sd.cart.u.items_cart[entry->index].nameid) ? entry->amount : 1;
			*value = entry->price;

			p += 8;
		}

		sd.state.prevend = 1; // Set him into a hacked prevend state
		sd.state.autotrade = 1;

		// Make sure abort all NPCs
		npc_event_dequeue(&sd);
		pc_cleareventtimer(&sd);

		// Open the vending again
		if( (fail = vending_openvending(sd, at->title, data, count, at)) == 0 ) {
			// Make vendor look perfect
			pc_setdir(&sd, at->dir, at->head_dir);
			clif_changed_dir(sd, AREA_WOS);
			if( at->sit ) {
				pc_setsit(&sd);
				skill_sit(&sd, 1);
				clif_sitting(sd);
			}

			// Immediate save
			chrif_save(&sd, CSAVE_AUTOTRADE);

			ShowInfo("Vending loaded for '" CL_WHITE "%s" CL_RESET "' with '" CL_WHITE "%d" CL_RESET "' items at " CL_WHITE "%s (%d,%d)" CL_RESET "\n",
				sd.status.name, count, mapindex_id2name(sd.mapindex), sd.x, sd.y);
		}
		aFree(data);
	}

	if (at) {
		vending_autotrader_remove(at, true);
		if (db_size(vending_autotrader_db) == 0)
			vending_autotrader_db->clear(vending_autotrader_db, vending_autotrader_free);
	}

	if (fail != 0) {
		ShowError("vending_reopen: (Error:%d) Load failed for autotrader '" CL_WHITE "%s" CL_RESET "' (CID=%d/AID=%d)\n", fail, sd.status.name, sd.status.char_id, sd.status.account_id);
		map_quit(&sd);
	}
}

/**
* Initializing autotraders from table
*/
void do_init_vending_autotrade(void)
{
	if (battle_config.feature_autotrade) {
		if (Sql_Query(mmysql_handle,
			"SELECT `id`, `account_id`, `char_id`, `sex`, `title`, `body_direction`, `head_direction`, `sit`, `extended_vending_item` "
			"FROM `%s` "
			"WHERE `autotrade` = 1 AND (SELECT COUNT(`vending_id`) FROM `%s` WHERE `vending_id` = `id`) > 0 "
			"ORDER BY `id`;",
			vendings_table, vending_items_table ) != SQL_SUCCESS )
		{
			Sql_ShowDebug(mmysql_handle);
			return;
		}

		if( Sql_NumRows(mmysql_handle) > 0 ) {
			uint16 items = 0;
			DBIterator *iter = nullptr;
			struct s_autotrader *at = nullptr;

			// Init each autotrader data
			while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
				size_t len;
				char* data;

				at = nullptr;
				CREATE(at, struct s_autotrader, 1);
				Sql_GetData(mmysql_handle, 0, &data, nullptr); at->id = atoi(data);
				Sql_GetData(mmysql_handle, 1, &data, nullptr); at->account_id = atoi(data);
				Sql_GetData(mmysql_handle, 2, &data, nullptr); at->char_id = atoi(data);
				Sql_GetData(mmysql_handle, 3, &data, nullptr); at->sex = (data[0] == 'F') ? SEX_FEMALE : SEX_MALE;
				Sql_GetData(mmysql_handle, 4, &data, &len); safestrncpy(at->title, data, zmin(len + 1, MESSAGE_SIZE));
				Sql_GetData(mmysql_handle, 5, &data, nullptr); at->dir = atoi(data);
				Sql_GetData(mmysql_handle, 6, &data, nullptr); at->head_dir = atoi(data);
				Sql_GetData(mmysql_handle, 7, &data, nullptr); at->sit = atoi(data);
				Sql_GetData(mmysql_handle, 8, &data, nullptr); at->vending_currency = strtoul(data, nullptr, 10);
				at->count = 0;

				if (battle_config.feature_autotrade_direction >= 0)
					at->dir = battle_config.feature_autotrade_direction;
				if (battle_config.feature_autotrade_head_direction >= 0)
					at->head_dir = battle_config.feature_autotrade_head_direction;
				if (battle_config.feature_autotrade_sit >= 0)
					at->sit = battle_config.feature_autotrade_sit;

				// initialize player
				CREATE(at->sd, map_session_data, 1); // TODO: Dont use Memory Manager allocation anymore and rely on the C++ container
				new (at->sd) map_session_data();
				pc_setnewpc(at->sd, at->account_id, at->char_id, 0, gettick(), at->sex, 0);
				at->sd->state.autotrade = 1|2;
				at->sd->vending_currency = at->vending_currency;
				if (battle_config.autotrade_monsterignore)
					at->sd->state.block_action |= PCBLOCK_IMMUNE;
				else
					at->sd->state.block_action &= ~PCBLOCK_IMMUNE;
				chrif_authreq(at->sd, true);
				uidb_put(vending_autotrader_db, at->char_id, at);
			}
			Sql_FreeResult(mmysql_handle);

			// Init items for each autotraders
			iter = db_iterator(vending_autotrader_db);
			for (at = (struct s_autotrader *)dbi_first(iter); dbi_exists(iter); at = (struct s_autotrader *)dbi_next(iter)) {
				uint16 j = 0;

				if (SQL_ERROR == Sql_Query(mmysql_handle,
					"SELECT `cartinventory_id`, `amount`, `price` "
					"FROM `%s` "
					"WHERE `vending_id` = %d "
					"ORDER BY `index` ASC;",
					vending_items_table, at->id ) )
				{
					Sql_ShowDebug(mmysql_handle);
					continue;
				}

				if (!(at->count = (uint16)Sql_NumRows(mmysql_handle))) {
					map_quit(at->sd);
					vending_autotrader_remove(at, true);
					continue;
				}

				//Init the list
				CREATE(at->entries, struct s_autotrade_entry *, at->count);

				//Add the item into list
				j = 0;
				while (SQL_SUCCESS == Sql_NextRow(mmysql_handle) && j < at->count) {
					char *data;
					CREATE(at->entries[j], struct s_autotrade_entry, 1);
					Sql_GetData(mmysql_handle, 0, &data, nullptr); at->entries[j]->cartinventory_id = atoi(data);
					Sql_GetData(mmysql_handle, 1, &data, nullptr); at->entries[j]->amount = atoi(data);
					Sql_GetData(mmysql_handle, 2, &data, nullptr); at->entries[j]->price = atoi(data);
					j++;
				}
				items += j;
				Sql_FreeResult(mmysql_handle);
			}
			dbi_destroy(iter);

			ShowStatus("Done loading '" CL_WHITE "%d" CL_RESET "' vending autotraders with '" CL_WHITE "%d" CL_RESET "' items.\n", db_size(vending_autotrader_db), items);
		}
	}

	// Everything is loaded fine, their entries will be reinserted once they are loaded
	if (Sql_Query( mmysql_handle, "DELETE FROM `%s`;", vendings_table ) != SQL_SUCCESS ||
		Sql_Query( mmysql_handle, "DELETE FROM `%s`;", vending_items_table ) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
	}
}

/**
 * Remove an autotrader's data
 * @param at Autotrader
 * @param remove If true will removes from vending_autotrader_db
 **/
static void vending_autotrader_remove(struct s_autotrader *at, bool remove) {
	nullpo_retv(at);
	if (at->count && at->entries) {
		uint16 i = 0;
		for (i = 0; i < at->count; i++) {
			if (at->entries[i])
				aFree(at->entries[i]);
		}
		aFree(at->entries);
	}
	if (remove)
		uidb_remove(vending_autotrader_db, at->char_id);
	aFree(at);
}

/**
* Clear all autotraders
* @author [Cydh]
*/
static int32 vending_autotrader_free(DBKey key, DBData *data, va_list ap) {
	struct s_autotrader *at = (struct s_autotrader *)db_data2ptr(data);
	if (at)
		vending_autotrader_remove(at, false);
	return 0;
}

/**
* Update vendor location
* @param sd: Player's session data
*/
void vending_update(map_session_data &sd)
{
	if (Sql_Query(mmysql_handle, "UPDATE `%s` SET `map` = '%s', `x` = '%d', `y` = '%d', `body_direction` = '%d', `head_direction` = '%d', `sit` = '%d', `autotrade` = '%d', `extended_vending_item` = '%u' WHERE `id` = '%d'",
		vendings_table, map_getmapdata(sd.m)->name, sd.x, sd.y, sd.ud.dir, sd.head_dir, pc_issit(&sd), sd.state.autotrade, sd.vending_currency,
		sd.vender_id
	) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
	}
}

static std::unordered_map<uint32, struct npc_data*> active_vending_clones;

// Returns an active persistent vending clone by its NPC runtime id.
// Search Store uses the NPC id as both account/store id for clone results so
// clicks can be routed back to the clone instead of map_id2sd().
struct npc_data* vending_clone_by_npc_id(int32 npc_id) {
	for (const auto& entry : active_vending_clones) {
		struct npc_data* nd = entry.second;
		if (nd != nullptr && nd->subtype == NPCTYPE_VENDING_CLONE && nd->id == npc_id)
			return nd;
	}
	return nullptr;
}

// Adds Market Clone (@autotrade2) shops to the native Search Store result set.
// Normal/autotrade player vending is already covered by vending_searchall().
bool vending_clone_searchall(const struct s_search_store_search* s) {
	if (s == nullptr || s->search_sd == nullptr)
		return true;

	for (const auto& entry : active_vending_clones) {
		struct npc_data* nd = entry.second;
		if (nd == nullptr || nd->subtype != NPCTYPE_VENDING_CLONE || nd->u.vending_clone.items == nullptr)
			continue;

		// Respect the map scope selected by Search Store. mapid == 0 means all maps.
		if (s->search_sd->searchstore.mapid != 0 && nd->m != s->search_sd->searchstore.mapid)
			continue;

		for (uint32 q = 0; q < s->item_count; ++q) {
			for (const struct s_vending_clone_item& vci : *nd->u.vending_clone.items) {
				const struct item& it = vci.item_data;
				if (vci.amount <= 0 || it.nameid != s->itemlist[q].itemId)
					continue;
				if (s->min_price && s->min_price > vci.price)
					continue;
				if (s->max_price && s->max_price < vci.price)
					continue;

				if (s->card_count) {
					if (itemdb_isspecial(it.card[0]))
						continue;
					int32 slot = itemdb_slots(it.nameid);
					bool matched = false;
					for (int32 c = 0; c < slot && it.card[c]; ++c) {
						for (uint32 ci = 0; ci < s->card_count; ++ci) {
							if (s->cardlist[ci].itemId == it.card[c]) {
								matched = true;
								break;
							}
						}
						if (matched) break;
					}
					if (!matched)
						continue;
				}

				if (s->search_sd->searchstore.items.size() >= (uint32)battle_config.searchstore_maxresults)
					return false;

				auto ssitem = std::make_shared<s_search_store_info_item>();
				ssitem->store_id = nd->id;
				ssitem->account_id = nd->id;
				// FreokRO Black Market: persistent clones also publish map + coordinates.
				snprintf(ssitem->store_name, sizeof(ssitem->store_name), "%s [%s %d,%d]",
					nd->u.vending_clone.title, map_getmapdata(nd->m)->name, nd->x, nd->y);
				ssitem->nameid = it.nameid;
				ssitem->amount = vci.amount;
				ssitem->price = vci.price;
				for (int32 j = 0; j < MAX_SLOTS; ++j)
					ssitem->card[j] = it.card[j];
				ssitem->refine = it.refine;
				ssitem->enchantgrade = it.enchantgrade;
				s->search_sd->searchstore.items.push_back(ssitem);
			}
		}
	}
	return true;
}


static int vending_clone_db_cleanup_timer(int tid, int64 tick, int id, intptr_t data) {
	uint32 clone_id = (uint32)data;
	int64 t1 = gettick();
	// Use simple single-table DELETEs using the primary key to avoid MySQL table scans and locks.
	// Multi-table DELETE with LEFT JOIN can block the main thread for seconds.
	if (Sql_Query(mmysql_handle, "DELETE FROM `vending_clone_items` WHERE `clone_id` = %d", clone_id) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
	}
	int64 t2 = gettick();
	if (Sql_Query(mmysql_handle, "DELETE FROM `vending_clones` WHERE `clone_id` = %d", clone_id) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
	}
	int64 t3 = gettick();
	
	if (t3 - t1 > 50) {
		ShowInfo("VENDING CLONE DB CLEANUP: Took %d ms (Items: %d ms, Clones: %d ms)\n", (int)(t3 - t1), (int)(t2 - t1), (int)(t3 - t2));
	}
	return 0;
}

void vending_clone_delete(struct npc_data* nd) {
	if (!nd || nd->subtype != NPCTYPE_VENDING_CLONE) return;
	
	// Defer DB deletion to a timer to avoid blocking the main loop with SQL
	add_timer(gettick() + 100, vending_clone_db_cleanup_timer, 0, nd->u.vending_clone.clone_id);
	
	if (nd->u.vending_clone.items) {
		delete nd->u.vending_clone.items;
		nd->u.vending_clone.items = nullptr;
	}
	
	active_vending_clones.erase(nd->u.vending_clone.char_id);
	
	// Removed clif_closevendingboard here because ZC_NOTIFY_VANISH handles it automatically.
	
	// Safe NPC removal using the exact same function as @unloadnpc
	// CLR_OUTSIGHT prevents the client from playing a teleport animation
	// which was the cause of the "stuttering while walking" issue.
	npc_unload(nd, true, CLR_OUTSIGHT);
}



void vending_create_clone(map_session_data* sd) {
	if (!sd || !sd->state.vending) return;

	char esc_name[NAME_LENGTH*2+1];
	char esc_title[MESSAGE_SIZE*2+1];
	Sql_EscapeString(mmysql_handle, esc_name, sd->status.name);
	Sql_EscapeString(mmysql_handle, esc_title, sd->message);

	if (Sql_Query(mmysql_handle, 
		"INSERT INTO `vending_clones` (`char_id`, `name`, `title`, `map`, `x`, `y`, `sex`, `class`, `hair`, `hair_color`, `head_top`, `head_mid`, `head_bottom`, `robe`, `weapon`, `shield`, `body`, `currency`) "
		"VALUES (%d, '%s', '%s', '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
		sd->status.char_id, esc_name, esc_title, mapindex_id2name(sd->mapindex), sd->x, sd->y,
		sd->status.sex, sd->status.class_, sd->status.hair, sd->status.hair_color, sd->status.head_top, 
		sd->status.head_mid, sd->status.head_bottom, sd->status.robe, 
		sd->status.weapon, sd->status.shield, sd->status.clothes_color, sd->vending_currency) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
		return;
	}
	
	uint32 clone_id = (uint32)Sql_LastInsertId(mmysql_handle);
	std::vector<struct s_vending_clone_item>* clone_items = new std::vector<struct s_vending_clone_item>();

	for(int i = 0; i < sd->vend_num; i++) {
		int idx = sd->vending[i].index;
		struct item* it = &sd->cart.u.items_cart[idx];
		if (it->nameid == 0) continue;
		
		struct s_vending_clone_item vci;
		vci.item_data = *it;
		vci.amount = sd->vending[i].amount;
		vci.price = sd->vending[i].value;
		clone_items->push_back(vci);
		
		if (Sql_Query(mmysql_handle, 
			"INSERT INTO `vending_clone_items` (`clone_id`, `index`, `nameid`, `amount`, `price`, `refine`, `attribute`, `identify`, `expire_time`, `bound`, `unique_id`, `enchantgrade`, `durability`, `card0`, `card1`, `card2`, `card3`, `opt_idx0`, `opt_val0`, `opt_parm0`, `opt_idx1`, `opt_val1`, `opt_parm1`, `opt_idx2`, `opt_val2`, `opt_parm2`, `opt_idx3`, `opt_val3`, `opt_parm3`, `opt_idx4`, `opt_val4`, `opt_parm4`) "
			"VALUES (%d, %d, %d, %d, %d, %d, %d, %d, %u, %d, %" PRIu64 ", %d, %u, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
			clone_id, i, it->nameid, sd->vending[i].amount, sd->vending[i].value,
			it->refine, it->attribute, it->identify, it->expire_time, it->bound, it->unique_id, it->enchantgrade, it->durability, it->card[0], it->card[1], it->card[2], it->card[3],
			it->option[0].id, it->option[0].value, it->option[0].param, it->option[1].id, it->option[1].value, it->option[1].param,
			it->option[2].id, it->option[2].value, it->option[2].param, it->option[3].id, it->option[3].value, it->option[3].param,
			it->option[4].id, it->option[4].value, it->option[4].param) != SQL_SUCCESS) {
			Sql_ShowDebug(mmysql_handle);
		}
		
		// Remove item from real player's cart
		pc_cart_delitem(sd, idx, sd->vending[i].amount, 0, LOG_TYPE_VENDING);
	}
	
	clif_openvending_ack(*sd, OPENSTORE2_NOVENDING); // Try to force the client to close the active shop window
	vending_closevending(sd);
	clif_closevendingboard(*sd, SELF, sd);

	npc_data *nd = nullptr;
	CREATE(nd, npc_data, 1);
	new (nd) npc_data();

	nd->id = npc_get_new_npc_id();
	nd->type = BL_NPC;
	nd->prev = nd->next = nullptr;
	nd->m = sd->m;
	nd->x = sd->x;
	nd->y = sd->y;
	nd->dynamicnpc.removal_tid = INVALID_TIMER;

	if (!nd) {
		delete clone_items;
		return;
	}

	safestrncpy(nd->name, sd->status.name, sizeof(nd->name));
	safestrncpy(nd->exname, sd->status.name, sizeof(nd->exname));
	nd->class_ = sd->status.class_;
	nd->speed = sd->battle_status.speed;
	nd->vd.sex = sd->status.sex;
	nd->vd.look[LOOK_BASE] = sd->status.class_;
	nd->vd.look[LOOK_HAIR] = sd->status.hair;
	nd->vd.look[LOOK_HAIR_COLOR] = sd->status.hair_color;
	nd->vd.look[LOOK_HEAD_TOP] = sd->status.head_top;
	nd->vd.look[LOOK_HEAD_MID] = sd->status.head_mid;
	nd->vd.look[LOOK_HEAD_BOTTOM] = sd->status.head_bottom;
	nd->vd.look[LOOK_ROBE] = sd->status.robe;
	nd->vd.look[LOOK_WEAPON] = sd->status.weapon;
	nd->vd.look[LOOK_SHIELD] = sd->status.shield;
	nd->vd.look[LOOK_CLOTHES_COLOR] = sd->status.clothes_color;
	nd->vd.dead_sit = 2;

	nd->subtype = NPCTYPE_VENDING_CLONE;
	nd->u.vending_clone.clone_id = clone_id;
	nd->u.vending_clone.char_id = sd->status.char_id;
	safestrncpy(nd->u.vending_clone.title, sd->message, sizeof(nd->u.vending_clone.title));
	nd->u.vending_clone.currency = sd->vending_currency;
	nd->u.vending_clone.items = clone_items;

	map_addnpc(sd->m, nd); map_addiddb(nd);
	if (map_addblock(nd) != 0) return;
	
	clif_spawn(nd);
	clif_showvendingboard_clone(*nd);
	
	active_vending_clones[sd->status.char_id] = nd;
	
	vending_closevending(sd);
}

void vending_clone_purchase(map_session_data* sd, struct npc_data* nd, const uint8* data, int32 count) {
	if (!battle_config.funk_master_enable || !battle_config.funk_market_clone) { if(sd) clif_displaymessage(sd->fd,"[FUNK] Market Clone desativado."); return; }
	if (!sd || !nd || nd->subtype != NPCTYPE_VENDING_CLONE || count <= 0) return;
	if (!pc_can_give_items(sd)) return;
	
	std::vector<struct s_vending_clone_item>* items = nd->u.vending_clone.items;
	if (!items || items->empty()) return;
	
	int32 currency = nd->u.vending_clone.currency;
	int32 total_cost = 0;
	int32 new_amount = 0;
	int32 w = 0;
	int blank = pc_inventoryblank(sd);
	int new_ = 0;
	
	// Validation
	for (int i = 0; i < count; i++) {
		int16 amount = *(uint16*)(data + 4*i);
		int16 idx    = *(uint16*)(data + 4*i + 2);
		idx -= 2;
		
		if (idx < 0 || idx >= (int16)items->size() || amount <= 0) return;
		struct s_vending_clone_item& it = items->at(idx);
		if (it.amount < amount) return;
		
		total_cost += amount * it.price;
		new_amount += amount;
		w += itemdb_weight(it.item_data.nameid) * amount;
		
		switch( pc_checkadditem(sd, it.item_data.nameid, amount) ) {
		case CHKADDITEM_EXIST:
			break;
		case CHKADDITEM_NEW:
			new_++;
			if (new_ > blank) return;
			break;
		case CHKADDITEM_OVERAMOUNT:
			return;
		}
	}
	
	if (total_cost < 0 || new_amount <= 0 || new_amount > MAX_AMOUNT) return;
	if ((sd->weight + w) > sd->max_weight) return;
	
	if (currency == VENDING_CURRENCY_ZENY) {
		if (sd->status.zeny < total_cost) {
			clif_displaymessage(sd->fd, "Not enough Zeny.");
			return;
		}
	} else if (currency == VENDING_CURRENCY_CASH) {
		if (sd->cashPoints < total_cost) { clif_displaymessage(sd->fd, "Not enough Cash Points."); return; }
	} else if (currency == VENDING_CURRENCY_KAFRA) {
		if (sd->kafraPoints < total_cost) { clif_displaymessage(sd->fd, "Not enough Kafra Points."); return; }
	} else {
		int32 loot_count = 0;
		for (int k = 0; k < MAX_INVENTORY; k++) {
			if (sd->inventory.u.items_inventory[k].nameid == currency) {
				if (sd->inventory.u.items_inventory[k].bound)
					continue;
				loot_count += sd->inventory.u.items_inventory[k].amount;
			}
		}

		if (loot_count < total_cost) {
			clif_displaymessage(sd->fd, "Not enough currency items.");
			return;
		}
	}
	
	if (pc_inventoryblank(sd) < count) {
		clif_displaymessage(sd->fd, msg_txt(sd, 18));
		return;
	}
	
	// Process
	if (currency == VENDING_CURRENCY_ZENY) {
		pc_payzeny(sd, total_cost, LOG_TYPE_VENDING);
	} else if (currency == VENDING_CURRENCY_CASH) {
		if (pc_paycash(sd, total_cost, 0, LOG_TYPE_VENDING) < 0) return;
	} else if (currency == VENDING_CURRENCY_KAFRA) {
		if (pc_paycash(sd, total_cost, total_cost, LOG_TYPE_VENDING) < 0) return;
	} else {
		int32 remaining_z = total_cost;
		for (int i = 0; i < MAX_INVENTORY && remaining_z > 0; i++) {
			if (sd->inventory.u.items_inventory[i].nameid == currency) {
				if (sd->inventory.u.items_inventory[i].bound)
					continue;
				int32 amount_to_del = min(sd->inventory.u.items_inventory[i].amount, remaining_z);
				pc_delitem(sd, i, amount_to_del, 0, 6, LOG_TYPE_VENDING);
				remaining_z -= amount_to_del;
			}
		}
	}
	
	// Send mail to owner
	struct mail_message msg;
	memset(&msg, 0, sizeof(msg));
	msg.dest_id = nd->u.vending_clone.char_id;
	msg.timestamp = time(nullptr);
	
	if (currency == VENDING_CURRENCY_ZENY) {
		int32 tax_rate = battle_config.vending_tax;
		int32 tax_amount = (tax_rate && total_cost >= battle_config.vending_tax_min) ? (int32)((double)total_cost * ((double)tax_rate / 10000.)) : 0;
		int32 profit = total_cost - tax_amount;
		
		msg.zeny = profit;
		snprintf(msg.title, sizeof(msg.title), "Vending Clone Zeny");
		snprintf(msg.body, sizeof(msg.body), "Your Clone sold %d items for %d Zeny.\nTax paid: %d Zeny.\nTotal profit: %d Zeny.", new_amount, total_cost, tax_amount, profit);
	} else if (currency == VENDING_CURRENCY_CASH || currency == VENDING_CURRENCY_KAFRA) {
		// Cash/Kafra points cannot be attached to RodEx as point balances. Persist payout for delivery on seller login.
		if (Sql_Query(mmysql_handle, "INSERT INTO `vending_clone_point_payouts` (`char_id`,`currency`,`amount`) VALUES (%d,%u,%d) ON DUPLICATE KEY UPDATE `amount`=`amount`+VALUES(`amount`)", nd->u.vending_clone.char_id, currency, total_cost) != SQL_SUCCESS) Sql_ShowDebug(mmysql_handle);
		msg.item[0].nameid = 0; msg.item[0].amount = 0;
		snprintf(msg.title, sizeof(msg.title), "Vending Clone Points");
		snprintf(msg.body, sizeof(msg.body), "Your Clone sold %d items for %d %s. The points are queued for delivery on login.", new_amount, total_cost, currency == VENDING_CURRENCY_CASH ? "Cash Points" : "Kafra Points");
	} else {
		msg.item[0].nameid = currency;
		msg.item[0].amount = total_cost;
		msg.item[0].identify = 1;
		snprintf(msg.title, sizeof(msg.title), "Vending Clone Items");
		snprintf(msg.body, sizeof(msg.body), "Your Clone sold %d items for %d %s.", new_amount, total_cost, itemdb_name(currency));
	}
	intif_Mail_send(0, &msg);
	
	for (int i = 0; i < count; i++) {
		int16 amount = *(uint16*)(data + 4*i);
		int16 idx    = *(uint16*)(data + 4*i + 2);
		idx -= 2;
		struct s_vending_clone_item& it = items->at(idx);
		
		struct item item_copy = it.item_data;
		item_copy.amount = amount;
		pc_additem(sd, &item_copy, amount, LOG_TYPE_VENDING);
		
		log_pick_pc(sd, LOG_TYPE_VENDING, 1, &item_copy);
		
		it.amount -= amount;
	}
	
	bool all_sold = true;
	for (size_t i = 0; i < items->size(); i++) {
		if (items->at(i).amount > 0) {
			all_sold = false;
			break;
		}
	}
	
	if (all_sold) {
		// Immediately close the shop sign so nobody else can interact
		clif_closevendingboard(*nd, AREA, nullptr);
		
		// Clear items to prevent any interaction during the brief delay
		for (auto& item : *items)
			item.amount = 0;
		
		// The 5.5 second freeze issue was caused by an rAthena core bug in npc_unsetcells,
		// not by the client UI. We can safely remove the NPC immediately now!
		vending_clone_delete(nd);
	} else {
		for (int i = 0; i < count; i++) {
			int16 idx = *(uint16*)(data + 4*i + 2) - 2;
			struct s_vending_clone_item& it = items->at(idx);
			Sql_Query(mmysql_handle, "UPDATE `vending_clone_items` SET `amount` = %d WHERE `clone_id` = %d AND `index` = %d", it.amount, nd->u.vending_clone.clone_id, idx);
		}
	}
}

void vending_deliver_pending_point_payouts(map_session_data* sd) {
	if (!sd) return;
	if (Sql_Query(mmysql_handle, "SELECT `currency`,`amount` FROM `vending_clone_point_payouts` WHERE `char_id`=%d", sd->status.char_id) != SQL_SUCCESS) { Sql_ShowDebug(mmysql_handle); return; }
	int64 cash = 0, kafra = 0;
	while (Sql_NextRow(mmysql_handle) == SQL_SUCCESS) {
		char *data = nullptr;
		Sql_GetData(mmysql_handle, 0, &data, nullptr); uint32 currency = strtoul(data, nullptr, 10);
		Sql_GetData(mmysql_handle, 1, &data, nullptr); int64 amount = strtoll(data, nullptr, 10);
		if (currency == VENDING_CURRENCY_CASH) cash += amount;
		else if (currency == VENDING_CURRENCY_KAFRA) kafra += amount;
	}
	Sql_FreeResult(mmysql_handle);
	const int64 cash_delivered = cash > 0 ? std::min<int64>(cash, MAX_CASHPOINT - sd->cashPoints) : 0;
	const int64 kafra_delivered = kafra > 0 ? std::min<int64>(kafra, MAX_KAFRAPOINT - sd->kafraPoints) : 0;
	if (cash_delivered > 0) pc_getcash(sd, (int32)cash_delivered, 0, LOG_TYPE_VENDING);
	if (kafra_delivered > 0) pc_getcash(sd, 0, (int32)kafra_delivered, LOG_TYPE_VENDING);
	if (cash_delivered > 0) Sql_Query(mmysql_handle, "UPDATE `vending_clone_point_payouts` SET `amount`=`amount`-%" PRId64 " WHERE `char_id`=%d AND `currency`=%u", cash_delivered, sd->status.char_id, VENDING_CURRENCY_CASH);
	if (kafra_delivered > 0) Sql_Query(mmysql_handle, "UPDATE `vending_clone_point_payouts` SET `amount`=`amount`-%" PRId64 " WHERE `char_id`=%d AND `currency`=%u", kafra_delivered, sd->status.char_id, VENDING_CURRENCY_KAFRA);
	Sql_Query(mmysql_handle, "DELETE FROM `vending_clone_point_payouts` WHERE `char_id`=%d AND `amount`=0", sd->status.char_id);
}

void vending_init_clones(void) {
	if (Sql_Query(mmysql_handle, "SELECT `clone_id`, `char_id`, `name`, `title`, `map`, `x`, `y`, `sex`, `class`, `hair`, `hair_color`, `head_top`, `head_mid`, `head_bottom`, `robe`, `weapon`, `shield`, `body`, `currency` FROM `vending_clones`") != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
		return;
	}

	if (Sql_NumRows(mmysql_handle) > 0) {
		char* data;
		std::vector<uint32> clones_to_load;
		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
			Sql_GetData(mmysql_handle, 0, &data, nullptr);
			clones_to_load.push_back(atoi(data));
		}
		Sql_FreeResult(mmysql_handle);

		for (uint32 clone_id : clones_to_load) {
			if (Sql_Query(mmysql_handle, "SELECT `char_id`, `name`, `title`, `map`, `x`, `y`, `sex`, `class`, `hair`, `hair_color`, `head_top`, `head_mid`, `head_bottom`, `robe`, `weapon`, `shield`, `body`, `currency` FROM `vending_clones` WHERE `clone_id` = %d", clone_id) != SQL_SUCCESS) continue;
			if (Sql_NumRows(mmysql_handle) == 0) continue;
			Sql_NextRow(mmysql_handle);

			int16 m;
			int16 x, y;
			Sql_GetData(mmysql_handle, 4, &data, nullptr);
			m = map_mapname2mapid(data);
			if (m < 0) continue;

			Sql_GetData(mmysql_handle, 5, &data, nullptr); x = atoi(data);
			Sql_GetData(mmysql_handle, 6, &data, nullptr); y = atoi(data);

			npc_data *nd = nullptr;
			CREATE(nd, npc_data, 1);
			new (nd) npc_data();

			nd->id = npc_get_new_npc_id();
			nd->type = BL_NPC;
			nd->prev = nd->next = nullptr;
			nd->m = m;
			nd->x = x;
			nd->y = y;
			nd->dynamicnpc.removal_tid = INVALID_TIMER;

			if (!nd) continue;

			Sql_GetData(mmysql_handle, 2, &data, nullptr);
			safestrncpy(nd->name, data, sizeof(nd->name));
			safestrncpy(nd->exname, data, sizeof(nd->exname));

			Sql_GetData(mmysql_handle, 8, &data, nullptr); nd->class_ = atoi(data);
			nd->speed = 150;
			Sql_GetData(mmysql_handle, 7, &data, nullptr); nd->vd.sex = atoi(data);
			nd->vd.look[LOOK_BASE] = nd->class_;
			Sql_GetData(mmysql_handle, 9, &data, nullptr); nd->vd.look[LOOK_HAIR] = atoi(data);
			Sql_GetData(mmysql_handle, 10, &data, nullptr); nd->vd.look[LOOK_HAIR_COLOR] = atoi(data);
			Sql_GetData(mmysql_handle, 11, &data, nullptr); nd->vd.look[LOOK_HEAD_TOP] = atoi(data);
			Sql_GetData(mmysql_handle, 12, &data, nullptr); nd->vd.look[LOOK_HEAD_MID] = atoi(data);
			Sql_GetData(mmysql_handle, 13, &data, nullptr); nd->vd.look[LOOK_HEAD_BOTTOM] = atoi(data);
			Sql_GetData(mmysql_handle, 14, &data, nullptr); nd->vd.look[LOOK_ROBE] = atoi(data);
			Sql_GetData(mmysql_handle, 15, &data, nullptr); nd->vd.look[LOOK_WEAPON] = atoi(data);
			Sql_GetData(mmysql_handle, 16, &data, nullptr); nd->vd.look[LOOK_SHIELD] = atoi(data);
			Sql_GetData(mmysql_handle, 17, &data, nullptr); nd->vd.look[LOOK_CLOTHES_COLOR] = atoi(data);
			nd->vd.dead_sit = 2;

			nd->subtype = NPCTYPE_VENDING_CLONE;
			nd->u.vending_clone.clone_id = clone_id;
			
			Sql_GetData(mmysql_handle, 1, &data, nullptr); nd->u.vending_clone.char_id = atoi(data);
			Sql_GetData(mmysql_handle, 3, &data, nullptr); safestrncpy(nd->u.vending_clone.title, data, sizeof(nd->u.vending_clone.title));
			Sql_GetData(mmysql_handle, 18, &data, nullptr); nd->u.vending_clone.currency = atoi(data);
			nd->u.vending_clone.items = new std::vector<struct s_vending_clone_item>();
			Sql_FreeResult(mmysql_handle);

			if (Sql_Query(mmysql_handle, "SELECT `nameid`, `amount`, `price`, `refine`, `attribute`, `identify`, `expire_time`, `bound`, `unique_id`, `enchantgrade`, `durability`, `card0`, `card1`, `card2`, `card3`, `opt_idx0`, `opt_val0`, `opt_parm0`, `opt_idx1`, `opt_val1`, `opt_parm1`, `opt_idx2`, `opt_val2`, `opt_parm2`, `opt_idx3`, `opt_val3`, `opt_parm3`, `opt_idx4`, `opt_val4`, `opt_parm4` FROM `vending_clone_items` WHERE `clone_id` = %d ORDER BY `index` ASC", clone_id) == SQL_SUCCESS) {
				while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
					struct s_vending_clone_item vci;
					memset(&vci.item_data, 0, sizeof(struct item));
					
					Sql_GetData(mmysql_handle, 0, &data, nullptr); vci.item_data.nameid = atoi(data);
					Sql_GetData(mmysql_handle, 1, &data, nullptr); vci.amount = atoi(data);
					Sql_GetData(mmysql_handle, 2, &data, nullptr); vci.price = atoi(data);
					Sql_GetData(mmysql_handle, 3, &data, nullptr); vci.item_data.refine = atoi(data);
					Sql_GetData(mmysql_handle, 4, &data, nullptr); vci.item_data.attribute = atoi(data);
					Sql_GetData(mmysql_handle, 5, &data, nullptr); vci.item_data.identify = atoi(data);
					Sql_GetData(mmysql_handle, 6, &data, nullptr); vci.item_data.expire_time = strtoul(data, nullptr, 10);
					Sql_GetData(mmysql_handle, 7, &data, nullptr); vci.item_data.bound = atoi(data);
					Sql_GetData(mmysql_handle, 8, &data, nullptr); vci.item_data.unique_id = strtoull(data, nullptr, 10);
					Sql_GetData(mmysql_handle, 9, &data, nullptr); vci.item_data.enchantgrade = atoi(data);
					Sql_GetData(mmysql_handle, 10, &data, nullptr); vci.item_data.durability = atoi(data);
					for (int j=0;j<MAX_SLOTS;j++){ Sql_GetData(mmysql_handle, 11+j, &data, nullptr); vci.item_data.card[j]=strtoul(data,nullptr,10); }
					for (int j=0;j<MAX_ITEM_RDM_OPT;j++){
						Sql_GetData(mmysql_handle, 15+j*3, &data, nullptr); vci.item_data.option[j].id=atoi(data);
						Sql_GetData(mmysql_handle, 16+j*3, &data, nullptr); vci.item_data.option[j].value=atoi(data);
						Sql_GetData(mmysql_handle, 17+j*3, &data, nullptr); vci.item_data.option[j].param=atoi(data);
					}

					nd->u.vending_clone.items->push_back(vci);
				}
				Sql_FreeResult(mmysql_handle);
			}

			map_addnpc(m, nd);
			map_addiddb(nd);
			if (map_addblock(nd) == 0) {
				clif_spawn(nd);
				clif_showvendingboard_clone(*nd);
			}
			
			active_vending_clones[nd->u.vending_clone.char_id] = nd;
		}
	}
}

/**	
 * Initialise the vending module
 * called in map::do_init
 */
void do_final_vending(void)
{
	db_destroy(vending_db);
	vending_autotrader_db->destroy(vending_autotrader_db, vending_autotrader_free);
}

/**
 * Destory the vending module
 * called in map::do_final
 */
void do_init_vending(void)
{
	vending_db = idb_alloc(DB_OPT_BASE);
	vending_autotrader_db = uidb_alloc(DB_OPT_BASE);
	vending_nextid = 0;
	
	vending_init_clones();
}

bool vending_has_clone(uint32 char_id) {
	return active_vending_clones.find(char_id) != active_vending_clones.end();
}

int8 vending_close_clone(map_session_data* sd) {
	auto it = active_vending_clones.find(sd->status.char_id);
	if (it == active_vending_clones.end()) return 0;
	
	struct npc_data* nd = it->second;
	if (nd->u.vending_clone.items) {
		int new_ = 0;
		int blank = pc_inventoryblank(sd);
		int w = 0;

		// Validation check
		for (auto& item_it : *nd->u.vending_clone.items) {
			if (item_it.amount > 0) {
				switch (pc_checkadditem(sd, item_it.item_data.nameid, item_it.amount)) {
				case CHKADDITEM_EXIST:
					break;
				case CHKADDITEM_NEW:
					new_++;
					if (new_ > blank) {
						// Removed clif_displaymessage because atcommand.cpp handles it
						return 2;
					}
					break;
				case CHKADDITEM_OVERAMOUNT:
					return 3;
				}
				w += itemdb_weight(item_it.item_data.nameid) * item_it.amount;
			}
		}

		if (sd->weight + w > sd->max_weight) {
			return 4;
		}

		// Proceed to add items
		for (auto& item_it : *nd->u.vending_clone.items) {
			if (item_it.amount > 0) {
				struct item item_copy = item_it.item_data;
				item_copy.amount = item_it.amount;
				pc_additem(sd, &item_copy, item_it.amount, LOG_TYPE_VENDING);
			}
		}
	}
	
	vending_clone_delete(nd);
	return 1;
}
