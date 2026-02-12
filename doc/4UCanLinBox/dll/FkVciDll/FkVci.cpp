#include"pch.h"
#include"FkVciDll.h"
#include"VciDebugCf.h"
#include"BaseDevice.h"
#include"BaseProtocol.h"
#include"LanDevice.h"
#include"AcanProtocol.h"
#include"CanProtocol.h"
#include"LinProtocol.h"
#include"SentProtocol.h"
#include "j2534_v0404.h"
#define MAX_DEV_NUM 16
SHANDLE DevList[MAX_DEV_NUM];/*可以把这里换成vector*/
/// <summary>
/// 
/// </summary>
/// <param name="device_type"></0:LAN 1:USB 2:COM> 
/// <param name="device_index"></param>
/// <param name="reserved"></param>
/// <returns></returns>
SHANDLE FkVciOpenDev(UINT32_T device_type, UINT32_T device_index, UINT32_T reserved)
{
	BaseDevice* dev = NULL;
	char devipaddr[32];
	//通过 pName 定义打开的dev 类型
	dev = new LanDevice();/*新建一个sn*/
	switch (device_index)
	{
	case 0:
		strcpy_s(devipaddr, "192.168.201.130");
		break;
	case 1:
		strcpy_s(devipaddr, "192.168.201.131");
		break;
	case 2:
		strcpy_s(devipaddr, "192.168.201.132");
		break;
	case 3:
		strcpy_s(devipaddr, "192.168.201.133");
		break;
	case 4:
		strcpy_s(devipaddr, "192.168.201.134");
		break;
	case 5:
		strcpy_s(devipaddr, "192.168.201.135");
		break;
	case 6:
		strcpy_s(devipaddr, "192.168.201.136");
		break;
	case 7:
		strcpy_s(devipaddr, "192.168.201.137");
		break;
	case 8:
		strcpy_s(devipaddr, "192.168.201.138");
		break;
	case 9:
		strcpy_s(devipaddr, "192.168.201.139");
		break;
	case 10:
		strcpy_s(devipaddr, "192.168.201.140");
		break;
	case 11:
		strcpy_s(devipaddr, "192.168.201.141");
		break;
	case 12:
		strcpy_s(devipaddr, "192.168.201.142");
		break;
	case 13:
		strcpy_s(devipaddr, "192.168.201.143");
		break;
	case 14:
		strcpy_s(devipaddr, "192.168.201.144");
		break;
	case 15:
		strcpy_s(devipaddr, "192.168.201.145");
		break;

	default:
		return NULL;
		break;
	}
	uint32_t devindex = inet_addr(devipaddr);
	/*查询该IP 是否打开了*/
	BaseDevice* pOpenCheckDev;
	for (int i = 0; i < MAX_DEV_NUM; i++)//将新建的DEV 句柄保存起来
	{
		if (DevList[i] != NULL)
		{
			pOpenCheckDev = (BaseDevice*)DevList[i];
			if (pOpenCheckDev->DeviceIndexNum == devindex)
			{
				return DevList[i];
			}
		}
	}

	if (dev->DevOpen((void*)(devipaddr)) == ERRCODE_NOERR)/*打开成功*/
	{
		for (int i = 0; i < MAX_DEV_NUM; i++)//将新建的DEV 句柄保存起来
		{
			if (DevList[i] == NULL)
			{
				DevList[i] = dev;
				return DevList[i];
			}
		}
		dev->DevClose();//创建成功了 但是打到了最大打开量 先关闭 再删除
		delete (LanDevice*)dev;
		return NULL;
	}
	else
	{
		delete (LanDevice*)dev;;//未创建成功  删除
	}
	return NULL;

}
/// <summary>
/// 
/// </summary>
/// <param name="device_handle"></通过OpenDev 打开的句柄>
/// <returns></returns>
int FkVciCloseDev(SHANDLE device_handle)
{
	if (device_handle == NULL)
		return ERRCODE_INVALID_DEVICE_ID;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (device_handle == DevList[i])/**/
		{
			BaseDevice* dev = (BaseDevice*)device_handle;
			dev->DevClose();
			if(dev->DeviceType = eLAN)//判定当前使用的是何种硬件
			{
				delete (LanDevice*)dev;/*删除之前new的 class*/
			}			
			DevList[i] = NULL;
			return ERRCODE_NOERR;/*关闭成功*/
		}
	}
	return ERRCODE_INVALID_DEVICE_ID;/*DevID 不对*/
}

