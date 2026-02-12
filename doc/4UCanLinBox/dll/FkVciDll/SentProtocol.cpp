#include"pch.h"
#include <fstream>
#include"SentProtocol.h"
#include"BaseDevice.h"
#include"VciDebugCf.h"
#include"j2534_v0404.h"
SentProtocol::SentProtocol(void* drvhandle, uint8_t Chanl)
{
	debugout("SentProtocol Create ...\r\n");
	DeviceDrvHandle = drvhandle;
	ChanlNum = Chanl;
	ProtocolID = ePROTOCOL_SENT;
	DataEventHandle = CreateEventA(
		NULL,		// default security attributes
		TRUE,		// manual-reset event
		FALSE,		// initial state is nonsignaled
		NULL);		// object name
	CommondEventHandle = CreateEventA(
		NULL,		// default security attributes
		TRUE,		// manual-reset event
		FALSE,		// initial state is nonsignaled
		NULL);		// object name
}

SentProtocol::~SentProtocol()
{
	debugout("SentProtocol Delete ...\r\n");

}
/// <summary>
/// 
/// </summary>
/// <param name="msg"></param>
void SentProtocol::ProtocolDealMsg(void* msg)
{
	debugout("SentProtocol ProtocolDealMsg Chanl:%d...\r\n", ChanlNum);

	ProtocolDataType* ResponseDataPtr = (ProtocolDataType*)msg;//将接收到的数据转化为协议数据
	if (CommondCheckBuf[0] != ResponseDataPtr->protocolid || CommondCheckBuf[2] != ResponseDataPtr->protocolchanl)//如果不为该协议通道数据不做处理 直接返回
	{
		return;
	}

	/*验证是否为指令返回报文*/
	if (
		CommondCheckBuf[1] == ResponseDataPtr->protocolcmd //当前请求的Command
		)//当前请求的 通道
	{
		debugout("Recieve CommondCheck Msg...\r\n");
		memcpy(CommondCheckBuf, msg, (ResponseDataPtr->datalen + 6));
		SetEvent(CommondEventHandle);
	}
	else//协议数据
	{
		debugout("DEV主动发送的消息...\r\n");
		/*将数据打包成PassThruMsg 并存放到队列中*/
		if (
			CMDCODE_RXMSG == ResponseDataPtr->protocolcmd
			)/*PROTOCOL_ACAN 报文*/
		{
			debugout("Recevie One Can Msg...\r\n");

			//这里要添加 处理了  当前工作再KWP模式下 还是 RAW模式下  下位机一直工作在RAW 模式下

			FkVciDataType msgrx;
			memcpy((void*)&msgrx, ResponseDataPtr->pData, ResponseDataPtr->datalen);/*拷贝 数据到 FkVciDataType 中*/
			PushMsg(msgrx);
			SetEvent(DataEventHandle);
		}
	}
}
/// <summary>
/// 
/// </summary>
/// <param name="data"></param>
/// <param name="len"></param>
/// <param name="Timeout"></param>
/// <returns></returns>
int SentProtocol::TrsmitWithResponse(uint8_t* data, int len, uint32_t Timeout)
{
	if (DeviceDrvHandle != NULL)/*设备不为空*/
	{
		BaseDevice* dev = (BaseDevice*)DeviceDrvHandle;//获取硬件句柄
		ProtocolDataType* ResponseDataPtr = (ProtocolDataType*)CommondCheckBuf; //将缓存转换为协议数据
		ResetEvent(CommondEventHandle);//先清空掉之前的event请求
		ResponseDataPtr->protocolid = data[0];//协议ID
		ResponseDataPtr->protocolcmd = data[1];//协议功能
		ResponseDataPtr->protocolchanl = ChanlNum;//协议通道

		dev->DevTrsmit(data, len);
		/*等待DEV返回数据并判定*/
		if (WaitForSingleObjectEx(CommondEventHandle, Timeout, true) == WAIT_OBJECT_0)/*等待设备释放*/
		{
			//读取buf中的数据并判定结果
			if (ResponseDataPtr->pData[0] != 0)
			{
				debugout("TrsmitWithResponse ErrCode:%02X\r\n", ResponseDataPtr->pData[0]);
				return ERRCODE_FAILED;
			}
			ResetEvent(CommondEventHandle);
		}
		else//等待超时
		{
			debugout("TrsmitWithResponse WaitRspMsg TimeOut!\r\n");
			return ERRCODE_TIMEOUT;
		}
	}
	else
	{
		debugout("Device Is Null\r\n");
		return ERRCODE_INVALID_DEVICE_ID;

	}
	debugout("TrsmitWithResponse Sucess!\r\n");
	return ERRCODE_NOERR;
}

