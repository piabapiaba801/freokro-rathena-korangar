// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// Custom server-side skills for this project.

#include "skill_factory_custom.hpp"

#include <common/sql.hpp>
#include <common/showmsg.hpp>
#include <common/timer.hpp>

#include "map/battle.hpp"
#include "map/chrif.hpp"
#include "map/clif.hpp"
#include "map/intif.hpp"
#include "map/log.hpp"
#include "map/map.hpp"
#include "map/mob.hpp"
#include "map/party.hpp"
#include "map/pet.hpp"
#include "map/npc.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

#include <cstdio>
#include <cinttypes>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#ifdef _WIN32
#include <windows.h>
#endif

namespace {
std::filesystem::path auction_runtime_path() {
	std::filesystem::path runtime = std::filesystem::path("../client/auction-ui/runtime");
#ifdef _WIN32
	char executable[MAX_PATH] = {};
	if (GetModuleFileNameA(nullptr, executable, MAX_PATH) > 0)
		runtime = std::filesystem::path(executable).parent_path().parent_path() / "client" / "auction-ui" / "runtime";
#endif
	return runtime;
}

std::string json_escape(const char* input) {
	std::string output;
	for (const unsigned char c : std::string(input ? input : "")) {
		switch (c) { case '\\': output += "\\\\"; break; case '"': output += "\\\""; break; case '\n': output += "\\n"; break; case '\r': break; default: output += static_cast<char>(c); }
	}
	return output;
}

uint64 request_number(const std::unordered_map<std::string, std::string>& values, const char* key) {
	auto found = values.find(key);
	return found == values.end() ? 0 : strtoull(found->second.c_str(), nullptr, 10);
}

uint64 auction_sql_rows() {
	if (Sql_Query(mmysql_handle, "SELECT ROW_COUNT()") != SQL_SUCCESS || Sql_NextRow(mmysql_handle) != SQL_SUCCESS) return 0;
	char* value = nullptr; Sql_GetData(mmysql_handle, 0, &value, nullptr);
	const uint64 rows = value ? strtoull(value, nullptr, 10) : 0; Sql_FreeResult(mmysql_handle); return rows;
}

const char* auction_item_type(item_types type) {
	switch (type) {
		case IT_HEALING: return "Restaurativo";
		case IT_USABLE: case IT_DELAYCONSUME: return "Consumivel";
		case IT_ETC: return "Diversos";
		case IT_ARMOR: return "Equipamento";
		case IT_WEAPON: return "Arma";
		case IT_CARD: return "Carta";
		case IT_PETEGG: return "Ovo de pet";
		case IT_PETARMOR: return "Equipamento de pet";
		case IT_AMMO: return "Municao";
		case IT_SHADOWGEAR: return "Equipamento sombrio";
		case IT_CASH: return "Item especial";
		default: return "Item";
	}
}

std::string auction_item_details(const item_data* data, bool identified, int enchant_grade,
	const t_itemid* cards, const s_item_randomoption* options) {
	std::ostringstream out;
	out << "{\"identified\":" << (identified ? "true" : "false")
		<< ",\"type\":\"" << auction_item_type(data->type) << "\""
		<< ",\"weight\":" << data->weight << ",\"attack\":" << data->atk
		<< ",\"defense\":" << data->def << ",\"slots\":" << data->slots
		<< ",\"weapon_level\":" << data->weapon_level << ",\"armor_level\":" << data->armor_level
		<< ",\"equip_level_min\":" << data->elv << ",\"equip_level_max\":" << data->elvmax
		<< ",\"enchant_grade\":" << enchant_grade;
#ifdef RENEWAL
	out << ",\"magic_attack\":" << data->matk;
#else
	out << ",\"magic_attack\":0";
#endif
	out << ",\"cards\":[";
	bool first = true;
	for (int i = 0; i < MAX_SLOTS; ++i) {
		if (!cards[i]) continue;
		std::shared_ptr<item_data> card = item_db.find(cards[i]);
		if (!card || card->type != IT_CARD) continue;
		if (!first) out << ',';
		first = false;
		out << cards[i];
	}
	out << "],\"options\":[";
	first = true;
	for (int i = 0; i < MAX_ITEM_RDM_OPT; ++i) {
		if (options[i].id <= 0) continue;
		if (!first) out << ',';
		first = false;
		out << "{\"id\":" << options[i].id << ",\"value\":" << options[i].value
			<< ",\"param\":" << static_cast<int>(options[i].param) << '}';
	}
	out << "]}";
	return out.str();
}

bool auction_insert_listing(const map_session_data* sd, const item& entry, int amount,
	uint64 start, uint64 buy, uint64 fee, int hours) {
	char seller_name[NAME_LENGTH * 2 + 1] = {};
	Sql_EscapeString(mmysql_handle, seller_name, sd->status.name);
	std::ostringstream columns, values;
	columns << "`seller_account_id`,`seller_char_id`,`seller_name`,`item_id`,`amount`,`identify`,`refine`,`attribute`,`bound`,`expire_time`,`unique_id`,`enchantgrade`,`durability`,`card0`,`card1`,`card2`,`card3`";
	values << sd->status.account_id << ',' << sd->status.char_id << ",'" << seller_name << "',"
		<< entry.nameid << ',' << amount << ',' << static_cast<int>(entry.identify) << ','
		<< static_cast<int>(entry.refine) << ',' << static_cast<int>(entry.attribute) << ','
		<< static_cast<int>(entry.bound) << ',' << entry.expire_time << ',' << entry.unique_id << ','
		<< static_cast<int>(entry.enchantgrade) << ',' << entry.durability;
	for (int i = 0; i < MAX_SLOTS; ++i) values << ',' << entry.card[i];
	for (int i = 0; i < MAX_ITEM_RDM_OPT; ++i) {
		columns << ",`opt_id" << i << "`,`opt_val" << i << "`,`opt_par" << i << '`';
		values << ',' << entry.option[i].id << ',' << entry.option[i].value << ',' << static_cast<int>(entry.option[i].param);
	}
	columns << ",`start_price`,`buy_now`,`listing_fee`,`ends_at`,`status`";
	values << ',' << start << ',' << buy << ',' << fee << ",DATE_ADD(NOW(),INTERVAL " << hours << " HOUR),'ESCROW_PENDING'";
	const std::string statement = "INSERT INTO `custom_auction` (" + columns.str() + ") VALUES (" + values.str() + ')';
	return Sql_Query(mmysql_handle, "%s", statement.c_str()) == SQL_SUCCESS;
}

uint64 auction_sql_number(const char* value) {
	return value ? strtoull(value, nullptr, 10) : 0;
}

TIMER_FUNC(auction_mail_refresh_timer) {
	map_session_data* sd = map_charid2sd(id);
	if (sd)
		intif_Mail_requestinbox(sd->status.char_id, 1, MAIL_INBOX_NORMAL);
	return 0;
}

void auction_response(const std::filesystem::path& runtime, const std::string& request_id, bool ok, const std::string& message, map_session_data* sd) {
	std::ostringstream json;
	json << "{\"request_id\":\"" << json_escape(request_id.c_str()) << "\",\"ok\":" << (ok ? "true" : "false")
		<< ",\"message\":\"" << json_escape(message.c_str()) << "\",\"zeny\":" << (sd ? sd->status.zeny : 0)
		<< ",\"fee_rate\":" << (sd && pc_get_group_id(sd) >= 1 ? 7 : 27) << ",\"auctions\":[";
	bool first = true;
	if (sd && Sql_Query(mmysql_handle,
		"SELECT a.`auction_id`,a.`item_id`,a.`amount`,a.`refine`,GREATEST(a.`current_bid`,a.`start_price`),a.`buy_now`,GREATEST(0,TIMESTAMPDIFF(SECOND,NOW(),a.`ends_at`)),a.`seller_name`,a.`seller_account_id`,a.`highest_bidder_account_id`,a.`identify`,a.`enchantgrade`,a.`card0`,a.`card1`,a.`card2`,a.`card3`,a.`opt_id0`,a.`opt_val0`,a.`opt_par0`,a.`opt_id1`,a.`opt_val1`,a.`opt_par1`,a.`opt_id2`,a.`opt_val2`,a.`opt_par2`,a.`opt_id3`,a.`opt_val3`,a.`opt_par3`,a.`opt_id4`,a.`opt_val4`,a.`opt_par4` FROM `custom_auction` a WHERE a.`status`='ACTIVE' AND a.`ends_at`>NOW() ORDER BY a.`ends_at` LIMIT 100") == SQL_SUCCESS) {
		while (Sql_NextRow(mmysql_handle) == SQL_SUCCESS) {
			char *v[31] = {}; for (int i=0;i<31;++i) Sql_GetData(mmysql_handle,i,&v[i],nullptr);
			const t_itemid item_id = static_cast<t_itemid>(auction_sql_number(v[1]));
			std::shared_ptr<item_data> item = item_db.find(item_id);
			t_itemid cards[MAX_SLOTS] = {};
			for (int i = 0; i < MAX_SLOTS; ++i) cards[i] = static_cast<t_itemid>(auction_sql_number(v[12 + i]));
			s_item_randomoption options[MAX_ITEM_RDM_OPT] = {};
			for (int i = 0; i < MAX_ITEM_RDM_OPT; ++i) {
				options[i].id = static_cast<int16>(auction_sql_number(v[16 + i * 3]));
				options[i].value = static_cast<int16>(auction_sql_number(v[17 + i * 3]));
				options[i].param = static_cast<char>(auction_sql_number(v[18 + i * 3]));
			}
			if (!first) json << ','; first = false;
			json << "{\"id\":"<<v[0]<<",\"item_id\":"<<v[1]<<",\"name\":\""<<json_escape(item ? item->ename.c_str() : "Unknown")
				<<"\",\"amount\":"<<v[2]<<",\"refine\":"<<v[3]<<",\"current\":"<<v[4]<<",\"buy_now\":"<<v[5]
				<<",\"seconds\":"<<v[6]<<",\"seller\":\""<<json_escape(v[7])<<"\",\"mine\":"<<(strtoul(v[8],nullptr,10)==sd->status.account_id?"true":"false")
				<<",\"leading\":"<<(strtoul(v[9],nullptr,10)==sd->status.account_id?"true":"false")
				<<",\"details\":"<<(item ? auction_item_details(item.get(), auction_sql_number(v[10]) != 0,
					static_cast<int>(auction_sql_number(v[11])), cards, options) : "null")<<'}';
		}
		Sql_FreeResult(mmysql_handle);
	}
	json << "],\"inventory\":["; first = true;
	if (sd) for (int i=0;i<MAX_INVENTORY;++i) {
		const item& entry = sd->inventory.u.items_inventory[i];
		item_data* data = sd->inventory_data[i];
		if (!data || !entry.nameid || !entry.amount || entry.equip) continue;
		const bool auctionable = !entry.bound && !entry.expire_time && pc_can_trade_item(sd,i)
			&& itemdb_canauction(&entry, pc_get_group_level(sd));
		const char* reason = entry.bound ? "Item vinculado" : entry.expire_time ? "Item temporario" : "Item nao negociavel";
		if (!first) json << ','; first=false;
		json << "{\"index\":"<<i<<",\"item_id\":"<<entry.nameid<<",\"name\":\""<<json_escape(data->ename.c_str())<<"\",\"amount\":"<<entry.amount<<",\"refine\":"<<static_cast<int>(entry.refine)
			<<",\"eligible\":"<<(auctionable?"true":"false")<<",\"reason\":\""<<(auctionable?"":reason)<<"\""
			<<",\"details\":"<<auction_item_details(data, entry.identify != 0, entry.enchantgrade, entry.card, entry.option)<<'}';
	}
	json << "]}";
	std::error_code error; std::filesystem::create_directories(runtime,error);
	const auto temporary=runtime/"response.json.tmp", target=runtime/"response.json";
	std::ofstream out(temporary,std::ios::trunc|std::ios::binary); out<<json.str(); out.close();
	std::filesystem::remove(target,error); error.clear(); std::filesystem::rename(temporary,target,error);
}

TIMER_FUNC(auction_bridge_timer) {
	const auto runtime=auction_runtime_path(), request=runtime/"request.pending";
	if (!std::filesystem::exists(request)) return 0;
	std::unordered_map<std::string,std::string> values; std::ifstream in(request); std::string line;
	while(std::getline(in,line)){ const size_t split=line.find('='); if(split!=std::string::npos) values[line.substr(0,split)]=line.substr(split+1); } in.close();
	std::error_code error; std::filesystem::remove(request,error);
	const std::string request_id=values["request_id"], action=values["action"];
	map_session_data* sd=map_charid2sd(static_cast<int32>(request_number(values,"char_id")));
	if (!sd || sd->status.account_id!=request_number(values,"account_id")) { auction_response(runtime,request_id,false,"Sessao do personagem indisponivel.",nullptr); return 0; }
	bool ok=true; std::string message;
	if(action=="bid" || action=="buy") {
		const uint64 auction=request_number(values,"auction_id"), offer=request_number(values,"amount");
		uint64 current=0,start=0,buy=0,seller=0,leader=0,leader_char=0; std::string leader_name; char *v[7]={};
		if(Sql_Query(mmysql_handle,"SELECT `current_bid`,`start_price`,`buy_now`,`seller_account_id`,`highest_bidder_account_id`,`highest_bidder_char_id`,`highest_bidder_name` FROM `custom_auction` WHERE `auction_id`=%" PRIu64 " AND `status`='ACTIVE' AND `ends_at`>NOW()",auction)!=SQL_SUCCESS || Sql_NextRow(mmysql_handle)!=SQL_SUCCESS) { ok=false; message="Leilao indisponivel."; Sql_FreeResult(mmysql_handle); }
		else { for(int i=0;i<7;++i) Sql_GetData(mmysql_handle,i,&v[i],nullptr); current=strtoull(v[0],nullptr,10); start=strtoull(v[1],nullptr,10); buy=strtoull(v[2],nullptr,10); seller=strtoull(v[3],nullptr,10); leader=strtoull(v[4],nullptr,10); leader_char=strtoull(v[5],nullptr,10); leader_name=v[6]?v[6]:""; Sql_FreeResult(mmysql_handle);
			if(seller==sd->status.account_id){ok=false;message="Voce nao pode comprar seu proprio leilao.";}
			else { uint64 price=action=="buy"?buy:offer, base=std::max(current,start), minimum=current?base+std::max<uint64>(1000,(base*5+99)/100):start;
				if((action=="buy" && !buy)||(action=="bid" && (price<minimum || (buy && price>=buy)))){ok=false;message="Valor invalido para esta operacao.";}
				else { uint64 reserve=(leader==sd->status.account_id&&current?price-current:price); if(reserve>static_cast<uint64>(sd->status.zeny)){ok=false;message="Zeny insuficiente.";}
					else if(pc_payzeny(sd,static_cast<int32>(reserve),LOG_TYPE_NPC)!=0){ok=false;message="Nao foi possivel reservar o Zeny.";}
					else { const char* status=action=="buy"?"SETTLEMENT_PENDING":"ACTIVE"; Sql_Query(mmysql_handle,"START TRANSACTION"); int changed=Sql_Query(mmysql_handle,"UPDATE `custom_auction` SET `current_bid`=%" PRIu64 ",`highest_bidder_account_id`=%u,`highest_bidder_char_id`=%u,`highest_bidder_name`='%s',`bid_count`=`bid_count`+1,`status`='%s',`ended_at`=IF('%s'='SETTLEMENT_PENDING',NOW(),`ended_at`),`settlement_reason`=IF('%s'='SETTLEMENT_PENDING','BUY_NOW',`settlement_reason`),`ends_at`=IF('%s'='ACTIVE' AND TIMESTAMPDIFF(SECOND,NOW(),`ends_at`)<=120,DATE_ADD(`ends_at`,INTERVAL 2 MINUTE),`ends_at`) WHERE `auction_id`=%" PRIu64 " AND `status`='ACTIVE' AND `ends_at`>NOW() AND `current_bid`=%" PRIu64 " AND `highest_bidder_account_id`=%" PRIu64,price,sd->status.account_id,sd->status.char_id,sd->status.name,status,status,status,status,auction,current,leader);
						bool sql_ok=changed==SQL_SUCCESS&&auction_sql_rows()==1;
						if(sql_ok) sql_ok=Sql_Query(mmysql_handle,"INSERT INTO `custom_auction_bid` (`auction_id`,`account_id`,`char_id`,`char_name`,`amount`,`refunded`) VALUES (%" PRIu64 ",%u,%u,'%s',%" PRIu64 ",0)",auction,sd->status.account_id,sd->status.char_id,sd->status.name,price)==SQL_SUCCESS;
						if(sql_ok&&current&&leader&&leader!=sd->status.account_id) sql_ok=Sql_Query(mmysql_handle,"INSERT INTO `custom_auction_wallet` (`account_id`,`balance`,`refund_char_id`,`refund_char_name`) VALUES (%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",'%s') ON DUPLICATE KEY UPDATE `balance`=`balance`+VALUES(`balance`),`refund_char_id`=VALUES(`refund_char_id`),`refund_char_name`=VALUES(`refund_char_name`)",leader,current,leader_char,leader_name.c_str())==SQL_SUCCESS;
						if(!sql_ok){Sql_Query(mmysql_handle,"ROLLBACK");pc_getzeny(sd,static_cast<int32>(reserve),LOG_TYPE_NPC);ok=false;message="O leilao mudou. Zeny devolvido.";} else {Sql_Query(mmysql_handle,"COMMIT");message=action=="buy"?"Compra imediata concluida.":"Lance registrado.";if(action=="buy"){npc_event_do("CustomAuctionSettlement::OnAuctionSettlement");add_timer(gettick()+750,auction_mail_refresh_timer,sd->status.char_id,0);if(Sql_Query(mmysql_handle,"SELECT `seller_char_id` FROM `custom_auction` WHERE `auction_id`=%" PRIu64,auction)==SQL_SUCCESS&&Sql_NextRow(mmysql_handle)==SQL_SUCCESS){char* seller_char=nullptr;Sql_GetData(mmysql_handle,0,&seller_char,nullptr);if(seller_char)add_timer(gettick()+750,auction_mail_refresh_timer,static_cast<int32>(strtoul(seller_char,nullptr,10)),0);Sql_FreeResult(mmysql_handle);}}}
					}
				}
			}
		}
	} else if(action=="create") {
		const int index=static_cast<int>(request_number(values,"index")), amount=static_cast<int>(request_number(values,"amount")); const uint64 start=request_number(values,"start"), buy=request_number(values,"buy"); int hours=static_cast<int>(request_number(values,"hours"));
		uint64 active_count=0; if(Sql_Query(mmysql_handle,"SELECT COUNT(*) FROM `custom_auction` WHERE `seller_account_id`=%u AND `status` IN ('ESCROW_PENDING','ACTIVE','SETTLEMENT_PENDING')",sd->status.account_id)==SQL_SUCCESS&&Sql_NextRow(mmysql_handle)==SQL_SUCCESS){char* count=nullptr;Sql_GetData(mmysql_handle,0,&count,nullptr);active_count=count?strtoull(count,nullptr,10):0;Sql_FreeResult(mmysql_handle);}
		if(active_count>=static_cast<uint64>(pc_get_group_id(sd)>=1?10:3)){ok=false;message="Limite de leiloes simultaneos atingido.";}
		else if(index<0||index>=MAX_INVENTORY||!sd->inventory_data[index]||amount<1||amount>sd->inventory.u.items_inventory[index].amount||start<1||(buy&&buy<=start)||sd->inventory.u.items_inventory[index].equip||sd->inventory.u.items_inventory[index].bound||sd->inventory.u.items_inventory[index].expire_time||!pc_can_trade_item(sd,index)||!itemdb_canauction(&sd->inventory.u.items_inventory[index],pc_get_group_level(sd))){ok=false;message="Item ou valores invalidos.";}
		else { if(hours!=6&&hours!=12&&hours!=24&&hours!=48)hours=24; uint64 fee=(start*(pc_get_group_id(sd)>=1?7:27)+99)/100; if(fee>static_cast<uint64>(sd->status.zeny)){ok=false;message="Zeny insuficiente para a taxa.";}
			else { item entry=sd->inventory.u.items_inventory[index]; if(!auction_insert_listing(sd,entry,amount,start,buy,fee,hours)){ok=false;message="Falha ao iniciar escrow.";}
				else { uint64 auction=Sql_LastInsertId(mmysql_handle); bool paid=pc_payzeny(sd,static_cast<int32>(fee),LOG_TYPE_NPC)==0; bool removed=paid&&pc_delitem(sd,index,amount,0,0,LOG_TYPE_NPC)==0; if(!removed){if(paid)pc_getzeny(sd,static_cast<int32>(fee),LOG_TYPE_NPC);Sql_Query(mmysql_handle,"DELETE FROM `custom_auction` WHERE `auction_id`=%" PRIu64 " AND `status`='ESCROW_PENDING'",auction);ok=false;message="Inventario mudou; operacao cancelada.";} else {Sql_Query(mmysql_handle,"UPDATE `custom_auction` SET `status`='ACTIVE' WHERE `auction_id`=%" PRIu64 " AND `status`='ESCROW_PENDING'",auction);message="Leilao criado com sucesso.";}}
			}
		}
	}
	auction_response(runtime,request_id,ok,message,sd); return 0;
}

void ensure_auction_bridge() { static int32 timer_id=INVALID_TIMER; if(timer_id==INVALID_TIMER) timer_id=add_timer_interval(gettick()+250,auction_bridge_timer,0,0,250); }

void signal_auction_hud(const map_session_data* sd) {
	// Local FreokRO distribution bridge. The client agent only reacts to a new
	// atomic signal created after it started; no inventory or credentials cross
	// this file boundary.
	std::filesystem::path runtime = auction_runtime_path();
	std::error_code directory_error;
	std::filesystem::create_directories(runtime, directory_error);
	const std::filesystem::path temporary = runtime / "open.signal.tmp";
	const std::filesystem::path target = runtime / "open.signal";
	std::ofstream stream(temporary, std::ios::out | std::ios::trunc);
	if (!stream.is_open()) {
		ShowWarning("Auction HUD: could not create local open signal.\n");
		return;
	}
	stream << "version=1\n"
		<< "timestamp=" << static_cast<long long>(std::time(nullptr)) << "\n"
		<< "account_id=" << sd->status.account_id << "\n"
		<< "char_id=" << sd->status.char_id << "\n"
		<< "group_id=" << pc_get_group_id(sd) << "\n"
		<< "fee_rate=" << (pc_get_group_id(sd) >= 1 ? 7 : 27) << "\n";
	stream.close();
	std::error_code publish_error;
	std::filesystem::remove(target, publish_error);
	publish_error.clear();
	std::filesystem::rename(temporary, target, publish_error);
	if (publish_error)
		ShowWarning("Auction HUD: could not publish local open signal.\n");
}

class SkillAreaLoot final : public SkillImpl {
public:
	SkillAreaLoot() : SkillImpl(CUSTOM_AREA_LOOT) {}
	void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32&) const override {
		map_session_data* sd = BL_CAST(BL_PC, src);
		if (!sd) return;
		if (!battle_config.funk_master_enable || !battle_config.funk_basic_skills || !battle_config.funk_area_loot) { clif_displaymessage(sd->fd,"[FUNK] Coleta em Area temporariamente desativada."); return; }
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		map_foreachinallrange(skill_greed, target, skill_get_splash(getSkillId(), skill_lv), BL_ITEM, target);
	}
};

