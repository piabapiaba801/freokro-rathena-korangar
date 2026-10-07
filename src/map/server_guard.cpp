#include "server_guard.hpp"

#include "battle.hpp"
#include "pc.hpp"

#include "../common/showmsg.hpp"
#include "../common/timer.hpp"

static bool server_guard_window(map_session_data* sd, t_tick now, t_tick& window_tick, uint16& count,
	uint16 limit, const char* label) {
	if (limit == 0)
		return true;
	if (window_tick == 0 || DIFF_TICK(now, window_tick) >= 1000) {
		window_tick = now;
		count = 0;
	}
	if (count < UINT16_MAX)
		++count;
	if (count <= limit)
		return true;

	if (sd->server_guard.last_violation_tick == 0 || DIFF_TICK(now, sd->server_guard.last_violation_tick) >= 1000) {
		sd->server_guard.last_violation_tick = now;
		if (sd->server_guard.violations < UINT16_MAX)
			++sd->server_guard.violations;
		if (battle_config.server_guard_log)
			ShowWarning("[ServerGuard] %s flood: AID=%u CID=%u name='%s' count=%u limit=%u violations=%u\n",
				label, sd->status.account_id, sd->status.char_id, sd->status.name, count, limit, sd->server_guard.violations);
	}

	// Rollout-safe: only drop excess requests. No automatic bans are performed server-side.
	return battle_config.server_guard_drop_excess == 0;
}

bool server_guard_check(map_session_data* sd, e_server_guard_event event) {
	if (sd == nullptr || !battle_config.funk_master_enable || !battle_config.funk_server_guard || !battle_config.server_guard_enable)
		return true;
	const t_tick now = gettick();
	switch (event) {
		case SG_EVENT_SKILL:
			return server_guard_window(sd, now, sd->server_guard.skill_window_tick, sd->server_guard.skill_packets,
				static_cast<uint16>(battle_config.server_guard_skill_pps), "skill");
		case SG_EVENT_WALK:
			return server_guard_window(sd, now, sd->server_guard.walk_window_tick, sd->server_guard.walk_packets,
				static_cast<uint16>(battle_config.server_guard_walk_pps), "walk");
		default:
			return true;
	}
}

void server_guard_reset(map_session_data* sd) {
	if (sd == nullptr)
		return;
	sd->server_guard = {};
}
