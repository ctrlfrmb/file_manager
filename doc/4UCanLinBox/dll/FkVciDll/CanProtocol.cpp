#include"pch.h"
#include <fstream>
#include"CanProtocol.h"
#include"BaseDevice.h"
#include"VciDebugCf.h"
#include"j2534_v0404.h"
CanProtocol::CanProtocol(void* drvhandle, uint8_t Chanl)
{
	debugout("CanProtocol Create ...\r\n");
	DeviceDrvHandle = drvhandle;
	ChanlNum = Chanl;
	WorkMode = 0;
	ProtocolID = ePROTOCOL_CAN;
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

CanProtocol::~CanProtocol()
{
	debugout("CanProtocol Delete ...\r\n");

}
/// <summary>
/// 
/// </summary>
/// <param name="msg"></param>
void CanProtocol::ProtocolDealMsg(void* msg)
{
	debugout("CanProtocol ProtocolDealMsg Chanl:%d...\r\n",ChanlNum);

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
int CanProtocol::TrsmitWithResponse(uint8_t* data, int len, uint32_t Timeout)
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

int CanProtocol::TrsmitWithNoResponse(uint8_t* data, int len, uint32_t Timeout)
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
		///*等待DEV返回数据并判定*/
		//if (WaitForSingleObjectEx(CommondEventHandle, Timeout, true) == WAIT_OBJECT_0)/*等待设备释放*/
		//{
		//	//读取buf中的数据并判定结果
		//	if (ResponseDataPtr->pData[0] != 0)
		//	{
		//		debugout("TrsmitWithResponse ErrCode:%02X\r\n", ResponseDataPtr->pData[0]);
		//		return ERRCODE_FAILED;
		//	}
		//	ResetEvent(CommondEventHandle);
		//}
		//else//等待超时
		//{
		//	debugout("TrsmitWithResponse WaitRspMsg TimeOut!\r\n");
		//	return ERRCODE_TIMEOUT;

		//}
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
int CanProtocol::ProtocolConnect(void* channelId, uint32_t Flags, uint32_t Baudrate)
{
	debugout("CanProtocol ProtocolConnect ...\r\n");

	uint8_t buf[256];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;

	ProtocolReqDptr->protocolid = ePROTOCOL_CAN;
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
int CanProtocol::ProtocolDisConnect()
{
	debugout("CanProtocol ProtocolDisConnect ...\r\n");

	uint8_t buf[256];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;

	ProtocolReqDptr->protocolid = ePROTOCOL_CAN;
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
int CanProtocol::ProtocolReadMsgs(void* pMsgs, uint32_t* pNumMsgs, uint32_t Timeout)
{
	debugout("CanProtocol ProtocolReadMsgs ...\r\n");
	uint32_t count = (uint32_t)GetRxMessageCount();/*从缓存中获取缓存数据条数*/
	if ((count == 0) && (Timeout == 0))/*缓存中没有数据且不等待 直接返回*/
	{
		*pNumMsgs = 0;
		return ERRCODE_BUFFER_EMPTY;
	}
	if (count >= (*pNumMsgs))/*缓存中有足够的数据供外部读取*/
	{
		FkVciDataType rxmsg;
		FkVciCanDataType* CanRxDptr = (FkVciCanDataType*)pMsgs;
		// we have (more than) enough of buffered messages, read just the amount requested
		debugout("Requeust Read: %d Buffer Num :%d ...\r\n", *pNumMsgs, count);
		for (unsigned long i = 0; i < (*pNumMsgs); i++)
		{
			PopMsg(&rxmsg);//做几个输出 Test ....
			memcpy(CanRxDptr, &rxmsg, sizeof(FkVciCanDataType));
			CanRxDptr++;//转换到下一个接收地址
		}
		return ERRCODE_NOERR;
	}
	else if ((Timeout == 0) && (count > 0))/*缓存中有数据 但不够请求数据条数 但不等待 直接输出缓存数据*/
	{
		FkVciDataType rxmsg;
		FkVciCanDataType* CanRxDptr = (FkVciCanDataType*)pMsgs;
		// we have (more than) enough of buffered messages, read just the amount requested
		debugout("Requeust Read: %d Buffer Num :%d ...\r\n", *pNumMsgs, count);
		for (unsigned long i = 0; i < (*pNumMsgs); i++)
		{
			PopMsg(&rxmsg);//做几个输出 Test ....
			memcpy(CanRxDptr, &rxmsg, sizeof(FkVciCanDataType));
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
			FkVciCanDataType* CanRxDptr = (FkVciCanDataType*)pMsgs;
			// we have (more than) enough of buffered messages, read just the amount requested
			debugout("Requeust Read: %d Buffer Num :%d ...\r\n", *pNumMsgs, count);
			for (unsigned long i = 0; i < (*pNumMsgs); i++)
			{
				PopMsg(&rxmsg);//做几个输出 Test ....
				memcpy(CanRxDptr, &rxmsg, sizeof(FkVciCanDataType));
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
int CanProtocol::ProtocolWriteMsgs(void* pMsg, uint32_t* pNumMsgs, uint32_t Timeout)
{
	debugout("CanProtocol ProtocolWriteMsgs ...\r\n");
	if (pMsg == NULL)
		return ERRCODE_INVALID_MSG;

	uint8_t buf[512];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	FkVciCanDataType* CanTxDptr = (FkVciCanDataType*)pMsg;
	int DisConnectRet = 0;
	ProtocolReqDptr->protocolid = ePROTOCOL_CAN;
	ProtocolReqDptr->protocolcmd = CMDCODE_TXMSG;
	ProtocolReqDptr->protocolchanl = ChanlNum;

	for (uint32_t i = 0; i < *pNumMsgs; i++)
	{
		ProtocolReqDptr->datalen = (CanTxDptr->DLC + 16);
		memcpy(ProtocolReqDptr->pData, CanTxDptr, (CanTxDptr->DLC + 16));//将要发送的数据拷贝过去
		DisConnectRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), Timeout);
		if (DisConnectRet != ERRCODE_NOERR)
		{
			*pNumMsgs = i;
			return DisConnectRet;
		}
		else//发送成功 转换到下一个发送
		{
			CanTxDptr++;
		}
	}
	return DisConnectRet;
}


/// <summary>
/// 
/// </summary>
/// <param name="pMsg"></param>
/// <param name="pMsgID"></param>
/// <param name="TimeInterval"></param>
/// <returns></returns>
int CanProtocol::ProtocolStartPeriodicMsg(void* pMsg, void* pMsgID, uint32_t TimeInterval)
{
	debugout("CanProtocol ProtocolStartPeriodicMsg ...\r\n");
	return ERRCODE_NOERR;
}
/// <summary>
/// 
/// </summary>
/// <param name="pMsgID"></param>
/// <returns></returns>
int CanProtocol::ProtocolStopPeriodicMsg(void* pMsgID)
{
	debugout("CanProtocol ProtocolStopPeriodicMsg ...\r\n");
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
int CanProtocol::ProtocolStartMsgFilter(unsigned long FilterType, void* pMaskMsg, void* pPatternMsg, void* pFlowControlMsg, unsigned long* pFilterID)
{
	debugout("CanProtocol ProtocolStartMsgFilter ...\r\n");
	return ERRCODE_NOERR;
}
/// <summary>
/// 
/// </summary>
/// <param name="FilterID"></param>
/// <returns></returns>
int CanProtocol::ProtocolStopMsgFilter(unsigned long FilterID)
{
	debugout("CanProtocol ProtocolStopMsgFilter ...\r\n");
	return ERRCODE_NOERR;
}
/// <summary>
/// 
/// </summary>
/// <param name="IoctlID"></param>
/// <param name="pInput"></param>
/// <param name="pOutput"></param>
/// <returns></returns>
int CanProtocol::ProtocolIOCTL(unsigned long IoctlID, void* pInput, void* pOutput)
{
	debugout("CanProtocol ProtocolIOCTL IoctlID=%d ...\r\n", IoctlID);

	switch (IoctlID)
	{
	case GET_CONFIG:/*获取配置 通过协议配置将本地配置信息返回*/
	{
		break;
	}
	case SET_CONFIG:/*设置配置 调用协议的配置*/
	{
		if (pInput == NULL)
		{
			return ERR_NULL_PARAMETER;
		}
		return SetIOCTLParam(pInput);
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
		return FastInit(NULL);
		break;
	default:
		//LOG(MAINFUNC, "CProtocol::IOCTL----- NOT SUPPORTED -----");
		return ERROR_NOT_SUPPORTED;
		break;
	}
	return STATUS_NOERROR;
}
/// <summary>
/// 
/// </summary>
/// <param name="pConf"></param>
/// <returns></returns>
int CanProtocol::SetIOCTLParam(void* pConf)
{
	VciConfigDataType* pConfig = (VciConfigDataType*)pConf;
	if (pConfig == NULL)
	{
		return ERR_NULL_PARAMETER;
	}
	else
	{
		switch (pConfig->Parameter)
		{
		case DATA_RATE:

			break;
		case NSEG1:

			break;
		case NSEG2:

			break;
		case DATA_RATE_BRS:

			break;
		case DSEG1:

			break;
		case DSEG2:

			break;
		//case P1_MIN: //

		//	break;

		default:
			return ERR_INVALID_IOCTL_VALUE;
			break;
		}
	}
	uint8_t buf[512];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	ProtocolReqDptr->protocolid = ePROTOCOL_CAN;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONF;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->rev = SET_CONFIG; //

	ProtocolReqDptr->pData[0] = (uint8_t)(pConfig->Parameter & 0xff);
	*(uint32_t*)((ProtocolReqDptr->pData + 1)) = pConfig->Value;
	ProtocolReqDptr->datalen = 5;

	int SetConfigRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 100);
	return SetConfigRet;
}



int CanProtocol::FastInit(void* pInput)
{
	uint8_t buf[512];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	ProtocolReqDptr->protocolid = ePROTOCOL_CAN;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONF;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->rev = FAST_INIT; //

	ProtocolReqDptr->datalen = 0;

	int SetConfigRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 100);
	return SetConfigRet;

}





