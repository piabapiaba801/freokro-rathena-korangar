#ifndef MAP_SERVER_GUARD_HPP
#define MAP_SERVER_GUARD_HPP

#include "../common/cbasetypes.hpp"
class map_session_data;

enum e_server_guard_event : uint8 {
	SG_EVENT_SKILL = 1,
	SG_EVENT_WALK = 2,
};

// Returns false when the request should be dropped.
bool server_guard_check(map_session_data* sd, e_server_guard_event event);
void server_guard_reset(map_session_data* sd);

#endif
