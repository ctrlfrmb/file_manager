#pragma once
#include "stdint.h"

typedef void* SHANDLE;
typedef unsigned char      UINT8_T;
typedef unsigned short     UINT16_T;
typedef unsigned int       UINT32_T;

#define ERRCODE_NOERR 0
#define ERRCODE_MEMERR 1
#define ERRCODE_PROTOCOLIDERR 2
#define ERRCODE_PROTOCOLOPENED 3
#define ERRCODE_LANRECERR  4
#define ERRCODE_INVALID_DEVICE_ID 5
#define ERRCODE_INVALID_PROTOCOL_ID 6
#define ERRCODE_INVALID_MSG 10
#define ERRCODE_FAILED  7
#define ERRCODE_TIMEOUT 8
#define ERRCODE_BUFFER_EMPTY 9
#define ERRCODE_OPENDEV_FAILED 11

#define LIN_EX_MSG_TYPE_UN      0  //未知类型
#define LIN_EX_MSG_TYPE_MW      1  //主机向从机发送数据
#define LIN_EX_MSG_TYPE_MR      2  //主机从从机读取数据
#define LIN_EX_MSG_TYPE_SW      3  //从机发送数据
#define LIN_EX_MSG_TYPE_SR      4  //从机接收数据
#define LIN_EX_MSG_TYPE_BK      5  //只发送BREAK信号，若是反馈回来的数据，表明只检测到BREAK信号
#define LIN_EX_MSG_TYPE_SY      6  //表明检测到了BREAK，SYNC信号
#define LIN_EX_MSG_TYPE_ID      7  //表明检测到了BREAK，SYNC，PID信号
#define LIN_EX_MSG_TYPE_DT      8  //表明检测到了BREAK，SYNC，PID,DATA信号
#define LIN_EX_MSG_TYPE_CK      9  //表明检测到了BREAK，SYNC，PID,DATA,CHECK信号
/// <summary>
/// 
/// </summary>
typedef struct FkVciCanDataType
{
	unsigned int   CanID;
	unsigned char  DLC;
	unsigned char  FLAG;//bit0:Stand=0/Ext=1   bit1:远程帧=1/数据帧=0   bit2:CANFDBRSEN=1  Bit6:CAN=0/CANFD=1
	unsigned char  REV1;
	unsigned char  REV2;
	unsigned long int TimesampL;
	unsigned long int TimesampH;
	unsigned char  Data[64];
}FkVciCanDataType;
/// <summary>
/// 
/// </summary>
typedef struct FkVciLinDataType
{
	unsigned char LinID;//输入的是ID
	unsigned char DLC;  //长度 
	unsigned char ChekcType;//校验方式 0:标准 1:增强
	unsigned char  MsgType;//见 LIN_EX_MSG_TYPE_MW LIN_EX_MSG_TYPE_MR
	unsigned long int TimesampL;//发送时候为时间间隔 单位ms  接收为时间戳 单位us
	unsigned long int TimesampH;
	unsigned char  Data[8];//数据
	unsigned char  Check; //校验和
	unsigned char  PID;  //返回PID
}FkVciLinDataType;
/// <summary>
/// 
/// </summary>
typedef struct FkVciKwpDataType
{
	unsigned char ClientAddr;
	unsigned char EcuAddr;
	unsigned short DLC;
	unsigned long int TimesampL;
	unsigned long int TimesampH;
	unsigned char  Data[260];
}FkVciKwpDataType;
/// <summary>
/// 
/// </summary>
typedef struct FkVciIsoDataType
{
	unsigned int CanID;
	unsigned short MsgLen;
	unsigned char  FLAG;
	unsigned char  REV1;
	unsigned long int TimesampL;
	unsigned long int TimesampH;
	unsigned char  Data[4096];
}FkVciIsoDataType;
typedef struct FkVciSentDataType
{
	unsigned char DLC;  //长度 Data 中包含个SENT 信息
	unsigned char REV;
	unsigned long int TimesampL;//发送时候为时间间隔 单位ms  接收为时间戳 单位us
	unsigned long int TimesampH;
	unsigned char  Data[64];
}FkVciSentDataType;
/// <summary>
/// 
/// </summary>
typedef union FkVciDataType
{
	FkVciCanDataType Can;
	FkVciLinDataType Lin;
	FkVciKwpDataType Kwp;
	FkVciIsoDataType Iso;
}FkVciDataType;


