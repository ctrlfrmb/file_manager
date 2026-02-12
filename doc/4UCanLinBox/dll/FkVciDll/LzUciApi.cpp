#include"pch.h"
#include"FkVciDll.h"
#include <string.h>
//#include <tchar.h>
#include"j2534_v0404.h"
#include "CRC32.h"
#include <iostream>
#include <fstream>
#include "AcanProtocol.h"
using namespace std;

unsigned long DevIdGet;
unsigned long ActionChanlIdHandle;/*当前工作使用的ID*/
unsigned long AscChanlIdHandle;
unsigned long KlinChanlIdHandle;



HANDLE UciDeviceHandle;
HANDLE ChanlHandle;
uint8_t KwpRawFlag = 0;
long K2000Start(long offset)/*连接板卡*/
{
	UciDeviceHandle = FkVciOpenDev(0, offset,0);

	if (UciDeviceHandle == NULL)
		return ERR_DEVICE_NOT_CONNECTED;
	return STATUS_NOERROR;
}

long K2000Close()/*关闭板卡*/
{
	return FkVciCloseDev(UciDeviceHandle);
}

long RawIni(long BaudRate, uint8_t type, long P1Max, long P2Max, long P3Min, long P4Min)/*创建一个ASC或则K连接*/
{
	
	if (type == 0)/*ASC-CAN*/
	{
		if (AscChanlIdHandle == NULL)/*还没有初始化*/
		{
			ChanlHandle = FkVciInitACAN(UciDeviceHandle, 0, BaudRate);
			if (ChanlHandle == NULL)
				return ERR_DEVICE_NOT_CONNECTED;
		}
		//else/*协议已经打开了  直接使用*/
		//{
			
			SCONFIG Config;
			int ConfigRet;

			Config.Parameter = DATA_RATE;
			Config.Value = BaudRate;
			ConfigRet = FkVciConfigACAN(ChanlHandle, SET_CONFIG, &Config);
			if (ConfigRet != 0)
				return ConfigRet;

			Config.Parameter = P1_MAX;
			Config.Value = P1Max;
			ConfigRet = FkVciConfigACAN(ChanlHandle, SET_CONFIG ,&Config);
			if (ConfigRet != 0)
				return ConfigRet;

			Config.Parameter = P2_MAX;
			Config.Value = P2Max;
			ConfigRet = FkVciConfigACAN(ChanlHandle, SET_CONFIG ,&Config);
			if (ConfigRet != 0)
				return ConfigRet;

			Config.Parameter = P3_MIN;
			Config.Value = P3Min;
			ConfigRet = FkVciConfigACAN(ChanlHandle, SET_CONFIG ,&Config);
			if (ConfigRet != 0)
				return ConfigRet;

			Config.Parameter = P4_MIN;
			Config.Value = P4Min;
			ConfigRet = FkVciConfigACAN(ChanlHandle, SET_CONFIG, &Config);
			if (ConfigRet != 0)
				return ConfigRet;
			KwpRawFlag = 0;
		//}
	}
	else if (type == 1)/*Kline*/
	{
			return ERR_NOT_SUPPORTED;
	}
	return STATUS_NOERROR;
}

uint8_t KWP2000_CheckSum(uint8_t* Datas, uint16_t Len)
{
	uint16_t idx_u16;
	uint8_t checksum;
	uint32_t sunm;
	sunm = 0;
	for (idx_u16 = 0; idx_u16 < Len; idx_u16++)
	{
		sunm += Datas[idx_u16];
	}
	checksum = (uint8_t)(sunm & 0xff);
	return (checksum);
}
/*使用协议发送数据*/
long ComBlock(uint8_t* txdata, uint16_t txlen, uint8_t* rxdata, uint16_t* rxlen, uint8_t mode, uint32_t timeout)/*使用Com 发送数据*/
{
	uint8_t databuf[300];
	int VciTrsmitRet;
	/*先清空掉所有的接收数据*/
	FkVciConfigACAN(ChanlHandle, CLEAR_RX_BUFFER,NULL);
	/*再发送数据*/
	FkVciKwpDataType comblockData;
	memset(&comblockData,0,sizeof(FkVciKwpDataType));
	if (KwpRawFlag == 1)//使用KWP 模式发送 添加 头  支支持  00 DLC 模式
	{
		databuf[0] = 0x00;
		databuf[1] = (uint8_t)txlen;
		memcpy(databuf+2, txdata, txlen);
		databuf[txlen + 2] = KWP2000_CheckSum(databuf, (txlen+2));//KWP checkSum

		comblockData.DLC = txlen + 3;
		memcpy(comblockData.Data, databuf, comblockData.DLC);//将数据拷贝过去

		VciTrsmitRet = FkVciTrsmitACAN(ChanlHandle,&comblockData,1);
	}
	else //使用RAW 模式直接发送
	{
		comblockData.DLC = txlen;
		memcpy(comblockData.Data, txdata, comblockData.DLC);//将数据拷贝过去

		VciTrsmitRet = FkVciTrsmitACAN(ChanlHandle, &comblockData, 1);
	}
	if (VciTrsmitRet != 1)//FkVciTrsmitACAN 实际传输的报文数量
		return 0x10;//Tx Faile

	/*看是否需要接收*/
	int VciReciveRet;
	if (mode == 0)/*需要接收*/
	{
		VciReciveRet = FkVciRecieveACAN(ChanlHandle, &comblockData, 1, timeout);
		if (VciReciveRet != 1)
		{
			*rxlen = 0;
			return 0x20;//Rx Fail
		}
		else
		{
			if (rxdata != NULL)/*接收数据buf 不为空 将数据拷贝过去*/
			{
				if (KwpRawFlag == 1)//按照KWP 解析数据 ？？？
				{
					memcpy(rxdata, comblockData.Data, comblockData.DLC);
					*rxlen = (uint16_t)(comblockData.DLC);
				}
				else //直接接收数据
				{
					memcpy(rxdata, comblockData.Data, comblockData.DLC);
					*rxlen = (uint16_t)(comblockData.DLC);
				}
				
			}
			else
			{
				*rxlen = 0;
			}
			

		}
	}
	else/*不需要接收  */
	{

	}
	return 0;
}

