#pragma once
#include"windows.h"
#include<queue>
#include<mutex>
#include"FkVciDll.h"
using namespace std;



#define MAX_RX_BUFFER_SIZE 1024


enum PROTOCOL
{
	ePROTOCOL_CAN = 1,
	ePROTOCOL_ISO = 2,
	ePROTOCOL_ASC = 3,
	ePROTOCOL_EXPROG = 4,
	ePROTOCOL_LIN = 5,
	ePROTOCOL_SENT = 6,
	ePROTOCOL_DEV = 0xf
};
enum CMD
{
	CMDCODE_CONNECT = 0x10,
	CMDCODE_DISCONNECT = 0x20,
	CMDCODE_TXMSG = 0x30,
	CMDCODE_RXMSG = 0x40,
	CMDCODE_ERRMSG = 0x50,
	CMDCODE_CONF = 0x60,
};

typedef struct ProtocolDataType
{
	uint8_t protocolid;
	uint8_t protocolcmd;
	uint8_t protocolchanl;
	uint8_t rev;
	uint16_t datalen;
	uint8_t pData[5000];
}ProtocolDataType;


typedef struct _VciConfigDataType
{
	uint32_t Parameter;		// Name of parameter
	uint32_t Value;			// Value of the parameter
} VciConfigDataType;


//typedef struct _VciCanConfigDataType
//{
//	uint32_t Parameter;		// Name of parameter
//	uint32_t CanNormalBaud;	// Value of the parameter
//	uint32_t CanDataBaud;   //
//	uint32_t CanFilterStartId;
//	uint32_t CanFilterStopId;
//} VciCanConfigDataType;


class BaseProtocol
{
public:

	BaseProtocol(void *drvhandle);
	BaseProtocol();
	~BaseProtocol();

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


	/*每个协议通用的接口*/
	//1.保存一条msg到缓存
	bool PushMsg(FkVciDataType Msg);	// takes ownership
	//2.读取一条或多条msg到端口
	bool PopMsg(FkVciDataType* pMsg);	// takes ownership
	//3.获取buf条数
	int GetRxMessageCount();
	//4.清空buf
	void ClearRxBuffer();

	/*消息队列*/
	queue<FkVciDataType> MsgQueue;
	mutex  queue_lock;/*在读取的时候 不能写入 在写入的时候不能读取*/

	/*指令数据缓存区*/
	HANDLE CommondEventHandle;          //接收到DEV返回指令的事件
	HANDLE DataEventHandle;          //接收到DEV返回指令的事件
	char CommondCheckBuf[128];
	/*硬件通道指针*/
	HANDLE DeviceDrvHandle;             //协议对应的硬件通道

	uint8_t ChanlNum;                   //多通道时候 需要验证对应通道是否已经打开了

	uint8_t ProtocolID;
	 
private:

};