/// <summary>
/// 
/// </summary>
/// <param name="channelId"></param>
/// <param name="Flags"></param>
/// <param name="Baudrate"></param>
/// <returns></returns>
int SentProtocol::ProtocolConnect(void* channelId, uint32_t Flags, uint32_t Baudrate)
{
	debugout("SentProtocol ProtocolConnect ...\r\n");

	uint8_t buf[256];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;

	ProtocolReqDptr->protocolid = ePROTOCOL_SENT;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONNECT;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->datalen = 4;
	*(uint32_t*)ProtocolReqDptr->pData = Baudrate;

	int ConnectRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 100);
	return ConnectRet;
}
/// <summary>
/// 
/// </summary>
/// <returns></returns>
int SentProtocol::ProtocolDisConnect()
{
	debugout("SentProtocol ProtocolDisConnect ...\r\n");

	uint8_t buf[256];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;

	ProtocolReqDptr->protocolid = ePROTOCOL_SENT;
	ProtocolReqDptr->protocolcmd = CMDCODE_DISCONNECT;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->datalen = 0;
	int DisConnectRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 100);
	return DisConnectRet;
}
/// <summary>
/// 
/// </summary>
/// <param name="pMsgs"></param>
/// <param name="pNumMsgs"></param>
/// <param name="Timeout"></param>
/// <returns></returns>
int SentProtocol::ProtocolReadMsgs(void* pMsgs, uint32_t* pNumMsgs, uint32_t Timeout)
{
	debugout("SentProtocol ProtocolReadMsgs ...\r\n");
	uint32_t count = (uint32_t)GetRxMessageCount();/*从缓存中获取缓存数据条数*/
	if ((count == 0) && (Timeout == 0))/*缓存中没有数据且不等待 直接返回*/
	{
		*pNumMsgs = 0;
		return ERRCODE_BUFFER_EMPTY;
	}
	if (count >= (*pNumMsgs))/*缓存中有足够的数据供外部读取*/
	{
		FkVciDataType rxmsg;
		FkVciSentDataType* CanRxDptr = (FkVciSentDataType*)pMsgs;
		// we have (more than) enough of buffered messages, read just the amount requested
		debugout("Requeust Read: %d Buffer Num :%d ...\r\n", *pNumMsgs, count);
		for (unsigned long i = 0; i < (*pNumMsgs); i++)
		{
			PopMsg(&rxmsg);//做几个输出 Test ....
			memcpy(CanRxDptr, &rxmsg, sizeof(FkVciSentDataType));
			CanRxDptr++;//转换到下一个接收地址
		}
		return ERRCODE_NOERR;
	}
	else if ((Timeout == 0) && (count > 0))/*缓存中有数据 但不够请求数据条数 但不等待 直接输出缓存数据*/
	{
		FkVciDataType rxmsg;
		FkVciSentDataType* CanRxDptr = (FkVciSentDataType*)pMsgs;
		// we have (more than) enough of buffered messages, read just the amount requested
		debugout("Requeust Read: %d Buffer Num :%d ...\r\n", *pNumMsgs, count);
		for (unsigned long i = 0; i < (*pNumMsgs); i++)
		{
			PopMsg(&rxmsg);//做几个输出 Test ....
			memcpy(CanRxDptr, &rxmsg, sizeof(FkVciSentDataType));
			CanRxDptr++;//转换到下一个接收地址
		}
		return ERRCODE_NOERR;
	}
	else/*缓存数据不够  等待接收*/
	{
		debugout("Wait Recieve Msg......\r\n");
		ResetEvent(DataEventHandle);//使用阻塞
		if (WaitForSingleObjectEx(DataEventHandle, Timeout, true) == WAIT_OBJECT_0)/*等待设备释放*/
		{
			ResetEvent(DataEventHandle);//结束阻塞
		}

		count = GetRxMessageCount();
		if (count > 0)/*等待超时*/
		{
			FkVciDataType rxmsg;
			FkVciSentDataType* CanRxDptr = (FkVciSentDataType*)pMsgs;
			// we have (more than) enough of buffered messages, read just the amount requested
			debugout("Requeust Read: %d Buffer Num :%d ...\r\n", *pNumMsgs, count);
			for (unsigned long i = 0; i < (*pNumMsgs); i++)
			{
				PopMsg(&rxmsg);//做几个输出 Test ....
				memcpy(CanRxDptr, &rxmsg, sizeof(FkVciSentDataType));
				CanRxDptr++;//转换到下一个接收地址
			}
			return ERRCODE_NOERR;
		}
		else
			debugout("Time Out And Buffer Empty...\r\n");
		return ERRCODE_BUFFER_EMPTY;
	}
	return ERRCODE_NOERR;
}

