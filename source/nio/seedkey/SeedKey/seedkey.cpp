#include "seedkey.h"

static unsigned char crc8(unsigned char* data, int length) //function of calculate the Key
{
    unsigned char t_crc;
    int f, b;
    t_crc = 0xFF;
    for (f = 0; f < length; f++)
    {
        t_crc ^= data[f];
        for (b = 0; b < 8; b++)
        {
            if ((t_crc & 0x80) != 0)
            {
                t_crc <<= 1;
                t_crc ^= 0x1D;
            }
            else
            {
                t_crc <<= 1;
            }
        }
    }
    return ~t_crc;
}

bool Vi_ComputeKeyFromSeed(char* seed, unsigned
    short sizeSeed, char* key, unsigned short maxSizeKey, unsigned short* sizeKey)
{
    //L2 0xCDE5 L4 0x6EA3
    int seedlength = 6;
    unsigned char buf_byte[6];
    unsigned char crc_byte[7];
    seed[4] = static_cast<char>(0x12);//high byte of secret code(example)
    seed[5] = static_cast<char>(0x34);//low byte of secret code(example)
    buf_byte[0] = seed[0];
    buf_byte[1] = seed[1];
    buf_byte[2] = seed[2];
    buf_byte[3] = seed[3];
    buf_byte[4] = seed[4];
    buf_byte[5] = seed[5];
    crc_byte[0] = crc8(buf_byte, seedlength);
    buf_byte[0] = crc_byte[0];
    crc_byte[1] = crc8(buf_byte, seedlength);
    buf_byte[0] = seed[0];
    buf_byte[1] = crc_byte[1];
    crc_byte[2] = crc8(buf_byte, seedlength);
    buf_byte[1] = seed[1];
    buf_byte[2] = crc_byte[2];
    crc_byte[3] = crc8(buf_byte, seedlength);
    buf_byte[2] = seed[2];
    buf_byte[3] = crc_byte[3];
    crc_byte[4] = crc8(buf_byte, seedlength);
    buf_byte[3] = seed[3];
    buf_byte[4] = crc_byte[4];
    crc_byte[5] = crc8(buf_byte, seedlength);
    buf_byte[4] = seed[4];
    buf_byte[5] = crc_byte[5];
    crc_byte[6] = crc8(buf_byte, seedlength);
    if (crc_byte[3] == 0 && crc_byte[4] == 0 && crc_byte[5] == 0 && crc_byte[6] == 0)
    {
        key[0] = crc_byte[1];
        key[1] = crc_byte[2];
        key[2] = crc_byte[3];
        key[3] = crc_byte[4];
    }
    else
    {
        key[0] = crc_byte[3];
        key[1] = crc_byte[4];
        key[2] = crc_byte[5];
        key[3] = crc_byte[6];
    }
    *sizeKey = 4;
    // If the return value is false the flash tool stops
    return true;
}

bool Vi_ComputeKeyFromSeed_L2(char* seed, unsigned
    short sizeSeed, char* key, unsigned short maxSizeKey, unsigned short* sizeKey)
{
    //L2 0xCDE5 L4 0x6EA3
    int seedlength = 6;
    unsigned char buf_byte[6];
    unsigned char crc_byte[7];
    seed[4] = static_cast<char>(0xCD);//high byte of secret code(example)
    seed[5] = static_cast<char>(0xE5);//low byte of secret code(example)
    buf_byte[0] = seed[0];
    buf_byte[1] = seed[1];
    buf_byte[2] = seed[2];
    buf_byte[3] = seed[3];
    buf_byte[4] = seed[4];
    buf_byte[5] = seed[5];
    crc_byte[0] = crc8(buf_byte, seedlength);
    buf_byte[0] = crc_byte[0];
    crc_byte[1] = crc8(buf_byte, seedlength);
    buf_byte[0] = seed[0];
    buf_byte[1] = crc_byte[1];
    crc_byte[2] = crc8(buf_byte, seedlength);
    buf_byte[1] = seed[1];
    buf_byte[2] = crc_byte[2];
    crc_byte[3] = crc8(buf_byte, seedlength);
    buf_byte[2] = seed[2];
    buf_byte[3] = crc_byte[3];
    crc_byte[4] = crc8(buf_byte, seedlength);
    buf_byte[3] = seed[3];
    buf_byte[4] = crc_byte[4];
    crc_byte[5] = crc8(buf_byte, seedlength);
    buf_byte[4] = seed[4];
    buf_byte[5] = crc_byte[5];
    crc_byte[6] = crc8(buf_byte, seedlength);
    if (crc_byte[3] == 0 && crc_byte[4] == 0 && crc_byte[5] == 0 && crc_byte[6] == 0)
    {
        key[0] = crc_byte[1];
        key[1] = crc_byte[2];
        key[2] = crc_byte[3];
        key[3] = crc_byte[4];
    }
    else
    {
        key[0] = crc_byte[3];
        key[1] = crc_byte[4];
        key[2] = crc_byte[5];
        key[3] = crc_byte[6];
    }
    *sizeKey = 4;
    // If the return value is false the flash tool stops
    return true;
}


bool Vi_ComputeKeyFromSeed_L4(char* seed, unsigned
    short sizeSeed, char* key, unsigned short maxSizeKey, unsigned short* sizeKey)
{
    //L2 0xCDE5 L4 0x6EA3
    int seedlength = 6;
    unsigned char buf_byte[6];
    unsigned char crc_byte[7];
    seed[4] = static_cast<char>(0x6E);//high byte of secret code(example)
    seed[5] = static_cast<char>(0xA3);//low byte of secret code(example)
    buf_byte[0] = seed[0];
    buf_byte[1] = seed[1];
    buf_byte[2] = seed[2];
    buf_byte[3] = seed[3];
    buf_byte[4] = seed[4];
    buf_byte[5] = seed[5];
    crc_byte[0] = crc8(buf_byte, seedlength);
    buf_byte[0] = crc_byte[0];
    crc_byte[1] = crc8(buf_byte, seedlength);
    buf_byte[0] = seed[0];
    buf_byte[1] = crc_byte[1];
    crc_byte[2] = crc8(buf_byte, seedlength);
    buf_byte[1] = seed[1];
    buf_byte[2] = crc_byte[2];
    crc_byte[3] = crc8(buf_byte, seedlength);
    buf_byte[2] = seed[2];
    buf_byte[3] = crc_byte[3];
    crc_byte[4] = crc8(buf_byte, seedlength);
    buf_byte[3] = seed[3];
    buf_byte[4] = crc_byte[4];
    crc_byte[5] = crc8(buf_byte, seedlength);
    buf_byte[4] = seed[4];
    buf_byte[5] = crc_byte[5];
    crc_byte[6] = crc8(buf_byte, seedlength);
    if (crc_byte[3] == 0 && crc_byte[4] == 0 && crc_byte[5] == 0 && crc_byte[6] == 0)
    {
        key[0] = crc_byte[1];
        key[1] = crc_byte[2];
        key[2] = crc_byte[3];
        key[3] = crc_byte[4];
    }
    else
    {
        key[0] = crc_byte[3];
        key[1] = crc_byte[4];
        key[2] = crc_byte[5];
        key[3] = crc_byte[6];
    }
    *sizeKey = 4;
    // If the return value is false the flash tool stops
    return true;
}