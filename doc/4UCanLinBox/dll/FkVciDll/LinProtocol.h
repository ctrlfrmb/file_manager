#pragma once
#include"BaseProtocol.h"

class LinProtocol :public BaseProtocol
{
public:
	LinProtocol(void* drvhandle, uint8_t Chanl);
	~LinProtocol();
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
	int SchWriteMsg(void* pMsg);
	int SchStart();
	int SchStop();

	uint8_t LinSchRunFlag;
private:

	
};