long K2000Ini(uint8_t EcuAddr, uint8_t InitType, uint8_t InitLine, long P1max, long P2Max, long P3Min, long P3Max, long P4min, long TW5, long TiniL, long TWuP, long BaudRate)
{
	/*线初始化时间参数*/
	SCONFIG CfgItem[9];
	//SBYTE_ARRAY InputArray;
	CfgItem[0].Parameter = DATA_RATE;
	CfgItem[0].Value = BaudRate;
	CfgItem[1].Parameter = P1_MAX;
	CfgItem[1].Value = P1max;
	CfgItem[2].Parameter = P2_MAX;
	CfgItem[2].Value = P2Max;
	CfgItem[3].Parameter = P3_MIN;
	CfgItem[3].Value = P3Min;
	CfgItem[4].Parameter = P3_MAX;
	CfgItem[4].Value = P3Max;
	CfgItem[5].Parameter = P4_MIN;
	CfgItem[5].Value = P4min;
	CfgItem[6].Parameter = W5;
	CfgItem[6].Value = TW5;
	CfgItem[7].Parameter = TINIL;
	CfgItem[7].Value = TiniL;
	CfgItem[8].Parameter = TWUP;
	CfgItem[8].Value = TWuP;
	KwpRawFlag = 1;
	/*再初调用快速初始化*/
	if (InitLine == 3)/*ASC*/
	{
		if (ChanlHandle == NULL)/*协议还没有打开 打开协议*/
		{
			ChanlHandle = FkVciInitACAN(UciDeviceHandle, 0, BaudRate);
			if (ChanlHandle == NULL)
			{
					return ERR_DEVICE_NOT_CONNECTED;
			}
			else
			{

			}
		}
	}
	else if (InitLine == 0)/*K 线*/
	{
		//ActionChanlIdHandle = KlinChanlIdHandle;
		return ERR_NOT_SUPPORTED;

	}
	else/*其他总线不支持*/
	{
		return ERR_NOT_SUPPORTED;
	}
	//初始化参数
	for (int i = 0; i < 9; i++)
	{
		int ConfigRet = FkVciConfigACAN(ChanlHandle, SET_CONFIG, &CfgItem[i]);
		if (ConfigRet != STATUS_NOERROR)
			return ConfigRet;
	}
	/*进行对应的初始化*/
	if (InitType == 0)//No initialization. (This mode is used to set/alter communication parameters for the interface board without reestablishing)
	{
		return STATUS_NOERROR;
	}
	else if (InitType == 1)//5-baud initialization (is no longer supported. )
	{
		return ERR_NOT_SUPPORTED;
	}
	else if (InitType == 3)//Quick initialization
	{
		uint8_t buf[16];
		buf[0] = 0x81; /* Format(functional addressing, 1 byte payload), first Header byte */
		buf[1] = EcuAddr; /* Initialization address used to activate all ECUs, second Header byte */
		buf[2] = 0xF1; /* Scan Tool physical source address, third Header byte */
		buf[3] = 0x81; /* Start Communication Request Service Identifier, first Data byte */
		buf[4] = KWP2000_CheckSum(buf, 4);
		int ConfigRet = FkVciConfigACAN(ChanlHandle, FAST_INIT, buf);
		return ConfigRet;

	}
	else/*其他模式*/
	{
		return ERR_NOT_SUPPORTED;

	}
	return ERR_NOT_SUPPORTED;
}