class SkillMonsterRead final : public SkillImpl {
public:
	SkillMonsterRead() : SkillImpl(CUSTOM_MONSTER_READ) {}
	void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32&) const override {
		map_session_data* sd = BL_CAST(BL_PC, src);
		mob_data* md = BL_CAST(BL_MOB, target);
		if (!battle_config.funk_master_enable || !battle_config.funk_basic_skills || !battle_config.funk_monster_catalog || !battle_config.funk_monster_read) { if(sd) clif_displaymessage(sd->fd,"[FUNK] Olhar do Cacador temporariamente desativado."); return; }
		if (!sd) return;
		if (!md) {
			pet_catch_process_start(*sd, 0, PET_CATCH_CUSTOM_READ);
			return;
		}

		// Server is authoritative: the client only supplies a target GID. Never accept a Mob ID from it.
		// Slaves/summons, guardians and technical castle objects are not catalog species.
		if (md->master_id != 0 || md->guardian_data != nullptr || md->mob_id == MOBID_EMPERIUM || md->db == nullptr) {
			clif_displaymessage(sd->fd, "[Olhar do Cacador] Esta criatura nao pode ser catalogada.");
			return;
		}

		bool first_discovery = false;
		if (!pc_monster_catalog_known(sd, md->mob_id)) {
			if (Sql_Query(mmysql_handle, "START TRANSACTION") != SQL_SUCCESS ||
				Sql_Query(mmysql_handle,
					"INSERT IGNORE INTO `monster_catalog` (`account_id`,`mob_id`,`discovered_char_id`,`category`,`map`,`reward_state`) "
					"VALUES (%u,%u,%u,%u,'%s',0)", sd->status.account_id, md->mob_id, sd->status.char_id,
					static_cast<unsigned>(md->get_bosstype()), map_getmapdata(sd->m)->name) != SQL_SUCCESS ||
				Sql_Query(mmysql_handle, "SELECT ROW_COUNT()") != SQL_SUCCESS || Sql_NextRow(mmysql_handle) != SQL_SUCCESS) {
				Sql_ShowDebug(mmysql_handle);
				Sql_Query(mmysql_handle, "ROLLBACK");
				clif_displaymessage(sd->fd, "[Olhar do Cacador] Falha ao registrar a descoberta. Nenhuma recompensa foi consumida.");
				return;
			}
			char* row = nullptr;
			Sql_GetData(mmysql_handle, 0, &row, nullptr);
			first_discovery = row != nullptr && strtoul(row, nullptr, 10) == 1;
			Sql_FreeResult(mmysql_handle);
			if (Sql_Query(mmysql_handle, "COMMIT") != SQL_SUCCESS) {
				Sql_ShowDebug(mmysql_handle);
				Sql_Query(mmysql_handle, "ROLLBACK");
				return;
			}
			// Another simultaneous request/account session may have won INSERT IGNORE.
			sd->monster_catalog_known.insert(md->mob_id);
		}

		if (first_discovery) {
			// Show a real card cut-in only when its extracted client artwork is
			// installed. No item is granted and no card-specific text is sent.
			for (const std::shared_ptr<s_mob_drop>& drop : md->db->dropitem) {
				std::shared_ptr<item_data> item = item_db.find(drop->nameid);
				if (item != nullptr && item->type == IT_CARD) {
					switch (item->nameid) {
						case 4007: clif_cutin(*sd, "freok_card_4007", 4); break; // Peco Peco Egg
						case 4031: clif_cutin(*sd, "freok_card_4031", 4); break; // Peco Peco
						default: break; // Never request a missing client file.
					}
					break;
				}
			}

			t_exp reward = (md->get_bosstype() == BOSSTYPE_NONE) ? 100000 : 500000;

			// Runtime kill-switch: discovery and account-wide knowledge remain fully active.
			// Only the EXP payment is disabled, so administrators can react instantly without restart.
			if (!battle_config.monster_catalog_exp_enable) {
				if (Sql_Query(mmysql_handle,
					"UPDATE `monster_catalog` SET `reward_state`=3,`reward_base`=0,`reward_job`=0 WHERE `account_id`=%u AND `mob_id`=%u AND `reward_state`=0",
					sd->status.account_id, md->mob_id) != SQL_SUCCESS)
					Sql_ShowDebug(mmysql_handle);
				clif_displaymessage(sd->fd, "[Olhar do Cacador] Nova especie registrada. Recompensa de EXP esta desativada pelo servidor.");
				clif_skill_estimation_self(*sd, *md);
				clif_name(sd, md, SELF);
				clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
				return;
			}

			// Anti-duplication journal. State 1 means claimed/in-flight; state 2 means delivered.
			// On a process crash during state 1 we intentionally do not auto-repay: this prevents duplication;
			// the row remains auditable for administrator recovery.
			if (Sql_Query(mmysql_handle,
				"UPDATE `monster_catalog` SET `reward_state`=1,`reward_base`=%" PRIu64 ",`reward_job`=%" PRIu64 " "
				"WHERE `account_id`=%u AND `mob_id`=%u AND `reward_state`=0",
				reward, reward, sd->status.account_id, md->mob_id) != SQL_SUCCESS) {
				Sql_ShowDebug(mmysql_handle);
				clif_displaymessage(sd->fd, "[Olhar do Cacador] Descoberta registrada; recompensa retida para auditoria.");
			} else {
				pc_gainexp(sd, md, reward, reward, 0); // normal personal/VIP/manual/race EXP boosts are allowed
				chrif_save(sd, CSAVE_NORMAL);
				if (Sql_Query(mmysql_handle,
					"UPDATE `monster_catalog` SET `reward_state`=2,`rewarded_at`=NOW() WHERE `account_id`=%u AND `mob_id`=%u AND `reward_state`=1",
					sd->status.account_id, md->mob_id) != SQL_SUCCESS)
					Sql_ShowDebug(mmysql_handle);
				char out[256];
				std::snprintf(out, sizeof(out), "[Olhar do Cacador] Nova especie: %s. Recompensa base: %" PRIu64 " Base EXP + %" PRIu64 " Job EXP (bonus de EXP aplicaveis).", md->name, reward, reward);
				clif_displaymessage(sd->fd, out);
			}
		} else {
			clif_displaymessage(sd->fd, "[Olhar do Cacador] Esta especie ja esta registrada nesta conta. Nenhuma nova recompensa.");
		}

		// Private result: never broadcast Sense data to party members.
		clif_skill_estimation_self(*sd, *md);
		clif_name(sd, md, SELF); // immediately refresh ????? -> real data for this viewer
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
};

