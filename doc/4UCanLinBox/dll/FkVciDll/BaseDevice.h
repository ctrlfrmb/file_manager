#pragma once
#include<iostream>
#include <windows.h>
using namespace std;

#define MAX_PROTOCOL_NUM 6


enum DEVICETYPE
{
	eNULL = 0,
	eLAN = 1,
	eUSB = 2,
	eCOM = 3,
	ePXI = 4
};

class BaseDevice
{
public:
	BaseDevice();
	~BaseDevice();

	virtual int DevRecieve();
	virtual int DevOpen(void* pName);
	virtual int DevClose();
	virtual int DevTrsmit(const uint8_t* data, int len);/**/

	int CreateRecieveThread();             //创建接收线程
	int InsetProtocol(void* protocol_handle);     //插入一条协议
	int DeleteProtocol(void* protocol_handle);    //删除一条协议
	int ClearProtocol();                   //删除所有协议
	HANDLE GetProtocolHandle(uint8_t ProtocolId, uint8_t Chanl);
	void* ProtocolTab[MAX_PROTOCOL_NUM];

	int DevRecieveRunFlag;

	HANDLE hDevRecieveThreadHandler;/*接收任务句柄*/
	HANDLE hDevRecieveThreadExitResponseHandler;/*退出完成句柄*/

	uint8_t DeviceType;
	int     LastErrCode;//记录最后一次故障码信息
	uint32_t  DeviceIndexNum;//Dev 序号

private:



};