/// <summary>
/// 打开设备
/// </summary>
/// <param name="device_type"></param>
/// <param name="device_index"></param>
/// <param name="reserved"></param>
/// <returns></返回设备句柄>
SHANDLE FkVciOpenDev(UINT32_T device_type, UINT32_T device_index, UINT32_T reserved);
/// <summary>
/// 关闭设备
/// </summary>
/// <param name="device_handle"></param>
/// <returns></错误码>
int FkVciCloseDev(SHANDLE device_handle);
/// <summary>
/// 读取协议中缓存数据条数
/// </summary>
/// <param name="channel_handle"></协议句柄>
/// <param name="type"></保留>
/// <returns></缓存中数据条数>
int FkVciGetReceiveNum(SHANDLE channel_handle, UINT8_T type);

/// <summary>
/// 清空接收缓存
/// </summary>
/// <param name="channel_handle"></设备句柄>
/// <returns></错误码>
int FkVciClearReceiveBuf(SHANDLE channel_handle);
#if(0)
/// <summary>
/// 初始化ACAN/Kline
/// </summary>
/// <param name="device_handle"></设备句柄>
/// <param name="can_index"></通道定义>
/// <param name="Baudrate"></协议通讯速率>
/// <returns></协议句柄>
SHANDLE FkVciInitACAN(SHANDLE device_handle, UINT32_T can_index, UINT32_T Baudrate);
/// <summary>
/// 关闭协议通道
/// </summary>
/// <param name="channel_handle"></协议句柄>
/// <returns></故障码>
int FkVciResetACAN(SHANDLE channel_handle);
/// <summary>
/// ACAN/Kline 接收数据
/// </summary>
/// <param name="channel_handle"></协议句柄>
/// <param name="pReceive"></接收缓存地址指针>
/// <param name="len"></想要接收数据条数>
/// <param name="wait_time"></如果没有数据等待时间>
/// <returns></实际接收数据条数>
int FkVciReceiveACAN(SHANDLE channel_handle, FkVciKwpDataType* pReceive, UINT32_T len, int wait_time);
/// <summary>
/// ACAN/Kline 发送数据
/// </summary>
/// <param name="channel_handle"></协议句柄>
/// <param name="pTransmit"></发送数据缓存地址指针>
/// <param name="len"></发送数据条数>
/// <returns></实际发送数据条数>
int FkVciTrsmitACAN(SHANDLE channel_handle, FkVciKwpDataType* pTransmit, UINT32_T len);
/// <summary>
/// 配置协议参数
/// </summary>
/// <param name="channel_handle"></协议句柄>
/// <param name="ConfigId"></配置参数ID 具体见xxx>
/// <param name="pInitConfig"></配置参数>
/// <returns></错误码>
int FkVciConfigACAN(SHANDLE channel_handle, UINT32_T ConfigId, void* pInitConfig);
#endif
/// <summary>
/// 初始化CAN 如果不使用CANFD 不配置滤波 调用后就可以直接使用了
/// </summary>
/// <param name="device_handle"></设备句柄>
/// <param name="can_index"></CAN通道>
/// <param name="Baudrate"></波特率>
/// <returns></协议句柄>
SHANDLE FkVciInitCAN(SHANDLE device_handle, UINT32_T can_index, UINT32_T Baudrate);
/// <summary>
/// 
/// </summary>
/// <param name="device_handle"></设备句柄>
/// <param name="can_index"></通道0-1>
/// <param name="Baudrate"></同步波特率>
/// <param name="DataBaudRate"></数据波特率>
/// <returns></打开成功通道句柄 失败为空>
SHANDLE FkVciInitCANFD(SHANDLE device_handle, UINT32_T can_index, UINT32_T Baudrate, UINT32_T DataBaudRate);