void FkDllDetachCallBack()
{	
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			dev->DevClose();
			if (dev->DeviceType = eLAN)//判定当前使用的是何种硬件
			{
				delete (LanDevice*)dev;/*删除之前new的 class*/
			}
			DevList[i] = NULL;		
		}
	}
}
/// <summary>
/// 获取协议缓存条数
/// </summary>
/// <param name="channel_handle"></通过对应协议初始化获得的句柄>
/// <param name="type"></param>
/// <returns></returns>
int FkVciGetReceiveNum(SHANDLE channel_handle, UINT8_T type)
{
	if (channel_handle == NULL)/*没有找到对应的SHANDLE*/
		return 2;
	for (int i = 0; i < MAX_DEV_NUM; i++)/*遍历dev表格*/
	{
		BaseDevice* dev = (BaseDevice*)DevList[i];/*获取句柄*/
		if (dev != NULL)/*句柄有效*/
		{
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)//查询每个dev中是否有通道
			{				
				if (channel_handle == dev->ProtocolTab[j])//找到对应的通道
				{
					BaseProtocol* Protocol = (BaseProtocol*)dev->ProtocolTab[j];
					return (int)(Protocol->GetRxMessageCount());
				}
			}
		}
		else
		{

		}
	}
	return 2;/*没有找到对应的SHANDLE*/
}
/// <summary>
/// 清空接收缓存
/// </summary>
/// <param name="channel_handle"></设备句柄>
/// <returns></错误码>
int FkVciClearReceiveBuf(SHANDLE channel_handle)
{
	if (channel_handle == NULL)/*没有找到对应的SHANDLE*/
		return 2;
	for (int i = 0; i < MAX_DEV_NUM; i++)/*遍历dev表格*/
	{
		BaseDevice* dev = (BaseDevice*)DevList[i];/*获取句柄*/
		if (dev != NULL)/*句柄有效*/
		{
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)//查询每个dev中是否有通道
			{
				if (channel_handle == dev->ProtocolTab[j])//找到对应的通道
				{
					BaseProtocol* Protocol = (BaseProtocol*)dev->ProtocolTab[j];
					Protocol->ClearRxBuffer();
					return 0;
				}
			}
		}
		else
		{

		}
	}
	return 2;
}
#if(0)
SHANDLE FkVciInitACAN(SHANDLE device_handle, UINT32_T can_index, UINT32_T Baudrate)
{
	if (device_handle == NULL)
		return NULL;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (device_handle == DevList[i])/**/
		{
			BaseDevice* dev = (BaseDevice*)device_handle;
			AcanProtocol* Protocol = new AcanProtocol(dev, can_index);//新建一个对应通道
			if (dev->InsetProtocol((void*)Protocol) != ERRCODE_NOERR)//必须先插入  应为调用了 dev->deal
			{
				delete Protocol;
				return NULL;
			}

			if (Protocol->ProtocolConnect(NULL, can_index, Baudrate) == ERRCODE_NOERR)//建立通道成功
			{
				return (SHANDLE)Protocol;
			}
			else
			{
				dev->DeleteProtocol(Protocol);//只是单纯的从表中删除了 协议并没有释放协议
				delete Protocol;
				return NULL;
			}
		}
	}
	return NULL;/*DevID 不对*/
}

int FkVciResetACAN(SHANDLE channel_handle)
{
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					AcanProtocol* Protocol = (AcanProtocol*)channel_handle;
					int DisConnectRet = Protocol->ProtocolDisConnect();					
					dev->DeleteProtocol(Protocol);//这里只是单纯删除了表格中数据 没有释放协议
					delete Protocol; //删除协议
					return DisConnectRet;
				}
			}
		}
	}
	return ERRCODE_INVALID_PROTOCOL_ID;/*DevID 不对*/
}

