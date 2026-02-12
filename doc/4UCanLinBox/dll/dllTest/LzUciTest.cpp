#include"LzUciApi.h"
#include"stdio.h"


void LzUciTestSequeue(void)
{
	if (K2000Start(0) != 0)
	{
		printf("K2000Start Fail..\r\n");

		K2000Close();
		return;
	}

	if (RawIni(250000, 0, 1, 1, 0, 0) != 0)
	{
		printf("RawIni Fail..\r\n");

		K2000Close();
		return;

	}

	////CANESB
	uint8_t EsbReadBack[16];
	//CANESB(0,0,8,5, EsbReadBack,3000);//进入ESB 



	//CANESB(0, 1, 8, 5, EsbReadBack, 10000);//等待读取ESB参数

	uint8_t ComTxBuf[300] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A};
	uint16_t ComRxLen=0;
	ComBlock(ComTxBuf, 10, ComTxBuf, &ComRxLen, 1, 5000);/*使用Com 发送数据*/

	K2000Ini(0xFE, 0, 3, 20, 500, 0, 500, 5, 300, 20, 20, 250000);

	ComBlock(ComTxBuf, 10, ComTxBuf, &ComRxLen, 1, 5000);/*使用Com 发送数据*/

	uint8_t SegmentHeader[8] = {0x36,0x00};
	uint8_t SegmentEnd[8] = {12};
	uint8_t PositiveResponse[8] = {0x76};
	if (TswFile("E:\\FigkeyProject\\RD-CANLIN\\Windows\\TSW", "tsw_013.hex", 0,//传输文件路径定义
		0, 0, //传输文件定义
		SegmentHeader, 1,//传输文件添加头信息
		SegmentEnd, 0, //传输文件添加尾部信息
		0xff,  //单次最大传输量
		PositiveResponse, 0)!=0)	
	{

		printf("TswFile Fail..\r\n");

		K2000Close();
		return;

	}

	K2000Close();

}