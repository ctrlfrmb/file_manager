#include"pch.h"
#include "BaseProtocol.h"
#include "FkVciDll.h"
#include "VciDebugCf.h"
#include "BaseDevice.h"

BaseProtocol::BaseProtocol(void* drvhandle)
{
	debugout("BaseProtocol Create ...\r\n");
	DeviceDrvHandle = drvhandle;
	CommondEventHandle = NULL;
	DataEventHandle = NULL;
	ChanlNum = 0;
	ProtocolID = 0;
	memset(CommondCheckBuf,0,sizeof(CommondCheckBuf));
}

BaseProtocol::BaseProtocol()
{
	debugout("BaseProtocol Create ...\r\n");
	DeviceDrvHandle = NULL;
	CommondEventHandle = NULL;
	DataEventHandle = NULL;
	ChanlNum = 0;
	memset(CommondCheckBuf, 0, sizeof(CommondCheckBuf));
}

BaseProtocol::~BaseProtocol()
{
	debugout("BaseProtocol Delete ...\r\n");
	CloseHandle(CommondEventHandle);
	CloseHandle(DataEventHandle);
}

int BaseProtocol::ProtocolConnect(void* channelId, uint32_t Flags, uint32_t Baudrate)
{
	debugout("BaseProtocol ProtocolConnect ...\r\n");
	return ERRCODE_NOERR;
}
int BaseProtocol::ProtocolDisConnect()
{
	debugout("BaseProtocol ProtocolDisConnect ...\r\n");
	return ERRCODE_NOERR;
}
 int BaseProtocol::ProtocolReadMsgs(void* pMsgs, uint32_t* pNumMsgs, uint32_t Timeout)
 {
	 debugout("BaseProtocol ProtocolReadMsgs ...\r\n");
	 return ERRCODE_NOERR;
 }
 int BaseProtocol::ProtocolWriteMsgs(void* pMsg, uint32_t* pNumMsgs, uint32_t Timeout)
 {
	 debugout("BaseProtocol ProtocolWriteMsgs ...\r\n");
	 return ERRCODE_NOERR;
 }
 int BaseProtocol::ProtocolStartPeriodicMsg(void* pMsg, void* pMsgID, uint32_t TimeInterval)
 {
	 debugout("BaseProtocol ProtocolStartPeriodicMsg ...\r\n");
	 return ERRCODE_NOERR;
 }
 int BaseProtocol::ProtocolStopPeriodicMsg(void* pMsgID)
 {
	 debugout("BaseProtocol ProtocolStopPeriodicMsg ...\r\n");
	 return ERRCODE_NOERR;
 }
 int BaseProtocol::ProtocolStartMsgFilter(unsigned long FilterType, void* pMaskMsg, void* pPatternMsg, void* pFlowControlMsg, unsigned long* pFilterID)
 {
	 debugout("BaseProtocol ProtocolStartMsgFilter ...\r\n");
	 return ERRCODE_NOERR;
 }
 int BaseProtocol::ProtocolStopMsgFilter(unsigned long FilterID)
 {
	 debugout("BaseProtocol ProtocolStopMsgFilter ...\r\n");
	 return ERRCODE_NOERR;
 }
 int BaseProtocol::ProtocolIOCTL(unsigned long IoctlID, void* pInput, void* pOutput)
 {
	 debugout("BaseProtocol ProtocolIOCTL ...\r\n");
	 return ERRCODE_NOERR;
 }
 void BaseProtocol::ProtocolDealMsg(void* msg)
 {
	 debugout("BaseProtocol ProtocolDealMsg ...\r\n");
 }




/*每个协议通用的接口*/
//1.保存一条msg到缓存
 bool BaseProtocol::PushMsg(FkVciDataType Msg)
 {
	 queue_lock.lock();
	 if (MsgQueue.size() == MAX_RX_BUFFER_SIZE)//限制队列长度
	 {
		 //队列满了 先删除一条
		 MsgQueue.pop();/**/
	 }
	 MsgQueue.push(Msg);
	 queue_lock.unlock();
	 return true;
 }
//2.读取一条或多条msg到端口
 bool BaseProtocol::PopMsg(FkVciDataType* pMsg)
 {
	 queue_lock.lock();
	 if (MsgQueue.empty())
	 {
		 queue_lock.unlock();
		 return false;
	 }
	 else
	 {
		 *pMsg = MsgQueue.front();
		 MsgQueue.pop();/**/
		 queue_lock.unlock();
		 return true;
	 }
 }
//3.获取buf条数
 int BaseProtocol::GetRxMessageCount()
 {
	 return (int)(MsgQueue.size());
 }
//4.清空buf
 void BaseProtocol::ClearRxBuffer()
 {
	 queue_lock.lock();
	 while (!MsgQueue.empty())
	 {
		 MsgQueue.pop();/**/
	 }
	 queue_lock.unlock();
 }