int FkVciReceiveACAN(SHANDLE channel_handle, FkVciKwpDataType* pReceive, UINT32_T len, int wait_time)
{
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					BaseProtocol* Protocol = (BaseProtocol*)channel_handle;
					UINT32_T ReadMsgNum = len;
					int ReadMsgRet = Protocol->ProtocolReadMsgs(pReceive,&ReadMsgNum, wait_time);
					if (ReadMsgRet == ERRCODE_NOERR)
					{
						return ReadMsgNum;//返回实际读取条数目
					}
					else
					{
						return 0;
					}
				}
			}
		}
	}
	return 0;
}

int FkVciTrsmitACAN(SHANDLE channel_handle, FkVciKwpDataType* pTransmit, UINT32_T len)
{
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					BaseProtocol* Protocol = (BaseProtocol*)channel_handle;
					UINT32_T WriteMsgNum = len;
					int WriteMsgRet = Protocol->ProtocolWriteMsgs(pTransmit, &WriteMsgNum, 1000);
					if (WriteMsgRet == ERRCODE_NOERR)
					{
						return WriteMsgNum;//返回实际读取条数目
					}
					else
					{
						return 0;
					}
				}
			}
		}
	}
	return 0;
}

int FkVciConfigACAN(SHANDLE channel_handle, UINT32_T ConfigId,void* pInitConfig)
{
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					BaseProtocol* Protocol = (BaseProtocol*)channel_handle;
					return Protocol->ProtocolIOCTL(ConfigId, pInitConfig,NULL);
				}
			}
		}
	}
	return 0;
}
#endif
/**************************************************************************************/
SHANDLE FkVciInitCAN(SHANDLE device_handle, UINT32_T can_index, UINT32_T Baudrate)
{
	if (device_handle == NULL)
		return NULL;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (device_handle == DevList[i])/**/
		{
			BaseDevice* dev = (BaseDevice*)device_handle;
			//先检查是否已经打开了
			//CanProtocol* Protocol = new CanProtocol(dev, can_index);//新建一个对应通道

			CanProtocol* Protocol = (CanProtocol*)dev->GetProtocolHandle(ePROTOCOL_CAN,can_index);
			if (Protocol == NULL)/*如果没有 创建一个 有的话直接使用*/
			{
				Protocol = new CanProtocol(dev, can_index);//新建一个对应通道
				if (dev->InsetProtocol((void*)Protocol) != ERRCODE_NOERR)//必须先插入  应为调用了 dev->deal
				{
					delete Protocol;
					return NULL;
				}
			}
			if (Protocol->ProtocolConnect(NULL, 0, Baudrate) == ERRCODE_NOERR)//建立通道成功
			{
				return (SHANDLE)Protocol;
			}
			else//可能是重复打开 
			{
				dev->DeleteProtocol(Protocol);
				delete Protocol;
				return NULL;
			}
		}
	}
	return NULL;/*DevID 不对*/
}

