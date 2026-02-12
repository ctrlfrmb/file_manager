#pragma once
#include"BaseProtocol.h"

class CanProtocol :public BaseProtocol
{
public:
	CanProtocol(void* drvhandle, uint8_t Chanl);
	~CanProtocol();
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
	int TrsmitWithNoResponse(uint8_t* data, int len, uint32_t Timeout);
	int SetIOCTLParam(void* pConf);
	int FastInit(void* pInput);
	//int FastInit(void* pInputMsg, void* pOutputMsg);
	//int ESB(void* pInputMsg, void* pOutputMsg);
	//int FsCheck(void* pInputMsg, void* pOutputMsg);
	//int FsClear(void* pInputMsg, void* pOutputMsg);
	//int FsWrite(void* pInputMsg, void* pOutputMsg);
	//int TswCfg(void* pInputMsg, void* pOutputMsg);
	//int TswStart(void* pInputMsg, void* pOutputMsg);
private:

	uint8_t WorkMode;
};