class SkillReturnSave final : public SkillImpl {
public:
	SkillReturnSave() : SkillImpl(CUSTOM_RETURN_SAVE) {}
	void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32&) const override {
		map_session_data* sd = BL_CAST(BL_PC, src);
		if (!sd) return;
		if (!battle_config.funk_master_enable || !battle_config.funk_basic_skills || !battle_config.funk_return_save) { clif_displaymessage(sd->fd,"[FUNK] Retorno ao Ponto Salvo temporariamente desativado."); return; }
		if (map_getmapflag(sd->m, MF_NORETURN) || map_getmapflag(sd->m, MF_NOTELEPORT)) {
			clif_displaymessage(sd->fd, "Return cannot be used on this map.");
			return;
		}
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		pc_setpos_savepoint(*sd, CLR_TELEPORT);
	}
};

class SkillUniversalCraft final : public SkillImpl {
public:
	SkillUniversalCraft() : SkillImpl(CUSTOM_UNIVERSAL_CRAFT) {}
	void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32&) const override {
		map_session_data* sd = BL_CAST(BL_PC, src);
		if (!sd) return;
		if (!battle_config.funk_master_enable || !battle_config.funk_basic_skills || !battle_config.funk_universal_craft) { clif_displaymessage(sd->fd,"[FUNK] Oficio Universal temporariamente desativado."); return; }
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		// CUSTOM_UNIVERSAL_CRAFT is handled specially by the production list/checks:
		// it exposes every recipe for which the character has the materials.
		clif_skill_produce_mix_list(*sd, getSkillId(), -1);
	}
};

