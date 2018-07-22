#pragma once

#pragma pack(1)
struct BFSPlotFile
{
	unsigned long long startNonce;
	unsigned int nonces;
	unsigned long long startPos;
	unsigned int status;
	unsigned int pos;
};
#pragma pack()

#pragma pack(1)
struct BFSTOC
{
	char version[4];
	unsigned int crc32;
	unsigned long long diskspace;
	unsigned long long id;
	unsigned long long reserved1;
	BFSPlotFile plotFiles[72];
	char reserved2[2048];	
};
#pragma pack()

extern BFSTOC bfsTOC;							// BFS Table of Contents
extern unsigned int bfsTOCOffset;				// 4k address of BFSTOC on harddisk. default = 5

bool LoadBFSTOC(std::string drive);				//read BFSTOC from drive
