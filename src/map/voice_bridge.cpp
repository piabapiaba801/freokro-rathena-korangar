#include "voice_bridge.hpp"
#include "battle.hpp"
#include <common/timer.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_map>
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# pragma comment(lib, "ws2_32.lib")
using voice_socket_t = SOCKET;
static constexpr voice_socket_t VOICE_INVALID_SOCKET = INVALID_SOCKET;
static void voice_close(voice_socket_t s){ closesocket(s); }
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
using voice_socket_t = int;
static constexpr voice_socket_t VOICE_INVALID_SOCKET = -1;
static void voice_close(voice_socket_t s){ ::close(s); }
#endif
namespace {
voice_socket_t sock = VOICE_INVALID_SOCKET;
sockaddr_in target{};
bool ready = false;
int32 voice_timer = INVALID_TIMER;
std::string secret;
struct Last { int16 x=-1,y=-1; t_tick sent=0,auth=0; int16 m=-1; };
std::unordered_map<uint32,Last> cache;
std::string trim(std::string s){ auto a=s.find_first_not_of(" \t\r\n"); auto b=s.find_last_not_of(" \t\r\n"); return a==std::string::npos?std::string():s.substr(a,b-a+1); }
void load(std::string& host,int& port){ std::ifstream f("conf/voice_athena.conf"); std::string line; while(std::getline(f,line)){ auto c=line.find("//"); if(c!=std::string::npos) line.resize(c); auto p=line.find(':'); if(p==std::string::npos) continue; auto k=trim(line.substr(0,p)),v=trim(line.substr(p+1)); if(k=="voice_api_ip"&&!v.empty()) host=v; else if(k=="voice_api_port"&&!v.empty()) port=std::atoi(v.c_str()); else if(k=="voice_bridge_secret") secret=v; } }
std::string esc(const char* s){ std::string o; for(;s&&*s;s++){ if(*s=='\\'||*s=='\"') o+='\\'; o+=*s; } return o; }
bool send_text(std::string s){ if(!ready) return false; if(!secret.empty() && !s.empty() && s.back()=='}') s.insert(s.size()-1,",\"bridge_secret\":\""+secret+"\""); return sendto(sock,s.c_str(),(int)s.size(),0,(sockaddr*)&target,sizeof(target))==(int)s.size(); }
void send_auth(map_session_data* sd){ if(!sd) return; char b[192]; std::snprintf(b,sizeof(b),"{\"type\":\"auth_advisory\",\"char_id\":%d,\"account_id\":%d,\"login_id1\":%u}",sd->status.char_id,sd->status.account_id,(unsigned)sd->login_id1); send_text(b); }
void send_pos(map_session_data* sd){ if(!sd) return; auto* md=map_getmapdata(sd->m); const char* name=md?md->name:""; char b[320]; std::snprintf(b,sizeof(b),"{\"type\":\"map_pos\",\"char_id\":%d,\"map\":\"%s\",\"x\":%d,\"y\":%d,\"level\":%d,\"job\":%d,\"group_id\":%d,\"war_map\":%s}",sd->status.char_id,esc(name).c_str(),sd->x,sd->y,(int)sd->status.base_level,(int)sd->status.class_,(int)sd->group_id,map_flag_gvg(sd->m)?"true":"false"); send_text(b); }
int ping(map_session_data* sd,va_list){ if(!sd||sd->status.char_id<=0) return 0; auto& l=cache[sd->status.char_id]; auto now=gettick(); if(l.x!=sd->x||l.y!=sd->y||l.m!=sd->m||now-l.sent>=1500){ send_pos(sd); l.x=sd->x;l.y=sd->y;l.m=sd->m;l.sent=now; } if(l.auth==0||now-l.auth>=5000){ send_auth(sd);l.auth=now; } return 0; }
TIMER_FUNC(tick){ map_foreachpc(ping); return 0; }
}
void voice_bridge_init(){ if(!battle_config.funk_master_enable || !battle_config.funk_voice_chat) return; if(ready) return; std::string host="127.0.0.1";int port=7001;load(host,port);
#ifdef _WIN32
WSADATA w{}; if(WSAStartup(MAKEWORD(2,2),&w)!=0) return;
#endif
sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); if(sock==VOICE_INVALID_SOCKET) return; target.sin_family=AF_INET;target.sin_port=htons((uint16)port); if(inet_pton(AF_INET,host.c_str(),&target.sin_addr)!=1){voice_close(sock);sock=VOICE_INVALID_SOCKET;return;} ready=true; if(voice_timer == INVALID_TIMER) voice_timer=add_timer_interval(gettick()+1000,tick,0,0,1000); }
void voice_bridge_final(){ if(voice_timer != INVALID_TIMER){ delete_timer(voice_timer,tick); voice_timer=INVALID_TIMER; } if(sock!=VOICE_INVALID_SOCKET){voice_close(sock);sock=VOICE_INVALID_SOCKET;} ready=false;cache.clear();
#ifdef _WIN32
WSACleanup();
#endif
}
void voice_bridge_send_auth_advisory(map_session_data* sd){send_auth(sd);} void voice_bridge_send_map_pos(map_session_data* sd){send_pos(sd);} 
void voice_bridge_send_join(map_session_data* sd){ if(!sd)return;send_auth(sd);send_pos(sd);auto& l=cache[sd->status.char_id];l.x=sd->x;l.y=sd->y;l.m=sd->m;l.sent=l.auth=gettick(); }
void voice_bridge_send_leave(map_session_data* sd){ if(!sd)return;cache.erase(sd->status.char_id);char b[96];std::snprintf(b,sizeof(b),"{\"type\":\"auth_revoke\",\"char_id\":%d}",sd->status.char_id);send_text(b);std::snprintf(b,sizeof(b),"{\"type\":\"map_leave\",\"char_id\":%d}",sd->status.char_id);send_text(b); }
void voice_bridge_send_room_join(map_session_data* sd,int room){if(!sd||room<=0)return;char b[112];std::snprintf(b,sizeof(b),"{\"type\":\"chat_join\",\"char_id\":%d,\"room_id\":%d}",sd->status.char_id,room);send_text(b);} void voice_bridge_send_room_leave(map_session_data* sd){if(!sd)return;char b[80];std::snprintf(b,sizeof(b),"{\"type\":\"chat_leave\",\"char_id\":%d}",sd->status.char_id);send_text(b);} void voice_bridge_send_guild_war_state(bool a){send_text(std::string("{\"type\":\"guild_war_state\",\"active\":")+(a?"true}":"false}"));}
