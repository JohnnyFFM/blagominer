#include "stdafx.h"
#include "bfs.h"

BFSTOC bfsTOC;							// BFS Table of Contents
unsigned int bfsTOCOffset = 5;			// 4k address of BFSTOC on harddisk. default = 5

bool LoadBFSTOC(std::string drive) {
	//open drive
	HANDLE iFile = CreateFileA(drive.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, nullptr);
	if (iFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	//seek to bfstoc start
	LARGE_INTEGER liDistanceToMove;
	liDistanceToMove.QuadPart = bfsTOCOffset * 4096;
	if (!SetFilePointerEx(iFile, liDistanceToMove, nullptr, FILE_BEGIN))
	{
		return false;
	}
	//read bfstoc sector
	DWORD b;
	if (!ReadFile(iFile, &bfsTOC, (DWORD)(4096), &b, NULL))
	{
		return false;
	}
	//close drive
	CloseHandle(iFile);
	//check if red content is really a BFSTOC
	char bfsversion[4] = { 'B', 'F', 'S', '1' };

	if (*bfsTOC.version != *bfsversion) {
		return false;	}
	//success
	return true;
}