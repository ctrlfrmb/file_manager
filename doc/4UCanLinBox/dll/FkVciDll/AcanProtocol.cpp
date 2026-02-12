#include"pch.h"
#include <fstream>
#include"AcanProtocol.h"
#include"BaseDevice.h"
#include"VciDebugCf.h"
#include"j2534_v0404.h"
AcanProtocol::AcanProtocol(void* drvhandle, uint8_t Chanl)
{
	debugout("AcanProtocol Create ...\r\n");
	DeviceDrvHandle = drvhandle;
	ChanlNum = Chanl;
	WorkMode = 0;
	ProtocolID = ePROTOCOL_ASC;
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

AcanProtocol::~AcanProtocol()
{
	debugout("AcanProtocol Delete ...\r\n");

}
/// <summary>
/// 
/// </summary>
/// <param name="msg"></param>
void AcanProtocol::ProtocolDealMsg(void* msg)
{
	debugout("AcanProtocol ProtocolDealMsg ...\r\n");

	ProtocolDataType* ResponseDataPtr = (ProtocolDataType*)msg;//将接收到的数据转化为协议数据
	/*验证是否为指令返回报文*/
	if (CommondCheckBuf[0] == ResponseDataPtr->protocolid //当前请求的协议
	 && CommondCheckBuf[1] == ResponseDataPtr->protocolcmd //当前请求的Command
	 && CommondCheckBuf[2] == ResponseDataPtr->protocolchanl)//当前请求的 通道
	{
		debugout("Recieve CommondCheck Msg...\r\n");
		memcpy(CommondCheckBuf, msg, (ResponseDataPtr->datalen+6));
		SetEvent(CommondEventHandle);
	}
	else//协议数据
	{
		debugout("DEV主动发送的消息...\r\n");
		/*将数据打包成PassThruMsg 并存放到队列中*/		
		if (ePROTOCOL_ASC == ResponseDataPtr->protocolid
			&& CMDCODE_RXMSG == ResponseDataPtr->protocolcmd
			&& ChanlNum == ResponseDataPtr->protocolchanl )/*PROTOCOL_ACAN 报文*/
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
int AcanProtocol::TrsmitWithResponse(uint8_t* data, int len, uint32_t Timeout)
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
int AcanProtocol::ProtocolConnect(void* channelId, uint32_t Flags, uint32_t Baudrate)
{
	debugout("AcanProtocol ProtocolConnect ...\r\n");

	uint8_t buf[256];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;

	ProtocolReqDptr->protocolid = ePROTOCOL_ASC;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONNECT;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->datalen = 4;
	*(uint32_t*)ProtocolReqDptr->pData = Baudrate;

	int ConnectRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6),100);
	return ConnectRet;
}
/// <summary>
/// 
/// </summary>
/// <returns></returns>
int AcanProtocol::ProtocolDisConnect()
{
	debugout("AcanProtocol ProtocolDisConnect ...\r\n");

	uint8_t buf[256];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;

	ProtocolReqDptr->protocolid = ePROTOCOL_ASC;
	ProtocolReqDptr->protocolcmd = CMDCODE_DISCONNECT;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->datalen = 0;
	int DisConnectRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6),100);
	return DisConnectRet;
}
/// <summary>
/// 
/// </summary>
/// <param name="pMsgs"></param>
/// <param name="pNumMsgs"></param>
/// <param name="Timeout"></param>
/// <returns></returns>
int AcanProtocol::ProtocolReadMsgs(void* pMsgs, uint32_t* pNumMsgs, uint32_t Timeout)
{
	debugout("AcanProtocol ProtocolReadMsgs ...\r\n");
	uint32_t count = (uint32_t)GetRxMessageCount();/*从缓存中获取缓存数据条数*/
	if ((count == 0) && (Timeout == 0))/*缓存中没有数据且不等待 直接返回*/
	{
		*pNumMsgs = 0;
		return ERRCODE_BUFFER_EMPTY;
	}
	if (count >= (*pNumMsgs))/*缓存中有足够的数据供外部读取*/
	{
		FkVciDataType rxmsg;
		FkVciKwpDataType* KwpRxDptr = (FkVciKwpDataType*)pMsgs;
		// we have (more than) enough of buffered messages, read just the amount requested
		debugout("Requeust Read: %d Buffer Num :%d ...\r\n", *pNumMsgs, count);
		for (unsigned long i = 0; i < (*pNumMsgs); i++)
		{
			PopMsg(&rxmsg);//做几个输出 Test ....
			memcpy(KwpRxDptr, &rxmsg, sizeof(FkVciKwpDataType));
			KwpRxDptr++;//转换到下一个接收地址
		}
		return ERRCODE_NOERR;
	}
	else if ((Timeout == 0) && (count > 0))/*缓存中有数据 但不够请求数据条数 但不等待 直接输出缓存数据*/
	{
		FkVciDataType rxmsg;
		FkVciKwpDataType* KwpRxDptr = (FkVciKwpDataType*)pMsgs;
		// we have (more than) enough of buffered messages, read just the amount requested
		debugout("Requeust Read: %d Buffer Num :%d ...\r\n", *pNumMsgs, count);
		for (unsigned long i = 0; i < (*pNumMsgs); i++)
		{
			PopMsg(&rxmsg);//做几个输出 Test ....
			memcpy(KwpRxDptr, &rxmsg, sizeof(FkVciKwpDataType));
			KwpRxDptr++;//转换到下一个接收地址
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
			FkVciKwpDataType* KwpRxDptr = (FkVciKwpDataType*)pMsgs;
			// we have (more than) enough of buffered messages, read just the amount requested
			debugout("Requeust Read: %d Buffer Num :%d ...\r\n", *pNumMsgs, count);
			for (unsigned long i = 0; i < (*pNumMsgs); i++)
			{
				PopMsg(&rxmsg);//做几个输出 Test ....
				memcpy(KwpRxDptr, &rxmsg, sizeof(FkVciKwpDataType));
				KwpRxDptr++;//转换到下一个接收地址
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
int AcanProtocol::ProtocolWriteMsgs(void* pMsg, uint32_t* pNumMsgs, uint32_t Timeout)
{
	debugout("AcanProtocol ProtocolWriteMsgs ...\r\n");
	if (pMsg == NULL)
		return ERRCODE_INVALID_MSG;

	uint8_t buf[512];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	FkVciKwpDataType* KwpTxDptr = (FkVciKwpDataType*)pMsg;
	int DisConnectRet = 0;
	ProtocolReqDptr->protocolid = ePROTOCOL_ASC;
	ProtocolReqDptr->protocolcmd = CMDCODE_TXMSG;
	ProtocolReqDptr->protocolchanl = ChanlNum;

	for (uint32_t i = 0; i < *pNumMsgs; i++)
	{
		ProtocolReqDptr->datalen = (KwpTxDptr->DLC + 12);
		memcpy(ProtocolReqDptr->pData, KwpTxDptr, (KwpTxDptr->DLC + 12));//将要发送的数据拷贝过去
		DisConnectRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), Timeout);
		if (DisConnectRet != ERRCODE_NOERR)
		{
			*pNumMsgs = i;
			return DisConnectRet;
		}
		else//发送成功 转换到下一个发送
		{
			KwpTxDptr++;
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
int AcanProtocol::ProtocolStartPeriodicMsg(void* pMsg, void* pMsgID, uint32_t TimeInterval)
{
	debugout("AcanProtocol ProtocolStartPeriodicMsg ...\r\n");
	return ERRCODE_NOERR;
}
/// <summary>
/// 
/// </summary>
/// <param name="pMsgID"></param>
/// <returns></returns>
int AcanProtocol::ProtocolStopPeriodicMsg(void* pMsgID)
{
	debugout("AcanProtocol ProtocolStopPeriodicMsg ...\r\n");
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
int AcanProtocol::ProtocolStartMsgFilter(unsigned long FilterType, void* pMaskMsg, void* pPatternMsg, void* pFlowControlMsg, unsigned long* pFilterID)
{
	debugout("AcanProtocol ProtocolStartMsgFilter ...\r\n");
	return ERRCODE_NOERR;
}
/// <summary>
/// 
/// </summary>
/// <param name="FilterID"></param>
/// <returns></returns>
int AcanProtocol::ProtocolStopMsgFilter(unsigned long FilterID)
{
	debugout("AcanProtocol ProtocolStopMsgFilter ...\r\n");
	return ERRCODE_NOERR;
}
/// <summary>
/// 
/// </summary>
/// <param name="IoctlID"></param>
/// <param name="pInput"></param>
/// <param name="pOutput"></param>
/// <returns></returns>
int AcanProtocol::ProtocolIOCTL(unsigned long IoctlID, void* pInput, void* pOutput)
{
	debugout("AcanProtocol ProtocolIOCTL IoctlID=%d ...\r\n", IoctlID);

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
	case FAST_INIT:/*快速初始化*/
	{
		if (pInput == NULL)
		{
			return ERR_NULL_PARAMETER;
		}
		return FastInit(pInput,NULL);
	}
	case CAN_ESB:/*ESB PAV 进入*/
		if (pInput == NULL)
		{
			return ERR_NULL_PARAMETER;
		}
		return ESB(pInput, NULL);
		break;
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

	case FS_CHECK:
		if (pInput == NULL)
		{
			return ERR_NULL_PARAMETER;
		}
		return FsCheck(pInput, NULL);
		break;
	case FS_CLEAR:
		if (pInput == NULL)
		{
			return ERR_NULL_PARAMETER;
		}
		return FsClear(pInput, NULL);
		break;
	case FS_WRITE:
		if (pInput == NULL)
		{
			return ERR_NULL_PARAMETER;
		}
		return FsWrite(pInput, NULL);
		break;
	case TSW_CFG:
		if (pInput == NULL)
		{
			return ERR_NULL_PARAMETER;
		}
		return TswCfg(pInput, NULL);
		break; 
	case START_EXPROGSEQUEUE:

		return TswStart(pInput, NULL);
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
int AcanProtocol::SetIOCTLParam(void* pConf)
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
		case P1_MIN:

			break;
		case P1_MAX:

			break;
		case P2_MIN:

			break;
		case P2_MAX:

			break;
		case P3_MIN:

			break;
		case P3_MAX:

			break;
		case P4_MIN:

			break;
		case P4_MAX:

			break;
		case W1:

			break;
		case W2:

			break;
		case W3:

			break;
		case W4:

			break;
		case W5:

			break;
		case TIDLE:

			break;
		case TINIL:

			break;
		case TWUP:

			break;
		default:
			return ERR_INVALID_IOCTL_VALUE;
			break;
		}
	}
	uint8_t buf[512];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	ProtocolReqDptr->protocolid = ePROTOCOL_ASC;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONF;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->rev = SET_CONFIG; //

	ProtocolReqDptr->pData[0] = (uint8_t)(pConfig->Parameter&0xff);
	*(uint32_t*)((ProtocolReqDptr->pData + 1)) = pConfig->Value;
	ProtocolReqDptr->datalen = 5;

	int SetConfigRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 100);
	return SetConfigRet;
}

/// <summary>
/// 
/// </summary>
/// <param name="pInputMsg"></param>
/// <param name="pOutputMsg"></param>
/// <returns></returns>
int AcanProtocol::FastInit(void* pInputMsg, void* pOutputMsg)
{

	uint8_t buf[512];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	ProtocolReqDptr->protocolid = ePROTOCOL_ASC;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONF;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->rev = FAST_INIT; //

	memcpy(ProtocolReqDptr->pData, pInputMsg, 5);//FMT TRGADDR SRADDR CMD CS
	ProtocolReqDptr->datalen = 5;
	int FastInitRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 100);
	return FastInitRet;

}

/// <summary>
/// 
/// </summary>
/// <param name="pInputMsg"></param>
/// <param name="pOutputMsg"></param>
/// <returns></returns>
int AcanProtocol::ESB(void* pInputMsg, void* pOutputMsg)
{

	uint8_t buf[512];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	ProtocolReqDptr->protocolid = ePROTOCOL_ASC;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONF;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->rev = CAN_ESB; //
	//PatternCount  ResponseLength   Timeout->持续发送时间  收到停止
	memcpy(ProtocolReqDptr->pData, pInputMsg, 12);//FMT TRGADDR SRADDR CMD CS
	ProtocolReqDptr->datalen = 12;
	int FastInitRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 100);//收到指令并启动发送后返回
	return FastInitRet;

}

/// <summary>
/// 
/// </summary>
/// <param name="pInputMsg"></param>
/// <param name="pOutputMsg"></param>
/// <returns></returns>
int AcanProtocol::FsCheck(void* pInputMsg, void* pOutputMsg)
{
	debugout("FsCheck Called...\r\n");
	uint8_t buf[512];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	ProtocolReqDptr->protocolid = ePROTOCOL_ASC;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONF;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->rev = FS_CHECK; //
	//CRC32  FileName
	int strlenval = strlen(((char*)(pInputMsg)+4));//去除CRC 后字符长度
	memcpy(ProtocolReqDptr->pData, pInputMsg,strlenval + 4);
	ProtocolReqDptr->datalen = strlenval + 4;//有效数据长度
	int FsCheckRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 2000);//收到指令并启动发送后返回
	return FsCheckRet;
}
/// <summary>
/// 
/// </summary>
/// <param name="pInputMsg"></param>
/// <param name="pOutputMsg"></param>
/// <returns></returns>
int AcanProtocol::FsClear(void* pInputMsg, void* pOutputMsg)
{
	debugout("FsClear Called...\r\n");
	uint8_t buf[512];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	ProtocolReqDptr->protocolid = ePROTOCOL_ASC;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONF;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->rev = FS_CLEAR; //
	//FileName
	int strlenval = strlen(((char*)(pInputMsg)));//去除CRC 后字符长度
	memcpy(ProtocolReqDptr->pData, pInputMsg, strlenval);
	ProtocolReqDptr->datalen = strlenval;//有效数据长度
	int FsClearRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 1000);//收到指令并启动发送后返回
	return FsClearRet;
}
/// <summary>
/// 
/// </summary>
/// <param name="pInputMsg"></param>
/// <param name="pOutputMsg"></param>
/// <returns></returns>
int AcanProtocol::FsWrite(void* pInputMsg, void* pOutputMsg)
{
	debugout("FsWrite Called...\r\n");
	uint8_t buf[5120];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	ProtocolReqDptr->protocolid = ePROTOCOL_ASC;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONF;
	ProtocolReqDptr->protocolchanl = ChanlNum;//dont care
	ProtocolReqDptr->rev = FS_WRITE; //

	ifstream inFile;
	char path[255];
	strcpy_s(path, (char*)pInputMsg);//拷贝完整的 文件路劲
	inFile.open((char*)path, ios::in | ios::binary);/*打开文件*/
	if (inFile)//文件打开成功
	{
		uint32_t addroffset = 0;
		while (inFile.peek() != EOF)
		{
			//前4个字节为 偏移地址  后面为数据长度
			*(uint32_t*)ProtocolReqDptr->pData = addroffset;
			long long readnum = inFile.read((char*)(ProtocolReqDptr->pData+4), 4096).gcount();/*请求读取4K 并获取实际读取到的值*/
			ProtocolReqDptr->datalen = (uint16_t)(readnum + 4);//
			int FsWriteRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 1000);//收到指令并启动发送后返回
			if(FsWriteRet != STATUS_NOERROR)//单次传输失败 返回错误
				return FsWriteRet;
			addroffset += (uint32_t)readnum;//数据偏移
		}
		inFile.close();/*关闭文件*/
	}
	else//文件打开失败
	{
		return ERR_INVALID_IOCTL_ID;
	}
	return STATUS_NOERROR;//传输完成
}
/// <summary>
/// 
/// </summary>
/// <param name="pInputMsg"></param>
/// <param name="pOutputMsg"></param>
/// <returns></returns>
int AcanProtocol::TswCfg(void* pInputMsg, void* pOutputMsg)//配置Tsw 传输参数
{
	debugout("TswCfg Called...\r\n");
	uint8_t buf[512];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	ProtocolReqDptr->protocolid = ePROTOCOL_ASC;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONF;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->rev = TSW_CFG; //
	//FsTswCfgFormat 
	memcpy(ProtocolReqDptr->pData, pInputMsg, sizeof(FsTswCfgFormat));//FMT TRGADDR SRADDR CMD CS
	ProtocolReqDptr->datalen = sizeof(FsTswCfgFormat);
	int TswCfgRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 100);//收到指令并启动发送后返回
	return TswCfgRet;
}
/// <summary>
/// 
/// </summary>
/// <param name="pInputMsg"></param>
/// <param name="pOutputMsg"></param>
/// <returns></returns>
int AcanProtocol::TswStart(void* pInputMsg, void* pOutputMsg)
{
	debugout("TswStart Called...\r\n");
	uint8_t buf[512];
	ProtocolDataType* ProtocolReqDptr = (ProtocolDataType*)buf;
	ProtocolReqDptr->protocolid = ePROTOCOL_ASC;
	ProtocolReqDptr->protocolcmd = CMDCODE_CONF;
	ProtocolReqDptr->protocolchanl = ChanlNum;
	ProtocolReqDptr->rev = START_EXPROGSEQUEUE; //
	ProtocolReqDptr->datalen = 0;
	int FastInitRet = TrsmitWithResponse(buf, (ProtocolReqDptr->datalen + 6), 60000);//最大等待一分钟
	return FastInitRet;
}




