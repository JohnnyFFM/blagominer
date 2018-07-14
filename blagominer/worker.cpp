#include "stdafx.h"
#include "worker.h"

bool use_boost = false;				// use optimisations if true
size_t cache_size1 = 16384;			// Cache in nonces (1 nonce in scoop = 64 bytes) for native POC
size_t cache_size2 = 262144;		// Cache in nonces (1 nonce in scoop = 64 bytes) for on-the-fly POC conversion
size_t readChunkSize = 16384;		// Size of HDD reads in nonces (1 nonce in scoop = 64 bytes)

void work_i(const size_t local_num)
{

	__int64 start_work_time, end_work_time;
	__int64 start_time_read, end_time_read;

	double sum_time_proc = 0;
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	double const pcFreq = double(li.QuadPart);
	QueryPerformanceCounter((LARGE_INTEGER*)&start_work_time);

	if (use_boost)
	{
		SetThreadIdealProcessor(GetCurrentThread(), (DWORD)(local_num % std::thread::hardware_concurrency()));
	}

	std::string const path_loc_str = paths_dir[local_num];
	unsigned long long files_size_per_thread = 0;

	Log("\nStart thread: ["); Log_llu(local_num); Log("]  ");	Log((char*)path_loc_str.c_str());

	std::vector<t_files> files;
	GetFiles(path_loc_str, &files);

	size_t cache_size_local;
	DWORD sectorsPerCluster;
	DWORD bytesPerSector;
	DWORD numberOfFreeClusters;
	DWORD totalNumberOfClusters;
	bool converted = false;
	bool isbfs = false;

	//for (auto iter = files.begin(); iter != files.end(); ++iter)
	for (auto iter = files.rbegin(); iter != files.rend(); ++iter)
	{
		unsigned long long key, nonce, nonces, stagger, offset, tail;
		bool p2, bfs;
		QueryPerformanceCounter((LARGE_INTEGER*)&start_time_read);
		key = iter->Key;
		nonce = iter->StartNonce;
		nonces = iter->Nonces;
		stagger = iter->Stagger;
		offset = iter->Offset;
		p2 = iter->P2;
		bfs = iter->BFS;
		tail = 0;

		// Проверка кратности нонсов стаггеру
		if ((double)(nonces % stagger) > DBL_EPSILON && !bfs)
		{
			bm_wattron(12);
			bm_wprintw("File %s wrong stagger?\n", iter->Name.c_str(), 0);
			bm_wattroff(12);
		}

		// Проверка на повреждения плота
		if (nonces != (iter->Size) / (4096 * 64))
		{
			bm_wattron(12);
			bm_wprintw("file \"%s\" name/size mismatch\n", iter->Name.c_str(), 0);
			bm_wattroff(12);
			if (nonces != stagger)
				nonces = (((iter->Size) / (4096 * 64)) / stagger) * stagger; //обрезаем плот по размеру и стаггеру
			else
				if (scoop > (iter->Size) / (stagger * 64)) //если номер скупа попадает в поврежденный смерженный плот, то пропускаем
				{
					bm_wattron(12);
					bm_wprintw("skipped\n", 0);
					bm_wattroff(12);
					continue;
				}
		}

		//get sector information, set to 4096 for BFS
		if (!bfs) {
			if (!GetDiskFreeSpaceA((iter->Path).c_str(), &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters, &totalNumberOfClusters))
			{
				bm_wattron(12);
				bm_wprintw("GetDiskFreeSpace failed\n", 0); //BFS
				bm_wattroff(12);
				continue;
			}
		}
		else
		{
			bytesPerSector = 4096;
		}

		// Если стаггер в плоте меньше чем размер сектора - пропускаем
		if ((stagger * 64) < bytesPerSector)
		{
			bm_wattron(12);
			bm_wprintw("stagger (%llu) must be >= %llu\n", stagger, bytesPerSector / 64, 0);
			bm_wattroff(12);
			continue;
		}

		// Если нонсов в плоте меньше чем размер сектора - пропускаем
		if ((nonces * 64) < bytesPerSector)
		{
			bm_wattron(12);
			bm_wprintw("nonces (%llu) must be >= %llu\n", nonces, bytesPerSector / 64, 0);
			bm_wattroff(12);
			continue;
		}

		// Если стаггер не выровнен по сектору - можем читать сдвигая последний стагер назад (доделать)
		if ((stagger % (bytesPerSector / 64)) != 0)
		{
			bm_wattron(12);
			bm_wprintw("stagger (%llu) must be a multiple of %llu\n", stagger, bytesPerSector / 64, 0);
			bm_wattroff(12);

			continue;
		}

		//PoC2 cache size added (shuffling needs more cache)

		if (p2 != POC2) {
			if ((stagger == nonces) && (cache_size2 < stagger)) cache_size_local = cache_size2;  // оптимизированный плот
			else cache_size_local = stagger; // обычный плот
		}
		else {
			if ((stagger == nonces) && (cache_size1 < stagger))
			{
				cache_size_local = cache_size1;  // оптимизированный плот
			}
			else
			{
				if (!bfs) {
					cache_size_local = stagger; // обычный плот 
				}
				else
				{
					cache_size_local = cache_size1;  //BFS
				}
			}
		}

		//Выравниваем cache_size_local по размеру сектора
		cache_size_local = (cache_size_local / (size_t)(bytesPerSector / 64)) * (size_t)(bytesPerSector / 64);
		//wprintw(win_main, "round: %llu\n", cache_size_local);

		size_t cache_size_local_backup = cache_size_local;
		char *cache = (char *)VirtualAlloc(nullptr, cache_size_local * 64, MEM_COMMIT, PAGE_READWRITE); //cache thread1
		char *cache2 = (char *)VirtualAlloc(nullptr, cache_size_local * 64, MEM_COMMIT, PAGE_READWRITE); //cache thread2
		char *MirrorCache = nullptr;
		if (p2 != POC2) {
			MirrorCache = (char *)VirtualAlloc(nullptr, cache_size_local * 64, MEM_COMMIT, PAGE_READWRITE); //PoC2 cache
			if (MirrorCache == nullptr) ShowMemErrorExit();
			converted = true;
		}

		if (bfs) isbfs = true;

		if (cache == nullptr) ShowMemErrorExit();
		if (cache2 == nullptr) ShowMemErrorExit();


		Log("\nRead file : ");	Log((char*)iter->Name.c_str());

		//wprintw(win_main, "%S \n", str2wstr(iter->Path + iter->Name).c_str());
		HANDLE ifile = INVALID_HANDLE_VALUE;
		if (bfs) {
			ifile = CreateFileA((iter->Path).c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, nullptr);
		}
		else {
			ifile = CreateFileA((iter->Path + iter->Name).c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, nullptr);
		}
		if (ifile == INVALID_HANDLE_VALUE)
		{
			bm_wattron(12);
			bm_wprintw("File \"%s\\%s\" error opening. code = %llu\n", iter->Path.c_str(), iter->Name.c_str(), GetLastError(), 0);
			bm_wattroff(12);
			VirtualFree(cache, 0, MEM_RELEASE);
			VirtualFree(cache2, 0, MEM_RELEASE); //Cleanup Thread 2
			if (p2 != POC2) VirtualFree(MirrorCache, 0, MEM_RELEASE); //PoC2 Cleanup
			continue;
		}
		files_size_per_thread += iter->Size;

		unsigned long long start, bytes;

		LARGE_INTEGER liDistanceToMove = { 0 };

		//PoC2 Vars
		unsigned long long MirrorStart;

		LARGE_INTEGER MirrorliDistanceToMove = { 0 };
		bool flip = false;


		size_t acc = Get_index_acc(key);
		for (unsigned long long n = 0; n < nonces; n += stagger)
		{
			cache_size_local = cache_size_local_backup;
			//Parallel Processing
			bool err = false;
			bool cont = false;

			start = n * 4096 * 64 + scoop * stagger * 64 + offset * 64 * 64; //BFSstaggerstart + scoopstart + offset
			MirrorStart = n * 4096 * 64 + (4095 - scoop) * stagger * 64 + offset * 64 * 64; //PoC2 Seek possition
			int count = 0;

			//Initial Reading
			th_read(ifile, start, MirrorStart, &cont, &bytes, &(*iter), &flip, p2, 0, stagger, &cache_size_local, cache, MirrorCache);

			char *cachep;
			unsigned long long i;
			for (i = cache_size_local; i < min(nonces, stagger); i += cache_size_local)
			{

				//Threadded Hashing
				err = false;
				if (count % 2 == 0) {
					cachep = cache;
				}
				else {
					cachep = cache2;
				}

				//check if hashing would fail
				if (bytes != cache_size_local * 64)
				{
					bm_wattron(12);
					bm_wprintw("Unexpected end of file %s\n", iter->Name.c_str(), 0);
					bm_wattroff(12);
					err = true;
					break;
				}

				std::thread hash(th_hash, &(*iter), &sum_time_proc, local_num, bytes, cache_size_local, i - cache_size_local, nonce, n, cachep, acc);

				cont = false;
				//Threadded Reading
				if (count % 2 == 1) {
					cachep = cache;
				}
				else {
					cachep = cache2;
				}
				std::thread read = std::thread(th_read, ifile, start, MirrorStart, &cont, &bytes, &(*iter), &flip, p2, i, stagger, &cache_size_local, cachep, MirrorCache);

				//Join threads
				hash.join();
				read.join();
				count += 1;
				if (cont) continue;

				// New block while processing: Stop.
				if (stopThreads)
				{
					worker_progress[local_num].isAlive = false;
					Log("\nReading file: ");	Log((char*)iter->Name.c_str()); Log(" interrupted");
					CloseHandle(ifile);
					files.clear();
					VirtualFree(cache, 0, MEM_RELEASE); //Cleanup Thread 1
					VirtualFree(cache2, 0, MEM_RELEASE); //Cleanup Thread 2
					if (p2 != POC2) VirtualFree(MirrorCache, 0, MEM_RELEASE); //PoC2 Cleanup
					return;
				}
			}

			//Final Hashing
			//check if hashing would fail
			if (!err) {
				if (bytes != cache_size_local * 64)
				{
					bm_wattron(12);
					bm_wprintw("Unexpected end of file %s\n", iter->Name.c_str(), 0);
					bm_wattroff(12);
					err = true;
				}
			}
			if (!err)
			{
				if (count % 2 == 0) {
					th_hash(&(*iter), &sum_time_proc, local_num, bytes, cache_size_local, i - cache_size_local, nonce, n, cache, acc);
				}
				else {
					th_hash(&(*iter), &sum_time_proc, local_num, bytes, cache_size_local, i - cache_size_local, nonce, n, cache2, acc);
				}
			}

		}
		QueryPerformanceCounter((LARGE_INTEGER*)&end_time_read);
		Log("\nClose file: ");	Log((char*)iter->Name.c_str()); Log(" [@ "); Log_llu((long long unsigned)((double)(end_time_read - start_time_read) * 1000 / pcFreq)); Log(" ms]");
		//bfs seek optimisation
		if (bfs && iter == files.rend()) {
			//assuming physical hard disk mid at scoop 1587
			start = 0 * 4096 * 64 + 1587 * stagger * 64 + 6 * 64 * 64;
			if (!SetFilePointerEx(ifile, liDistanceToMove, nullptr, FILE_BEGIN))
			{
				bm_wattron(12);
				bm_wprintw("BFS seek optimisation: error SetFilePointerEx. code = %llu\n", GetLastError(), 0);
				bm_wattroff(12);
			}
		}
		CloseHandle(ifile);
		VirtualFree(cache, 0, MEM_RELEASE);
		VirtualFree(cache2, 0, MEM_RELEASE); //Cleanup Thread 2
		if (p2 != POC2) VirtualFree(MirrorCache, 0, MEM_RELEASE); //PoC2 Cleanup
	}
	worker_progress[local_num].isAlive = false;
	QueryPerformanceCounter((LARGE_INTEGER*)&end_work_time);

	//if (use_boost) SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

	double thread_time = (double)(end_work_time - start_work_time) / pcFreq;
	if (use_debug)
	{
		char tbuffer[9];
		_strtime_s(tbuffer);
		bm_wattron(7);
		if (isbfs) {
			bm_wprintw("%s Thread \"%s\" @ %.1f sec (%.1f MB/s) CPU %.2f%% (BFS)\n", tbuffer, path_loc_str.c_str(), thread_time, (double)(files_size_per_thread) / thread_time / 1024 / 1024 / 4096, sum_time_proc / pcFreq * 100 / thread_time, 0);
		}
		else
		{
			if (converted) {
				bm_wprintw("%s Thread \"%s\" @ %.1f sec (%.1f MB/s) CPU %.2f%% (POC1<>POC2)\n", tbuffer, path_loc_str.c_str(), thread_time, (double)(files_size_per_thread) / thread_time / 1024 / 1024 / 4096, sum_time_proc / pcFreq * 100 / thread_time, 0);
			}
			else {
				bm_wprintw("%s Thread \"%s\" @ %.1f sec (%.1f MB/s) CPU %.2f%%\n", tbuffer, path_loc_str.c_str(), thread_time, (double)(files_size_per_thread) / thread_time / 1024 / 1024 / 4096, sum_time_proc / pcFreq * 100 / thread_time, 0);
			}
		}
		bm_wattroff(7);
	}
	return;
}

