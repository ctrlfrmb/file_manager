#include"pch.h"
#include "BaseDevice.h"
#include "FkVciDll.h"
#include "VciDebugCf.h"
#include "BaseProtocol.h"
#include "AcanProtocol.h"


DWORD WINAPI DeviceThreadFunction(LPVOID lpParam)
{
	BaseDevice* me = (BaseDevice*)lpParam;

	while (me->DevRecieveRunFlag)
	{
		me->DevRecieve();
	}
	SetEvent(me->hDevRecieveThreadExitResponseHandler);
	return 0;
}


BaseDevice::BaseDevice()
{
	debugout("BaseDevice Create ...\r\n");
	DevRecieveRunFlag = 0;
	/*for (int i = 0; i < MAX_PROTOCOL_NUM; i++)
	{
		ProtocolTab[i] = NULL;
	}*/
	memset(ProtocolTab, NULL, sizeof(ProtocolTab));
	hDevRecieveThreadHandler = NULL;
	hDevRecieveThreadExitResponseHandler = NULL;
	DeviceType = 0;
	LastErrCode = 0;
}

BaseDevice::~BaseDevice()
{
	debugout("BaseDevice Delete ...\r\n");
	//for (int i = 0; i < MAX_PROTOCOL_NUM; i++)
	//{
	//	ProtocolTab[i] = NULL;
	//}
	hDevRecieveThreadHandler = NULL;
	hDevRecieveThreadExitResponseHandler = NULL;
}

int BaseDevice::CreateRecieveThread()
{
	hDevRecieveThreadExitResponseHandler = CreateEventA(
											NULL,		// default security attributes
											TRUE,		// manual-reset event
											FALSE,		// initial state is nonsignaled
											NULL);		// object name
	if (hDevRecieveThreadExitResponseHandler == NULL)
	{
		return ERRCODE_MEMERR;
	}

	DevRecieveRunFlag = 1;//启动硬件接收标志位
	hDevRecieveThreadHandler = CreateThread(
											NULL,                   // default security attributes
											0,                      // use default stack size  
											DeviceThreadFunction,       // thread function name
											(void*)this, /*pDataArray[i],*/          // argument to thread function 
											0,                      // use default creation flags 
											NULL); //&dwThreadIdArray[i]);   // returns the thread identifier 
	if (hDevRecieveThreadHandler == NULL)
	{
		DevRecieveRunFlag = 0;
		return ERRCODE_MEMERR;
	}
	return ERRCODE_NOERR;
}

int BaseDevice::DevOpen(void* pName)
{
	debugout("BaseDevice DevOpen ...\r\n");
	//Add Some Hard Ware Api Here

	return CreateRecieveThread();
}

int BaseDevice::DevClose()
{
	debugout("BaseDevice DevClose ...\r\n");
	ClearProtocol();

	DevRecieveRunFlag = 0;
	WaitForSingleObjectEx(hDevRecieveThreadExitResponseHandler, INFINITE, true);/*等待设备释放*/
	return ERRCODE_NOERR;
}

int BaseDevice::DevTrsmit(const uint8_t* data, int len)
{
	debugout("BaseDevice DevTrsmit ...\r\n");
	return ERRCODE_NOERR;
}

int BaseDevice::DevRecieve()
{
	debugout("BaseDevice DevRecieve ...\r\n");
	Sleep(100);
	return ERRCODE_NOERR;
}
/*
* 当一条协议通道创建成功后调用
* 
* 
*/
int BaseDevice::InsetProtocol(void* protocol_handle)
{
	debugout("BaseDevice InsetProtocol ...\r\n");
	if (protocol_handle == NULL)
	{
		debugout("BaseDevice InsetProtocol -> Input==NULL...\r\n");
		return ERRCODE_PROTOCOLIDERR;
	}

	for (int i = 0; i < MAX_PROTOCOL_NUM; i++)
	{
		if (ProtocolTab[i] != NULL)
		{
			if (ProtocolTab[i] == protocol_handle)
			{
				debugout("BaseDevice InsetProtocol -> Protocol Have In\r\n");
				return ERRCODE_PROTOCOLOPENED;
			}
			if (((BaseProtocol*)ProtocolTab[i])->ProtocolID == ((BaseProtocol*)protocol_handle)->ProtocolID
				&& ((BaseProtocol*)ProtocolTab[i])->ChanlNum == ((BaseProtocol*)protocol_handle)->ChanlNum
				)
			{
				debugout("BaseDevice InsetProtocol -> Protocol Have OPENED\r\n");
				return ERRCODE_PROTOCOLOPENED;
			}

		}

	}

	for (int i = 0; i < MAX_PROTOCOL_NUM; i++)
	{
		if (ProtocolTab[i] == NULL)
		{
			ProtocolTab[i] = protocol_handle;
			return ERRCODE_NOERR;
		}
	}
	return ERRCODE_NOERR;
}
/*
* 删除一条协议通道 当断开连接后调用
* 
* 
* 
* 
*/
int BaseDevice::DeleteProtocol(void* protocol_handle)
{
	debugout("BaseDevice DeleteProtocol ...\r\n");
	for (int i = 0; i < MAX_PROTOCOL_NUM; i++)
	{
		if (ProtocolTab[i] == protocol_handle)
		{
			//BaseProtocol* Protocol = (BaseProtocol*)protocol_handle;
			//delete Protocol;
			ProtocolTab[i] = NULL;
			return ERRCODE_NOERR;
		}
	}
	return ERRCODE_NOERR;
}


/*
*  关闭所有协议  在关闭DEV时候调用
* 
* 
* 
*/
int BaseDevice::ClearProtocol()
{
	debugout("BaseDevice ClearProtocol ...\r\n");
	for (int i = 0; i < MAX_PROTOCOL_NUM; i++)
	{
		if (ProtocolTab[i] != NULL)
		{
			BaseProtocol* Protocol = (BaseProtocol*)ProtocolTab[i];
			Protocol->ProtocolDisConnect();//先调用关闭 不管结果如何
			if (Protocol->ProtocolID == ePROTOCOL_ASC)//通过当前协议 删除对应的协议 避免内存删除不干净
			{
				delete (AcanProtocol*)Protocol;
			}
			
			
			ProtocolTab[i] = NULL;
		}
	}
	return ERRCODE_NOERR;
}


HANDLE BaseDevice::GetProtocolHandle(uint8_t ProtocolId, uint8_t Chanl)
{
	for (int i = 0; i < MAX_PROTOCOL_NUM; i++)
	{
		if (ProtocolTab[i] != NULL)
		{
			BaseProtocol* Protocol = (BaseProtocol*)ProtocolTab[i];
			if (Protocol->ProtocolID == ProtocolId && Protocol->ChanlNum == Chanl)
			{
				return ProtocolTab[i];
			}
		}
	}
	return NULL;
}