class SkillTameCreature final : public SkillImpl {
public:
	SkillTameCreature() : SkillImpl(CUSTOM_TAME_CREATURE) {}
	void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32&) const override {
		map_session_data* sd = BL_CAST(BL_PC, src);
		mob_data* md = BL_CAST(BL_MOB, target);
		if (!sd) return;
		if (!battle_config.funk_master_enable || !battle_config.funk_basic_skills || !battle_config.funk_tame_creature) { clif_displaymessage(sd->fd,"[FUNK] Domar Criatura temporariamente desativado."); return; }
		if (md) return;
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		pet_catch_process_start(*sd, 0, PET_CATCH_CUSTOM_TAME);
	}
};

class SkillMonsterCatalog final : public SkillImpl {
public:
	SkillMonsterCatalog() : SkillImpl(CUSTOM_MONSTER_CATALOG) {}
	void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32&) const override {
		map_session_data* sd = BL_CAST(BL_PC, src);
		if (!sd) return;
		if (!battle_config.funk_master_enable || !battle_config.funk_basic_skills || !battle_config.funk_monster_catalog) {
			clif_displaymessage(sd->fd, "[FUNK] Identificacao/Catalogo temporariamente desativado.");
			return;
		}
		if (sd->npc_id != 0) return; // prevent dialog re-entry/spam
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		npc_event(sd, "FreaokMonsterCatalog::OnOpen", 0);
	}
};

