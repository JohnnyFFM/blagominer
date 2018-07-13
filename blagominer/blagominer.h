#pragma once
#include "InstructionSet.h"
#include "bfs.h"
#include "network.h"
#include "shabal.h"

// blago version
#ifdef __AVX2__
	char const *const version = "v1.170997_AVX2_BFS_DEV";
#else
	#ifdef __AVX__
		char const *const version = "v1.170997_AVX_BFS_DEV";
	#else
		char const *const version = "v1.170997_BFS_DEV";
	#endif
#endif 

extern HANDLE hHeap;							//heap

// locks
extern CRITICAL_SECTION sessionsLock;			// session lock
extern CRITICAL_SECTION bestsLock;				// best lock
extern CRITICAL_SECTION sharesLock;				// shares lock

extern SYSTEMTIME cur_time;						// current time

// global variables

// miner
extern bool exit_flag;							// true if miner is to be exited
extern volatile int stopThreads;
extern char *pass;								// passphrase for solo mining
extern unsigned long long total_size;			// sum of all local plot file sizes
extern std::vector<std::string> paths_dir;      // plot paths

//miner config items
extern size_t miner_mode;						// miner mode. 0=solo, 1=pool
extern bool use_debug;							// output debug information if true

// round info
extern char signature[33];						// signature of current block
extern unsigned long long baseTarget;			// base target of current block
extern unsigned long long targetDeadlineInfo;   // target deadline info from pool 
extern unsigned long long height;				// current block height
extern unsigned long long deadline;				// current deadline
extern unsigned int scoop;						// currenty scoop

// PoC2
extern unsigned long long POC2StartBlock;		// block where PoC2 was activated (block 502000 main net, block 71666 on test net)
extern bool POC2;								// true if PoC2 is activated


// structures


struct t_shares {
	std::string file_name;
	unsigned long long account_id;// = 0;
	unsigned long long best;// = 0;
	unsigned long long nonce;// = 0;
};

extern std::vector<t_shares> shares;

struct t_best {
	unsigned long long account_id;// = 0;
	unsigned long long best;// = 0;
	unsigned long long nonce;// = 0;
	unsigned long long DL;// = 0;
	unsigned long long targetDeadline;// = 0;
};

extern std::vector<t_best> bests;

struct t_session {
	SOCKET Socket;
	unsigned long long deadline;
	t_shares body;
};

extern std::vector<t_session> sessions;

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
#include "worker.h"

//headers
size_t GetFiles(const std::string &str, std::vector <t_files> *p_files);
