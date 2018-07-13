#pragma once
#include "blagominer.h"

extern bool use_boost;							// use optimisations if true
extern size_t cache_size1;						// Cache in nonces (1 nonce in scoop = 64 bytes) for native POC
extern size_t cache_size2;						// Cache in nonces (1 nonce in scoop = 64 bytes) for on-the-fly POC conversion
extern size_t readChunkSize;					// Size of HDD reads in nonces (1 nonce in scoop = 64 bytes)

// worker progress
struct t_worker_progress {
	size_t Number;
	unsigned long long Reads_bytes;
	bool isAlive;
};

extern std::vector<t_worker_progress> worker_progress;

// function headers
void work_i(const size_t local_num);
void th_hash(t_files const * const iter, double * const sum_time_proc, const size_t &local_num, unsigned long long const bytes, size_t const cache_size_local, unsigned long long const i, unsigned long long const nonce, unsigned long long const n, char const * const cache, size_t const acc);
void th_read(HANDLE ifile, unsigned long long const start, unsigned long long const MirrorStart, bool * const cont, unsigned long long * const bytes, t_files const * const iter, bool * const flip, bool p2, unsigned long long const i, unsigned long long const stagger, size_t * const cache_size_local, char * const cache, char * const MirrorCache);
