#pragma once
#include <string>
#include <windows.h> //windows API

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

//Read BFSTOC from drive
bool LoadBFSTOC(std::string drive, unsigned int bfsTOCOffset, BFSTOC *bfsTOC);