class SkillBlackMarket final : public SkillImpl {
public:
	SkillBlackMarket() : SkillImpl(CUSTOM_BLACK_MARKET) {}
	void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32&) const override {
		map_session_data* sd = BL_CAST(BL_PC, src);
		if (!sd) return;
		if (!battle_config.funk_master_enable || !battle_config.funk_basic_skills || !battle_config.funk_auction) { clif_displaymessage(sd->fd,"[FUNK] Leilao temporariamente desativado."); return; }
		// FreokRO: Black Market is a city-only utility skill.
		if (sd->m < 0 || !map_getmapflag(sd->m, MF_TOWN)) {
			clif_displaymessage(sd->fd, "Mercado Negro: esta habilidade so pode ser utilizada em cidades.");
			return;
		}
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		ensure_auction_bridge();
		signal_auction_hud(sd);
	}
};

class SkillEcommerce final : public SkillImpl {
public:
	SkillEcommerce() : SkillImpl(CUSTOM_ECOMMERCE) {}
	void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32&) const override {
		map_session_data* sd = BL_CAST(BL_PC, src);
		if (!sd) return;
		if (!battle_config.funk_master_enable || !battle_config.funk_basic_skills || !battle_config.funk_ecommerce) {
			clif_displaymessage(sd->fd, "[FUNK] Ecommerce temporariamente desativado.");
			return;
		}
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		npc_event(sd, "CustomBlackMarket::OnEcommerce", 0);
	}
};