long CANESB(uint8_t Variant, uint8_t Mode, uint32_t PatternCount, uint32_t ResponseLength, uint8_t*rddata, uint32_t Timeout)/*ESB 进入*/
{

	if (Variant != 0)/*Variant = 0  250K */
	{
		return ERR_NOT_SUPPORTED;
	}
	uint8_t buf[32];
	if (Mode == 0)/*启动ESB*/
	{		
		*(uint32_t*)buf = PatternCount;
		*(uint32_t*)(buf + 4) = ResponseLength;
		*(uint32_t*)(buf + 8) = Timeout;

		int ConfigRet = FkVciConfigACAN(ChanlHandle, CAN_ESB, &buf);
		return ConfigRet;
	}
	else if (Mode == 1)/*读取ESB结果*/
	{
		FkVciKwpDataType comblockData;
		int VciReciveRet = FkVciRecieveACAN(ChanlHandle, &comblockData, 1, Timeout);
		if (VciReciveRet != 0)
		{
			memcpy(rddata, comblockData.Data, comblockData.DLC);
			return STATUS_NOERROR;
		}
		else
		{
			return ERR_TIMEOUT;
		}
		
	}
	return ERR_NOT_SUPPORTED;
}

long TswFile(const char* FilePath, const char* FileName, uint8_t DataFormat,//传输文件路径定义
			uint32_t StartAddress, uint32_t BlockLength, //传输文件定义
			uint8_t* SegmentHeader, uint16_t SegmentHeaderLen,//传输文件添加头信息
			uint8_t* SegmentEnd, uint16_t SegmentEndlen, //传输文件添加尾部信息
	        uint16_t SegmentLength,  //单次最大传输量
	        uint8_t* PositiveResponse,uint16_t PostiveResponseLen)//单次传输后返回确认消息
{

	CCRC32 Crc32Handle;
	uint32_t Crc32Val;
	string PathString = FilePath;
	PathString += "\\";
	PathString += FileName;

	if (Crc32Handle.FileCrc32Win32(PathString.c_str(), Crc32Val) != 0)
	{
		return ERR_INVALID_FLAGS;/*文件路径不对 没有找到指定文件*/
	}
	uint8_t buf[512];
	memset(buf, 0, 512);
	*(uint32_t*)buf = Crc32Val;//
	strcpy_s((char*)(buf + 4),255, FileName);
	int ConfigRet;
	//FsCheckFormat
	 ConfigRet = FkVciConfigACAN(ChanlHandle, FS_CHECK, buf);//检验文件CRC 是否zhengque
	if (ConfigRet != STATUS_NOERROR)//
	{
		//FsClearFormat
		strcpy_s((char*)(buf), 255,FileName);//清空文件 下位机新建一个文件
		ConfigRet = FkVciConfigACAN(ChanlHandle, FS_CLEAR, buf);//检验文件CRC 是否zhengque
		if (ConfigRet != STATUS_NOERROR)//
		{
			return ConfigRet;
		}
		else//传输文件
		{
			//传递路径过去 FsWriteFormat
			//PathString
			strcpy_s((char*)(buf), 255,PathString.c_str());//清空文件 下位机新建一个文件
			ConfigRet = FkVciConfigACAN(ChanlHandle, FS_WRITE, buf);
			if (ConfigRet != STATUS_NOERROR)// 传输文件报错
			{
				return ConfigRet;
			}
		}
	}

	//FsTswCfgFormat
	FsTswCfgFormat *tswcfg = (FsTswCfgFormat*)buf;
	memset(tswcfg, 0, sizeof(FsTswCfgFormat));
	tswcfg->StartAddress = StartAddress;//传输开始地址
	tswcfg->BlockLength = BlockLength;//传输文件大小  0 为整个文件
	tswcfg->SegmentLength = SegmentLength;//
	tswcfg->SegmentHeaderLen = SegmentHeaderLen;
	memcpy(tswcfg->SegmentHeader, SegmentHeader, SegmentHeaderLen);//
	tswcfg->SegmentEndlen = SegmentEndlen;
	memcpy(tswcfg->SegmentEnd, SegmentEnd, SegmentEndlen);
	tswcfg->PostiveResponseLen = PostiveResponseLen;
	memcpy(tswcfg->PositiveResponse, PositiveResponse, PostiveResponseLen);

	ConfigRet = FkVciConfigACAN(ChanlHandle, TSW_CFG, buf);//下发TSW 配置
	if (ConfigRet != STATUS_NOERROR)//配置出错
	{
		return ConfigRet;
	}
	ConfigRet = FkVciConfigACAN(ChanlHandle, START_EXPROGSEQUEUE, buf);//启动刷写
	if (ConfigRet != STATUS_NOERROR)//配置出错
	{
		return ConfigRet;
	}

	return 0;
}