void th_read(HANDLE ifile, unsigned long long const start, unsigned long long const MirrorStart, bool * const cont, unsigned long long * const bytes, t_files const * const iter, bool * const flip, bool p2, unsigned long long const i, unsigned long long const stagger, size_t * const cache_size_local, char * const cache, char * const MirrorCache) {
	if (i + *cache_size_local > stagger)
	{
		*cache_size_local = stagger - i;  // остаток
#ifdef __AVX2__
		if (*cache_size_local < 8)
		{
			bm_wattron(12);
			bm_wprintw("WARNING: %llu\n", *cache_size_local);
			bm_wattroff(12);
		}
#else
#ifdef __AVX__
		if (*cache_size_local < 4)
		{
			bm_wattron(12);
			bm_wprintw("WARNING: %llu\n", *cache_size_local);
			bm_wattroff(12);
		}
#endif
#endif
	}

	DWORD b = 0;
	DWORD Mirrorb = 0;
	LARGE_INTEGER liDistanceToMove;
	LARGE_INTEGER MirrorliDistanceToMove;

	//alternating front scoop back scoop
	if (*flip) goto poc2read;


	//POC1 scoop read
poc1read:
	*bytes = 0;
	b = 0;
	liDistanceToMove.QuadPart = start + i * 64;
	if (!SetFilePointerEx(ifile, liDistanceToMove, nullptr, FILE_BEGIN))
	{
		bm_wattron(12);
		bm_wprintw("error SetFilePointerEx. code = %llu\n", GetLastError(), 0);
		bm_wattroff(12);
		*cont = true;
		return;
	}
	do {
		if (!ReadFile(ifile, &cache[*bytes], (DWORD)(min(readChunkSize, *cache_size_local - *bytes / 64) * 64), &b, NULL))
		{
			bm_wattron(12);
			bm_wprintw("error P1 ReadFile. code = %llu\n", GetLastError(), 0);
			bm_wattroff(12);
			break;
		}
		*bytes += b;

	} while (*bytes < *cache_size_local * 64);
	if (*flip) goto readend;

poc2read:
	//PoC2 mirror scoop read
	if (p2 != POC2) {
		*bytes = 0;
		Mirrorb = 0;
		MirrorliDistanceToMove.QuadPart = MirrorStart + i * 64;
		if (!SetFilePointerEx(ifile, MirrorliDistanceToMove, nullptr, FILE_BEGIN))
		{
			bm_wattron(12);
			bm_wprintw("error SetFilePointerEx. code = %llu\n", GetLastError(), 0);
			bm_wattroff(12);
			*cont = true;
			return;
		}
		do {
			if (!ReadFile(ifile, &MirrorCache[*bytes], (DWORD)(min(readChunkSize, *cache_size_local - *bytes / 64) * 64), &Mirrorb, NULL))
			{
				bm_wattron(12);
				bm_wprintw("error P2 ReadFile. code = %llu\n", GetLastError(), 0);
				bm_wattroff(12);
				break;
			}
			*bytes += Mirrorb;
		} while (*bytes < *cache_size_local * 64);
	}
	if (*flip) goto poc1read;
readend:
	*flip = !*flip;

	//PoC2 Merge data to Cache
	if (p2 != POC2) {
		for (unsigned long t = 0; t < *bytes; t += 64) {
			memcpy(&cache[t + 32], &MirrorCache[t + 32], 32); //copy second hash to correct place.
		}
	}
}

void th_hash(t_files const * const iter, double * const sum_time_proc, const size_t &local_num, unsigned long long const bytes, size_t const cache_size_local, unsigned long long const i, unsigned long long const nonce, unsigned long long const n, char const * const cache, size_t const acc) {
	LARGE_INTEGER li;
	LARGE_INTEGER start_time_proc;
	QueryPerformanceCounter(&start_time_proc);
#ifdef __AVX2__
	procscoop_m256_8_fast(n + nonce + i, cache_size_local, cache, acc, iter->Name);// Process block AVX2
#else
#ifdef __AVX__
	procscoop_m_4(n + nonce + i, cache_size_local, cache, acc, iter->Name);// Process block AVX
#else
	procscoop_sph(n + nonce + i, cache_size_local, cache, acc, iter->Name);// Process block SSE4
#endif
#endif
	QueryPerformanceCounter(&li);
	*sum_time_proc += (double)(li.QuadPart - start_time_proc.QuadPart);
	worker_progress[local_num].Reads_bytes += bytes;
}

