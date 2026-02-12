#pragma once
#include"BaseProtocol.h"

typedef struct CanEsbFormat
{
	uint32_t PatternCount;
	uint32_t ResponseLength;
	uint32_t Timeout;
}CanEsbFormat;
typedef struct FsCheckFormat
{
	uint32_t CRC;      //校验文件的CRC 值
	char filename[124];//检验文件的SD中名称
}FsCheckFormat;
typedef struct FsClearFormat
{
	char filename[124];//SD卡新建文件名称
}FsClearFormat;
typedef struct FsWriteFormat
{
	char filename[255];//PC 传输文件完整路径
}FsWriteFormat;
typedef struct FsTswCfgFormat
{
	uint32_t  StartAddress;//开始地址
	uint32_t  BlockLength;//传输文件长度
	uint16_t  SegmentLength;
	uint16_t  SegmentHeaderLen;
	uint8_t   SegmentHeader[16];
	uint16_t  SegmentEndlen;
	uint8_t   SegmentEnd[16];
	uint16_t  PostiveResponseLen;
	uint8_t  PositiveResponse[16];
}FsTswCfgFormat;



class AcanProtocol :public BaseProtocol
{
public:
	AcanProtocol(void* drvhandle,uint8_t Chanl);
	~AcanProtocol();
	virtual int ProtocolConnect(void* channelId, uint32_t Flags, uint32_t Baudrate);
	virtual int ProtocolDisConnect();
	virtual int ProtocolReadMsgs(void* pMsgs, uint32_t* pNumMsgs, uint32_t Timeout);/*所有协议公用*/
	virtual int ProtocolWriteMsgs(void* pMsg, uint32_t* pNumMsgs, uint32_t Timeout);
	virtual int ProtocolStartPeriodicMsg(void* pMsg, void* pMsgID, uint32_t TimeInterval);
	virtual int ProtocolStopPeriodicMsg(void* pMsgID);
	virtual int ProtocolStartMsgFilter(unsigned long FilterType, void* pMaskMsg, void* pPatternMsg, void* pFlowControlMsg, unsigned long* pFilterID);
	virtual int ProtocolStopMsgFilter(unsigned long FilterID);
	virtual int ProtocolIOCTL(unsigned long IoctlID, void* pInput, void* pOutput);
	virtual void ProtocolDealMsg(void* msg);/*处理接收数据  每个协议不一样*/

	int TrsmitWithResponse(uint8_t* data, int len, uint32_t Timeout);
	int SetIOCTLParam(void* pConf);
	int FastInit(void* pInputMsg, void* pOutputMsg);
	int ESB(void* pInputMsg, void* pOutputMsg);
	int FsCheck(void* pInputMsg, void* pOutputMsg);
	int FsClear(void* pInputMsg, void* pOutputMsg);
	int FsWrite(void* pInputMsg, void* pOutputMsg);
	int TswCfg(void* pInputMsg, void* pOutputMsg);
	int TswStart(void* pInputMsg, void* pOutputMsg);
private:

	uint8_t WorkMode;
};