/// <summary>
/// 
/// </summary>
/// <param name="pMsg"></param>
/// <param name="pNumMsgs"></param>
/// <param name="Timeout"></param>
/// <returns></returns>
int SentProtocol::ProtocolWriteMsgs(void* pMsg, uint32_t* pNumMsgs, uint32_t Timeout)
{
	debugout("SentProtocol ProtocolWriteMsgs ...\r\n");
	if (pMsg == NULL)
		return ERRCODE_INVALID_MSG;

	return ERRCODE_NOERR;
}


/// <summary>
/// 
/// </summary>
/// <param name="pMsg"></param>
/// <param name="pMsgID"></param>
/// <param name="TimeInterval"></param>
/// <returns></returns>
int SentProtocol::ProtocolStartPeriodicMsg(void* pMsg, void* pMsgID, uint32_t TimeInterval)
{
	debugout("SentProtocol ProtocolStartPeriodicMsg ...\r\n");
	return ERRCODE_NOERR;
}
/// <summary>
/// 
/// </summary>
/// <param name="pMsgID"></param>
/// <returns></returns>
int SentProtocol::ProtocolStopPeriodicMsg(void* pMsgID)
{
	debugout("SentProtocol ProtocolStopPeriodicMsg ...\r\n");
	return ERRCODE_NOERR;
}
/// <summary>
/// 
/// </summary>
/// <param name="FilterType"></param>
/// <param name="pMaskMsg"></param>
/// <param name="pPatternMsg"></param>
/// <param name="pFlowControlMsg"></param>
/// <param name="pFilterID"></param>
/// <returns></returns>
int SentProtocol::ProtocolStartMsgFilter(unsigned long FilterType, void* pMaskMsg, void* pPatternMsg, void* pFlowControlMsg, unsigned long* pFilterID)
{
	debugout("SentProtocol ProtocolStartMsgFilter ...\r\n");
	return ERRCODE_NOERR;
}
/// <summary>
/// 
/// </summary>
/// <param name="FilterID"></param>
/// <returns></returns>
int SentProtocol::ProtocolStopMsgFilter(unsigned long FilterID)
{
	debugout("SentProtocol ProtocolStopMsgFilter ...\r\n");
	return ERRCODE_NOERR;
}
/// <summary>
/// 
/// </summary>
/// <param name="IoctlID"></param>
/// <param name="pInput"></param>
/// <param name="pOutput"></param>
/// <returns></returns>
int SentProtocol::ProtocolIOCTL(unsigned long IoctlID, void* pInput, void* pOutput)
{
	debugout("SentProtocol ProtocolIOCTL IoctlID=%d ...\r\n", IoctlID);

	switch (IoctlID)
	{
	case GET_CONFIG:/*获取配置 通过协议配置将本地配置信息返回*/
	{
		break;
	}
	case SET_CONFIG:/*设置配置 调用协议的配置*/
	{
		break;
	}
	case CLEAR_TX_BUFFER:
		//ClearTXBuffer();
		break;
	case CLEAR_RX_BUFFER:
		ClearRxBuffer();
		break;
	case CLEAR_PERIODIC_MSGS:
		//return StopPeriodicMessages();
		break;
	case CLEAR_MSG_FILTERS:
		//return DeleteFilters();
		break;
	case FAST_INIT:
		break;
	default:
		//LOG(MAINFUNC, "CProtocol::IOCTL----- NOT SUPPORTED -----");
		return ERROR_NOT_SUPPORTED;
		break;
	}
	return STATUS_NOERROR;
}





