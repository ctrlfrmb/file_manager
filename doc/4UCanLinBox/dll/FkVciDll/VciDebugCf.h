#pragma once
#include "stdio.h"

//#define DEBUG 
#ifdef DEBUG
	#define debugout printf
#else
	#define debugout //
#endif // DEBUG