/// <summary>
/// 
/// </summary>
/// <param name="device_handle"></通道句柄>
/// <param name="can_index"></通道0-1>
/// <param name="StartId"></开始ID>
/// <param name="StopId"></结束ID>
/// <returns></returns>
int FkVciSetFilterCAN(SHANDLE device_handle, UINT32_T can_index, UINT32_T StartId, UINT32_T StopId);
/// <summary>
/// 复位CAN协议通道
/// </summary>
/// <param name="channel_handle"></协议通道句柄>
/// <returns></错误码>
int FkVciResetCAN(SHANDLE channel_handle);
/// <summary>
/// CAN 接收
/// </summary>
/// <param name="channel_handle"></协议句柄>
/// <param name="pReceive"></接收数据缓存地址>
/// <param name="len"></接收数据条数>
/// <param name="wait_time"></等待时间>
/// <returns></实际接收数据条数>
int FkVciReceiveCAN(SHANDLE channel_handle, FkVciCanDataType* pReceive, UINT32_T len, int wait_time);
/// <summary>
/// CAN 发送
/// </summary>
/// <param name="channel_handle"></协议句柄>
/// <param name="pTransmit"></发送数据缓存地址>
/// <param name="len"></请求发送条数 需与协议地址配对 否则内存读取错误>
/// <returns></实际发送条数>
int FkVciTrsmitCAN(SHANDLE channel_handle, FkVciCanDataType* pTransmit, UINT32_T len);
/// <summary>
/// 配置CAN  CANFD 滤波参数
/// </summary>
/// <param name="channel_handle"></param>
/// <param name="ConfigId"></param>
/// <param name="pInitConfig"></param>
/// <returns></returns>
int FkVciConfigCAN(SHANDLE channel_handle, UINT32_T ConfigId, void* pInitConfig);

/*******************************************************************************/
/// <summary>
/// 
/// </summary>
/// <param name="device_handle"></param>
/// <param name="can_index"></通道>
/// <param name="mode"></主机0 从机1>
/// <param name="Baudrate"></波特率>
/// <returns></通道句柄 失败为空>
SHANDLE FkVciInitLIN(SHANDLE device_handle, UINT32_T can_index, UINT32_T mode, UINT32_T Baudrate);
/// <summary>
/// 
/// </summary>
/// <param name="channel_handle"></通道句柄>
/// <returns></returns>
int FkVciResetLIN(SHANDLE channel_handle);
/// <summary>
/// 在设置了SCH 后 或则设置了 Slave后 调用该接口函数 读取数据
/// </summary>
/// <param name="channel_handle"></通道句柄>
/// <param name="pReceive"></接收缓存>
/// <param name="len"></想要接收数据条数>
/// <param name="wait_time"></等待时间>
/// <returns></实际接收数据条数>
int FkVciRecieveLIN(SHANDLE channel_handle, FkVciLinDataType* pReceive, UINT32_T len, int wait_time);
/// <summary>
/// 主机模式下 使用改接口单次发送或读取数据  单次发送多条数据
/// 该模式与 SCH模式不能同时使用 一旦开启了 SCH 调用该接口 失败
/// </summary>
/// <param name="channel_handle"></param>
/// <param name="pTransmit"></请求数据地址>
/// <param name="pReceive"></接收数据地址>
/// <param name="len"></请求数据条数>
/// <returns></返回数据条数 理论上应该等于请求数据条数>
int FkVciTrsmitLIN(SHANDLE channel_handle, FkVciLinDataType* pTransmit, FkVciLinDataType* pReceive, UINT32_T len);
/// <summary>
/// 主模式下 设置内部周期调度 并启动LIN  
/// 从模式下 配置一个调度表
/// </summary>
/// <param name="channel_handle"></通道句柄>
/// <param name="pTransmit"></请求数据开始地址>
/// <param name="len"></请求数据条数>
/// <returns></错误码>
int FkVciLinSchStart(SHANDLE channel_handle, FkVciLinDataType* pTransmit, UINT32_T len);
/// <summary>
/// 主模式下 关闭内部周期调度
/// 从模式下 清空从机数据
/// </summary>
/// <param name="channel_handle"></通道句柄>
/// <returns></错误码>
int FkVciLinSchStop(SHANDLE channel_handle);
/// <summary>
/// 从机模式下 修改/添加一条数据信息 
/// </summary>
/// <param name="channel_handle"></param>
/// <param name="pTransmit"></param>
/// <returns></returns>
int FkVciLinSlaveWriteMsg(SHANDLE channel_handle, FkVciLinDataType* pTransmit);
/// <summary>
/// 后期扩展功能使用
/// </summary>
/// <param name="channel_handle"></param>
/// <param name="ConfigId"></param>
/// <param name="pInitConfig"></param>
/// <returns></returns>
int FkVciConfigLIN(SHANDLE channel_handle, UINT32_T ConfigId, void* pInitConfig);


SHANDLE FkVciInitSent(SHANDLE device_handle, UINT32_T can_index, UINT32_T mode, UINT32_T Baudrate, UINT32_T CycleMs);

int FkVciResetSent(SHANDLE channel_handle);

int FkVciRecievSent(SHANDLE channel_handle, FkVciSentDataType* pReceive, UINT32_T len, int wait_time);
