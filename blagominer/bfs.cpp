#include "bfs.h"

bool LoadBFSTOC(std::string drive, unsigned int bfsTOCOffset, BFSTOC *bfsTOC) {
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
	if (!ReadFile(iFile, bfsTOC, (DWORD)(4096), &b, NULL))
	{
		return false;
	}
	//close drive
	CloseHandle(iFile);
	//check if red content is really a BFSTOC
	char version[4] = { 'B', 'F', 'S', '1' };
	if (*bfsTOC->version != *version) {
		return false;
	}
	//success
	return true;
}