SHANDLE FkVciInitCANFD(SHANDLE device_handle, UINT32_T can_index, UINT32_T Baudrate,UINT32_T DataBaudRate)
{
	if (device_handle == NULL)
		return NULL;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (device_handle == DevList[i])/**/
		{
			BaseDevice* dev = (BaseDevice*)device_handle;
			//CanProtocol* Protocol = new CanProtocol(dev, can_index);//新建一个对应通道
			//if (dev->InsetProtocol((void*)Protocol) != ERRCODE_NOERR)//必须先插入  应为调用了 dev->deal
			//{
			//	delete Protocol;
			//	return NULL;
			//}

			CanProtocol* Protocol = (CanProtocol*)dev->GetProtocolHandle(ePROTOCOL_CAN, can_index);//查询已经初始化的通道
			if (Protocol == NULL)
			{
				Protocol = new CanProtocol(dev, can_index);//新建一个对应通道
				if (dev->InsetProtocol((void*)Protocol) != ERRCODE_NOERR)//必须先插入  应为调用了 dev->deal
				{
					delete Protocol;
					return NULL;
				}
			}
			if (Protocol->ProtocolConnect(NULL, 0, Baudrate) == ERRCODE_NOERR)//建立通道成功
			{
				//使用FkVciConfigCAN 去配置 CANFD
				//设置 Baudrate
				VciConfigDataType cancfg;
				cancfg.Parameter = DATA_RATE;
				cancfg.Value = Baudrate;
				Protocol->ProtocolIOCTL(SET_CONFIG, &cancfg, NULL);
				//设置 nseg1
				cancfg.Parameter = NSEG1;
				cancfg.Value = 31;
				Protocol->ProtocolIOCTL(SET_CONFIG, &cancfg, NULL);
				//设置 nseg2
				cancfg.Parameter = NSEG2;
				cancfg.Value = 8;
				Protocol->ProtocolIOCTL(SET_CONFIG, &cancfg, NULL);
				//设置 DataBaudRate
				cancfg.Parameter = DATA_RATE_BRS;
				cancfg.Value = DataBaudRate;
				Protocol->ProtocolIOCTL(SET_CONFIG, &cancfg, NULL);
				//设置 dnseg1
				cancfg.Parameter = DSEG1;
				cancfg.Value = 13;
				Protocol->ProtocolIOCTL(SET_CONFIG, &cancfg, NULL);
				//设置 dnseg2
				cancfg.Parameter = DSEG2;
				cancfg.Value = 6;
				Protocol->ProtocolIOCTL(SET_CONFIG, &cancfg, NULL);
				//调用Fast Init 初始化CAN
				Protocol->ProtocolIOCTL(FAST_INIT, &cancfg, NULL);
				return (SHANDLE)Protocol;//返回协议
			}
			else//可能是重复打开 
			{
				dev->DeleteProtocol(Protocol);
				delete Protocol;
				return NULL;
			}
		}
	}
	return NULL;/*DevID 不对*/
}


/// <summary>
/// 设置滤波器  如果要清空滤波器 需要调用ResetCan 
/// </summary>
/// <param name="channel_handle"></param>
/// <param name="can_index"></param>
/// <param name="StartId"></param>
/// <param name="StopId"></param>
/// <returns></returns>
int FkVciSetFilterCAN(SHANDLE channel_handle, UINT32_T can_index, UINT32_T StartId, UINT32_T StopId)
{
	if (channel_handle == NULL)
		return 2;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					CanProtocol* Protocol = (CanProtocol*)channel_handle;
					UINT32_T start = StartId;
					UINT32_T stop = StopId;
					int StartMsgFilterRet = Protocol->ProtocolStartMsgFilter(0,&start,&stop,NULL,NULL);
					return StartMsgFilterRet;
				}
			}
		}
	}
	return ERRCODE_INVALID_PROTOCOL_ID;/*DevID 不对*/
}

int FkVciResetCAN(SHANDLE channel_handle)
{
	if (channel_handle == NULL)
		return 2;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					CanProtocol* Protocol = (CanProtocol*)channel_handle;
					int DisConnectRet = Protocol->ProtocolDisConnect();
					dev->DeleteProtocol(Protocol);
					delete Protocol; //删除协议
					return DisConnectRet;
				}
			}
		}
	}
	return ERRCODE_INVALID_PROTOCOL_ID;/*DevID 不对*/
}

int FkVciReceiveCAN(SHANDLE channel_handle, FkVciCanDataType* pReceive, UINT32_T len, int wait_time)
{
	if (channel_handle == NULL)
		return 0;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					BaseProtocol* Protocol = (BaseProtocol*)channel_handle;
					UINT32_T ReadMsgNum = len;
					int ReadMsgRet = Protocol->ProtocolReadMsgs(pReceive, &ReadMsgNum, wait_time);
					if (ReadMsgRet == ERRCODE_NOERR)
					{
						return ReadMsgNum;//返回实际读取条数目
					}
					else
					{
						return 0;
					}
				}
			}
		}
	}
	return 0;
}

