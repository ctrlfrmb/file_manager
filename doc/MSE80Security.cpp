#include"pch.h"
#include "stdint.h"


#define BITM(n,pos) ((((n>>(31-(pos&0xF)))&1)<<1)|((n>>(pos&0xF))&1))
unsigned short kwp2000_seed_crc(char* crcVarPtr, char* crcEndPtr);
uint32_t UAES_Calc_Key(uint32_t seed)
{
	unsigned char i; // uint8 i;
	unsigned char loops; //uint8 loops;
	unsigned short tmp; //uint16 tmp;
	unsigned long mask0 = 0xDEADBEAF;
	unsigned long mask1 = 0x20170822;
	/* Calculates the key from a seed */
	/* calculate the CRC checksum from the seed. Just to convert the seed to another value */
	tmp = kwp2000_seed_crc((char*)&seed, (char*)(&seed) + 3);
	/* determine the number of loops dependig on seed's crc value */
	loops = ((unsigned char)tmp >> 8) + ((unsigned char)tmp & 0xFF);
	loops = (loops & 0x0F) + 1;
	/* Do the Loop calculation */
	for (i = 0; i < loops; i++)
	{
		switch (BITM(seed, i)) /* seed = 32Bit */
		{
		case 0:
		{
			seed = (seed << 1) & 0xFFFFFFFE;
			seed ^= (~mask0);
			break;
		}
		case 1:
		{
			seed += mask0;
			break;
		}
		case 2:
		{
			seed = (seed << 1) | 0x01;
			seed ^= mask1;
			break;
		}
		default: /* (3) */
		{
			seed += (~mask1);
			break;
		}
		}
	}
	return seed;
}
unsigned short kwp2000_seed_crc(char* crcVarPtr, char* crcEndPtr)
{
	register unsigned char i;
	register unsigned short var;
	register unsigned short crc16_w = 0xFFEE;
	while (crcVarPtr <= crcEndPtr)
	{
		var = ((unsigned short)*crcVarPtr++) << 8;
		for (i = 0; i < 8; i++) /* calculate polynomian */
		{
			if ((crc16_w ^ var) & 0x8000) /* MSB set ? */
			{
				crc16_w = (crc16_w << 1) ^ 0xA001;
			}
			else
			{
				crc16_w = crc16_w << 1;
			}
			var <<= 1;
		}
	}
	return(crc16_w);
}