#ifndef SEED_KEY_H
#define SEED_KEY_H

#ifdef SEEDKEY_EXPORTS
#define SEEDKEY_API extern "C" __declspec(dllexport)
#else
#define SEEDKEY_API extern "C" __declspec(dllimport)
#endif

SEEDKEY_API bool Vi_ComputeKeyFromSeed(char* seed,
	unsigned short sizeSeed, char* key, unsigned short maxSizeKey, unsigned short* sizeKey);

SEEDKEY_API bool Vi_ComputeKeyFromSeed_L2(char* seed, 
	unsigned short sizeSeed, char* key, unsigned short maxSizeKey, unsigned short* sizeKey);

SEEDKEY_API bool Vi_ComputeKeyFromSeed_L4(char* seed,
	unsigned short sizeSeed, char* key, unsigned short maxSizeKey, unsigned short* sizeKey);
#endif // SEED_KEY_H