int FkVciTrsmitCAN(SHANDLE channel_handle, FkVciCanDataType* pTransmit, UINT32_T len)
{
	if (channel_handle == NULL)
		return 0;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					BaseProtocol* Protocol = (BaseProtocol*)channel_handle;
					UINT32_T WriteMsgNum = len;
					int WriteMsgRet = Protocol->ProtocolWriteMsgs(pTransmit, &WriteMsgNum, 1000);
					if (WriteMsgRet == ERRCODE_NOERR)
					{
						return WriteMsgNum;//返回实际读取条数目
					}
					else
					{
						return 0;
					}
				}
			}
		}
	}
	return 0;
}

int FkVciConfigCAN(SHANDLE channel_handle, UINT32_T ConfigId, void* pInitConfig)
{
	if (channel_handle == NULL)
		return 2;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					BaseProtocol* Protocol = (BaseProtocol*)channel_handle;
					return Protocol->ProtocolIOCTL(ConfigId, pInitConfig, NULL);
				}
			}
		}
	}
	return 0;
}


/**************************************************************************************/

#if(1)
/// <summary>
/// 
/// </summary>
/// <param name="device_handle"></param>
/// <param name="can_index"></通道>
/// <param name="mode"></主机 从机>
/// <param name="Baudrate"></param>
/// <returns></returns>
SHANDLE FkVciInitLIN(SHANDLE device_handle, UINT32_T can_index,UINT32_T mode, UINT32_T Baudrate)
{
	if (device_handle == NULL)
		return NULL;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (device_handle == DevList[i])/**/
		{
			BaseDevice* dev = (BaseDevice*)device_handle;
			//LinProtocol* Protocol = new LinProtocol(dev, can_index);//新建一个对应通道
			//if (dev->InsetProtocol((void*)Protocol) != ERRCODE_NOERR)//必须先插入  应为调用了 dev->deal
			//{
			//	delete Protocol;
			//	return NULL;
			//}
			LinProtocol* Protocol = (LinProtocol *)dev->GetProtocolHandle(ePROTOCOL_LIN, can_index);//查询已经初始化的通道
			if (Protocol == NULL)
			{
				Protocol = new LinProtocol(dev, can_index);//新建一个对应通道
				if (dev->InsetProtocol((void*)Protocol) != ERRCODE_NOERR)//必须先插入  应为调用了 dev->deal
				{
					delete Protocol;
					return NULL;
				}
			}

			if (Protocol->ProtocolConnect(NULL, mode, Baudrate) == ERRCODE_NOERR)//建立通道成功
			{
				return (SHANDLE)Protocol;
			}
			else//可能是重复打开 
			{
				dev->DeleteProtocol(Protocol);
				delete Protocol;
				return NULL;
			}
		}
	}
	return NULL;/*DevID 不对*/
}
/// <summary>
/// 
/// </summary>
/// <param name="channel_handle"></param>
/// <returns></returns>
int FkVciResetLIN(SHANDLE channel_handle)
{
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					LinProtocol* Protocol = (LinProtocol*)channel_handle;
					int DisConnectRet = Protocol->ProtocolDisConnect();
					dev->DeleteProtocol(Protocol);
					delete Protocol; //删除协议
					return DisConnectRet;
				}
			}
		}
	}
	return ERRCODE_INVALID_PROTOCOL_ID;/*DevID 不对*/
}
/// <summary>
/// 在设置了SCH 后 或则设置了 Slave后 调用该接口函数 读取数据
/// </summary>
/// <param name="channel_handle"></param>
/// <param name="pReceive"></param>
/// <param name="len"></param>
/// <param name="wait_time"></param>
/// <returns></returns>
int FkVciRecieveLIN(SHANDLE channel_handle, FkVciLinDataType* pReceive, UINT32_T len, int wait_time)//在主机读取前 需要线发送头信息
{
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					BaseProtocol* Protocol = (BaseProtocol*)channel_handle;
					UINT32_T ReadMsgNum = len;
					int ReadMsgRet = Protocol->ProtocolReadMsgs(pReceive, &ReadMsgNum, wait_time);
					if (ReadMsgRet == ERRCODE_NOERR)
					{
						return ReadMsgNum;//返回实际读取条数目
					}
					else
					{
						return 0;
					}
				}
			}
		}
	}
	return 0;
}
/// <summary>
/// 主机模式下 使用改接口单次发送或读取数据  单次发送多条数据
/// 该模式与 SCH模式不能同时使用 一旦开启了 SCH 调用该接口 失败
/// </summary>
/// <param name="channel_handle"></param>
/// <param name="pTransmit"></请求数据地址>
/// <param name="pReceive"></接收数据地址>
/// <param name="len"></请求数据条数>
/// <returns></返回数据条数 理论上应该等于请求数据条数>
int FkVciTrsmitLIN(SHANDLE channel_handle, FkVciLinDataType* pTransmit, FkVciLinDataType* pReceive, UINT32_T len)//
{
	if (channel_handle == NULL)
		return 0;

	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					LinProtocol* Protocol = (LinProtocol*)channel_handle;
					if (Protocol->LinSchRunFlag)//已经开启了 SCH 调度模式  
						return 0;

					UINT32_T WriteMsgNum = len;
					Protocol->ClearRxBuffer();//使用该模式  必须先清空buf
					int WriteMsgRet = Protocol->ProtocolWriteMsgs(pTransmit, &WriteMsgNum, 100);
					if (WriteMsgRet == ERRCODE_NOERR)//先配置了 所有的发送 
					{
						WriteMsgNum = len;
						int  ReadMsgRet = Protocol->ProtocolReadMsgs(pReceive, &WriteMsgNum, 1000);//再接收配置的数据
						return WriteMsgNum;//返回实际读取条数目
					}
					else
					{
						return 0;
					}
				}
			}
		}
	}
	return 0;
}
/// <summary>
/// 主模式下 设置内部周期调度 并启动LIN  
/// 从模式下 配置一个调度表
/// </summary>
/// <param name="channel_handle"></param>
/// <param name="pTransmit"></param>
/// <param name="len"></param>
/// <returns></returns>
int FkVciLinSchStart(SHANDLE channel_handle, FkVciLinDataType* pTransmit, UINT32_T len)
{
	if (channel_handle == NULL)
		return 2;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					LinProtocol* Protocol = (LinProtocol*)channel_handle;
					FkVciLinDataType* LinDptr = (FkVciLinDataType*)pTransmit;
					for (UINT32_T i = 0; i < len; i++)
					{
						//先下载配置数据
						Protocol->ProtocolIOCTL(FS_WRITE, LinDptr,NULL);
						LinDptr++;
					}
					//启动SCH调度
					Protocol->ProtocolIOCTL(START_EXPROGSEQUEUE,NULL,NULL);
					Protocol->LinSchRunFlag = 1;
					return 0;
				}
			}
		}
	}
	return 2;
}