class SkillLeafWings final : public SkillImpl {
public:
	SkillLeafWings() : SkillImpl(CUSTOM_LEAF_WINGS) {}
	void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32&) const override {
		map_session_data* sd = BL_CAST(BL_PC, src);
		if (!sd) return;
		if (!battle_config.funk_master_enable || !battle_config.funk_basic_skills || !battle_config.funk_leaf_wings) { clif_displaymessage(sd->fd,"[FUNK] Asa de Leaf temporariamente desativada."); return; }
		if (map_getmapflag(sd->m, MF_NOTELEPORT)) {
			clif_displaymessage(sd->fd, "Leaf Wings cannot be used on this map.");
			return;
		}
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		party_data* p = sd->status.party_id ? party_search(sd->status.party_id) : nullptr;
		bool leader = false;
		if (p) {
			for (int i = 0; i < MAX_PARTY; ++i)
				if (p->data[i].sd == sd) { leader = p->party.member[i].leader != 0; break; }
		}
		if (!p || !leader) {
			pc_randomwarp(sd, CLR_TELEPORT);
			return;
		}
		// Giant Fly Wing semantics: leader random-warps, then same-map online members are gathered nearby.
		const int16 old_map = sd->m;
		pc_randomwarp(sd, CLR_TELEPORT);
		for (int i = 0; i < MAX_PARTY; ++i) {
			map_session_data* member = p->data[i].sd;
			if (!member || member == sd || member->m != old_map || pc_isdead(member)) continue;
			pc_setpos(member, map_id2index(sd->m), sd->x, sd->y, CLR_TELEPORT);
		}
	}
};
}

std::unique_ptr<const SkillImpl> SkillFactoryCustom::create(const e_skill skill_id) const {
	switch (skill_id) {
		case CUSTOM_AREA_LOOT: return std::make_unique<SkillAreaLoot>();
		case CUSTOM_MONSTER_READ: return std::make_unique<SkillMonsterRead>();
		case CUSTOM_UNIVERSAL_CRAFT: return std::make_unique<SkillUniversalCraft>();
		case CUSTOM_RETURN_SAVE: return std::make_unique<SkillReturnSave>();
		case CUSTOM_LEAF_WINGS: return std::make_unique<SkillLeafWings>();
		case CUSTOM_TAME_CREATURE: return std::make_unique<SkillTameCreature>();
		case CUSTOM_BLACK_MARKET: return std::make_unique<SkillBlackMarket>();
		case CUSTOM_MONSTER_CATALOG: return std::make_unique<SkillMonsterCatalog>();
		case CUSTOM_ECOMMERCE: return std::make_unique<SkillEcommerce>();
		case ST_PRESERVE: return std::make_unique<StatusSkillImpl>(skill_id, true); // Toggle Preserve ON/OFF when recast.
		default: return nullptr;
	}
}
