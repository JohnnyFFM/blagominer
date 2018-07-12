#pragma once

#define RAPIDJSON_NO_SIZETYPEDEFINE

namespace rapidjson { typedef size_t SizeType; }
using namespace rapidjson;

#include "rapidjson/document.h"		// rapidjson's DOM-style API
#include "rapidjson/error/en.h"

#include <ws2tcpip.h>

//includes
#include <windows.h> //windows API
#undef  MOUSE_MOVED
#include "curses.h" //include pdcurses
#include <thread> //threadding
#include <vector> //vectors
#include <sstream>
#include "InstructionSet.h"
#include <map>
#include "sph_shabal.h"
#include "mshabal.h"
#include "mshabal256.h"
//BFS Support
#include "bfs.h"

#pragma comment(lib,"Ws2_32.lib")
//#include <mswsock.h> // Need for SO_UPDATE_CONNECT_CONTEXT


#include "picohttpparser.h"

HANDLE hHeap;

CRITICAL_SECTION sessionsLock;	
CRITICAL_SECTION bestsLock;		
CRITICAL_SECTION sharesLock;	

bool exit_flag = false;
char *p_minerPath = nullptr;		// Path
char *pass = nullptr;							// пароль
unsigned long long baseTarget = 0;
char signature[33];
char str_signature[65];
char oldSignature[33];
unsigned long long total_size = 0;	// Общий объем плотов
std::map <u_long, unsigned long long> satellite_size; // Структура с объемами плотов сателлитов
unsigned long long targetDeadlineInfo = 0;			// Максимальный дедлайн пула
unsigned long long height = 0;
volatile int stopThreads = 0;
int network_quality = 100;
unsigned long long deadline = 0;
unsigned int scoop = 0;

WINDOW * win_main;
#ifdef __AVX2__
char const *const version = "v1.170997_AVX2_BFS_DEV";
#else
#ifdef __AVX__
char const *const version = "v1.170997_AVX_BFS_DEV";
#else
char const *const version = "v1.170997_BFS_DEV";
#endif
#endif 
//config

std::string nodeaddr = "localhost";	// адрес пула
std::string nodeport = "8125";		// порт пула

std::string updateraddr = "localhost";// адрес пула
std::string updaterport = "8125";		// порт пула

std::string infoaddr = "localhost";	// адрес пула
std::string infoport = "8125";		// порт пула

std::string proxyport = "8125";		// порт пула

unsigned long long my_target_deadline = MAXDWORD;	// 4294967295;
SYSTEMTIME cur_time;				// Текущее время

size_t miner_mode = 0;				// режим майнера. 0=соло, 1=пул
size_t cache_size = 16384;			// Cache in nonces (1 nonce in scoop = 64 bytes) for native POC
size_t cache_size2 = 262144;		// Cache in nonces (1 nonce in scoop = 64 bytes) for on-the-fly POC conversion
size_t readChunkSize = 16384;		// Size of HDD reads in nonces (1 nonce in scoop = 64 bytes)

std::vector<std::string> paths_dir; // пути
									//bool show_msg = false;				// Показать общение с сервером в отправщике
									//bool show_updates = false;			// Показать общение с сервером в апдейтере
FILE * fp_Log = nullptr;			// указатель на лог-файл
size_t send_interval = 100;			// время ожидания между отправками
size_t update_interval = 1000;		// время ожидания между апдейтами
short win_size_x = 80;
short win_size_y = 60;
//bool use_fast_rcv = false;
bool use_debug = false;
bool enable_proxy = false;
//bool send_best_only = true;
bool use_wakeup = false;
bool use_log = true;				// Вести лог
bool use_boost = false;				// Использовать повышенный приоритет для потоков
bool show_winner = false;			// показывать победителя
									//short can_generate = 0;				// 0 - disable; 1 - can start generate; 2 - already run generator
//POC2: HF Block where POC2 gets active
unsigned long long POC2StartBlock = 502000;
//indicates if POC2 is active
bool POC2 = false;

//BFS TOC
BFSTOC bfsTOC;
//4k address of BFSTOC on harddisk. default = 5
unsigned int bfsTOCOffset = 5;
//HDD wakeup timer in seconds
unsigned int hddWakeUpTimer = 180;


std::vector<std::thread> worker;
std::thread showWinner;
sph_shabal_context global_y,local_y;
mshabal256_context global_x;
mshabal_context global_z;

bool done =  false;

struct t_worker_progress {
	size_t Number;
	unsigned long long Reads_bytes;
	bool isAlive;
};

std::vector<t_worker_progress> worker_progress;

struct t_shares {
	std::string file_name;
	unsigned long long account_id;// = 0;
	unsigned long long best;// = 0;
	unsigned long long nonce;// = 0;
};

std::vector<t_shares> shares;

struct t_best {
	unsigned long long account_id;// = 0;
	unsigned long long best;// = 0;
	unsigned long long nonce;// = 0;
	unsigned long long DL;// = 0;
	unsigned long long targetDeadline;// = 0;
};

std::vector<t_best> bests;

struct t_session {
	SOCKET Socket;
	unsigned long long deadline;
	t_shares body;
};

std::vector<t_session> sessions;

struct t_files {
	std::string Path;
	std::string Name;
	unsigned long long Size;
	unsigned long long Key;
	unsigned long long StartNonce;
	unsigned long long Nonces;
	unsigned long long Stagger;
	unsigned long long Offset;
	bool P2;
	bool BFS;
};

//headers
int load_config(char const *const filename);
void Log(char const *const strLog);
void GetCPUInfo(void);
void ShowMemErrorExit(void);
void GetPass(char const *const p_strFolderPath);
void hostname_to_ip(char const *const  in_addr, char* out_addr);
size_t GetFiles(const std::string &str, std::vector <t_files> *p_files);
void Log_llu(unsigned long long const llu_num);
void Log_u(size_t const u_num);
void updater_i(void);
void proxy_i(void);
void Log_init(void);
void send_i(void);
void work_i(const size_t local_num);
void ShowWinner(unsigned long long const num_block);
size_t Get_index_acc(unsigned long long const key);
void pollLocal(void);
int xdigit(char const digit);
size_t xstr2strr(char *buf, size_t const bufsize, const char *const in);
void th_hash(t_files const * const iter, double * const sum_time_proc, const size_t &local_num, unsigned long long const bytes, size_t const cache_size_local, unsigned long long const i, unsigned long long const nonce, unsigned long long const n, char const * const cache, size_t const acc);
void th_read(HANDLE ifile, unsigned long long const start, unsigned long long const MirrorStart, bool * const cont, unsigned long long * const bytes, t_files const * const iter, bool * const flip, bool p2, unsigned long long const i, unsigned long long const stagger, size_t * const cache_size_local, char * const cache, char * const MirrorCache);
void procscoop_sph(const unsigned long long nonce, const unsigned long long n, char const *const data, const size_t acc, const std::string &file_name);
char* GetJSON(char const *const req);
void procscoop_m256_8(unsigned long long const nonce, unsigned long long const n, char const *const data, size_t const acc, const std::string &file_name);
void procscoop_m_4(unsigned long long const nonce, unsigned long long const n, char const *const data, size_t const acc, const std::string &file_name);

