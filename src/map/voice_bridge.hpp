#pragma once
#include "map.hpp"
#include "pc.hpp"
void voice_bridge_init();
void voice_bridge_final();
void voice_bridge_send_join(map_session_data* sd);
void voice_bridge_send_auth_advisory(map_session_data* sd);
void voice_bridge_send_map_pos(map_session_data* sd);
void voice_bridge_send_leave(map_session_data* sd);
void voice_bridge_send_room_join(map_session_data* sd, int room_id);
void voice_bridge_send_room_leave(map_session_data* sd);
void voice_bridge_send_guild_war_state(bool active);
