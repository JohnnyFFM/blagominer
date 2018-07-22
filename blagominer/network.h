#pragma once
#include "logger.h"
#include "error.h"
#include "accounts.h"


#define RAPIDJSON_NO_SIZETYPEDEFINE

namespace rapidjson { typedef size_t SizeType; }
using namespace rapidjson;

//config items
extern size_t send_interval;					// время ожидания между отправками
extern size_t update_interval;					// время ожидания между апдейтами								
extern bool enable_proxy;						// enable client/server functionality
extern bool show_winner;						// true: show winner information after each round

#include "rapidjson/document.h"		// rapidjson's DOM-style API
#include "rapidjson/error/en.h"

#pragma comment(lib,"Ws2_32.lib")

#include "picohttpparser.h"

extern std::map <u_long, unsigned long long> satellite_size; // Структура с объемами плотов сателлитов

extern char str_signature[65];


extern int network_quality;
extern std::thread showWinner;

extern std::string nodeaddr;	// адрес пула
extern std::string nodeport;		// порт пула

extern std::string updateraddr;// адрес пула
extern std::string updaterport;		// порт пула

extern std::string infoaddr;	// адрес пула
extern std::string infoport;		// порт пула

extern std::string proxyport;		// порт пула



char* GetJSON(char const *const req);

void ShowWinner(unsigned long long const num_block);
void pollLocal(void);

void hostname_to_ip(char const *const  in_addr, char* out_addr);
void updater_i(void);
void proxy_i(void);
void send_i(void);
size_t xstr2strr(char *buf, size_t const bufsize, const char *const in);
