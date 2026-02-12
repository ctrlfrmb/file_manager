#pragma once

#include <iostream>
#include <fstream>
using namespace std;
class CCRC32
{
public:
	CCRC32();
	~CCRC32();

	uint32_t FileCrc32Win32(const char* szFilename, uint32_t& dwCrc32);

protected:
	static inline void CalcCrc32(const uint8_t byte, uint32_t& dwCrc32);
	static uint32_t s_arrdwCrc32Table[256];
};
