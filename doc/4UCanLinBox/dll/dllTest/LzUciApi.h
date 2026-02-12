#pragma once
#include"stdint.h"

long K2000Start(long offset);  /*连接板卡*/

long K2000Close();/*关闭板卡*/

long RawIni(long BaudRate, uint8_t type, long P1Max, long P2Max, long P3Min, long P4Min);

long ComBlock(uint8_t* txdata, uint16_t txlen, uint8_t* rxdata, uint16_t* rxlen, uint8_t mode, uint32_t timeout);/*使用Com 发送数据*/


long K2000Ini(uint8_t EcuAddr, uint8_t InitType, uint8_t InitLine, long P1max, long P2Max, long P3Min, long P3Max, long P4min, long TW5, long TiniL, long TWuP, long BaudRate);


long CANESB(uint8_t Variant, uint8_t Mode, uint32_t PatternCount, uint32_t ResponseLength, uint8_t* rddata, uint32_t Timeout);



long TswFile(const char* FilePath, const char* FileName, uint8_t DataFormat,//传输文件路径定义
	uint32_t StartAddress, uint32_t BlockLength, //传输文件定义
	uint8_t* SegmentHeader, uint16_t SegmentHeaderLen,//传输文件添加头信息
	uint8_t* SegmentEnd, uint16_t SegmentEndlen, //传输文件添加尾部信息
	uint16_t SegmentLength,  //单次最大传输量
	uint8_t* PositiveResponse, uint16_t PostiveResponseLen);//单次传输后返回确认消息