/// <summary>
/// 主模式下 关闭内部周期调度
/// 从模式下 清空从机数据
/// </summary>
/// <param name="channel_handle"></param>
/// <returns></returns>
int FkVciLinSchStop(SHANDLE channel_handle)
{
	if (channel_handle == NULL)
		return 2;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					LinProtocol* Protocol = (LinProtocol*)channel_handle;
					//停止SCH 调度
					Protocol->ProtocolIOCTL(FS_CLEAR, NULL, NULL);//清空
					Protocol->LinSchRunFlag = 0;
					return 0;
				}
			}
		}
	}
	return 2;
}

/// <summary>
/// 从机模式下 修改一条数据信息
/// </summary>
/// <param name="channel_handle"></param>
/// <param name="pTransmit"></param>
/// <returns></returns>
int FkVciLinSlaveWriteMsg(SHANDLE channel_handle, FkVciLinDataType* pTransmit)
{
	if (channel_handle == NULL)
		return 2;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					LinProtocol* Protocol = (LinProtocol*)channel_handle;
					//停止SCH 调度
					return Protocol->ProtocolIOCTL(FS_WRITE, pTransmit, NULL);
					
				}
			}
		}
	}
	return 2;
}
/// <summary>
/// 
/// </summary>
/// <param name="channel_handle"></param>
/// <param name="ConfigId"></param>
/// <param name="pInitConfig"></param>
/// <returns></returns>
int FkVciConfigLIN(SHANDLE channel_handle, UINT32_T ConfigId, void* pInitConfig)
{
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					BaseProtocol* Protocol = (BaseProtocol*)channel_handle;
					return Protocol->ProtocolIOCTL(ConfigId, pInitConfig, NULL);
				}
			}
		}
	}
	return 0;
}
#endif

