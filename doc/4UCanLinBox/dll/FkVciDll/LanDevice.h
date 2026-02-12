#pragma once

#pragma comment(lib, "ws2_32.lib")
#include "BaseDevice.h"
#include"winsock.h"

#define LAN_PORT_NUM 21001

class LanDevice :public BaseDevice
{
public:
	LanDevice();
	~LanDevice();

	virtual int DevRecieve();
	virtual int DevOpen(void* pName);
	virtual int DevClose();
	virtual int DevTrsmit(const uint8_t* data, int len);/**/

	uint8_t DataCheckSum(const uint8_t* buf, int len);/*校验和*/
	bool DevDataToPcCov(uint8_t* buf);/*转换数据-接收一条完整的PC数据*/
private:
	SOCKET sockclient;
};