/// <summary>
/// 
/// </summary>
/// <param name="device_handle">设备句柄</param>
/// <param name="can_index">通道序号 0/1</param>
/// <param name="mode">连续读取MSG 数量1-16 </param>
/// <param name="Baudrate">时钟数1-90(us) 1为1.5us</param>
/// <param name="CycleMs">读取时间间隔</param>
/// <returns>通道句柄</returns>

SHANDLE FkVciInitSent(SHANDLE device_handle, UINT32_T can_index, UINT32_T mode, UINT32_T Baudrate, UINT32_T CycleMs)
{
	uint32_t TempA = (mode & 0xff) << 24 | (Baudrate & 0xff) << 16 | (CycleMs & 0xffff);
	if (device_handle == NULL)
		return NULL;
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (device_handle == DevList[i])/**/
		{
			BaseDevice* dev = (BaseDevice*)device_handle;
			SentProtocol* Protocol = (SentProtocol*)dev->GetProtocolHandle(ePROTOCOL_SENT, can_index);//查询已经初始化的通道
			if (Protocol == NULL)
			{
				Protocol = new SentProtocol(dev, can_index);//新建一个对应通道
				if (dev->InsetProtocol((void*)Protocol) != ERRCODE_NOERR)//必须先插入  应为调用了 dev->deal
				{
					delete Protocol;
					return NULL;
				}
			}

			if (Protocol->ProtocolConnect(NULL, mode, TempA) == ERRCODE_NOERR)//建立通道成功
			{
				return (SHANDLE)Protocol;
			}
			else//可能是重复打开 
			{
				dev->DeleteProtocol(Protocol);
				delete Protocol;
				return NULL;
			}
		}
	}
	return NULL;/*DevID 不对*/	
}

int FkVciResetSent(SHANDLE channel_handle)
{
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					SentProtocol* Protocol = (SentProtocol*)channel_handle;
					int DisConnectRet = Protocol->ProtocolDisConnect();
					dev->DeleteProtocol(Protocol);
					delete Protocol; //删除协议
					return DisConnectRet;
				}
			}
		}
	}
	return ERRCODE_INVALID_PROTOCOL_ID;/*DevID 不对*/
}

int FkVciRecievSent(SHANDLE channel_handle, FkVciSentDataType* pReceive, UINT32_T len, int wait_time)//在主机读取前 需要线发送头信息
{
	for (int i = 0; i < MAX_DEV_NUM; i++)
	{
		if (DevList[i] != NULL)/**/
		{
			BaseDevice* dev = (BaseDevice*)DevList[i];
			for (int j = 0; j < MAX_PROTOCOL_NUM; j++)
			{
				if (dev->ProtocolTab[j] == channel_handle)
				{
					BaseProtocol* Protocol = (BaseProtocol*)channel_handle;
					UINT32_T ReadMsgNum = len;
					int ReadMsgRet = Protocol->ProtocolReadMsgs(pReceive, &ReadMsgNum, wait_time);
					if (ReadMsgRet == ERRCODE_NOERR)
					{
						return ReadMsgNum;//返回实际读取条数目
					}
					else
					{
						return 0;
					}
				}
			}
		}
	}
}

/**/

