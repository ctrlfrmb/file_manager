#include "pch.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
//#include <mswsock.h>
//#include <windows.h>
#include "LanDevice.h"
#include "FkVciDll.h"
#include "VciDebugCf.h"
#include "BaseProtocol.h"

#define LINK_USE_UNBLOCK_MODE 1 //

#ifndef SIO_TCP_SET_ACK_FREQUENCY
#define SIO_TCP_SET_ACK_FREQUENCY _WSAIOW(IOC_VENDOR, 23)
#endif

//#define _WIN32_WINNT 0x0601 // Windows 7

LanDevice::LanDevice()
{
	debugout("LanDevice Create ...\r\n");
	sockclient = INVALID_SOCKET;
	DeviceType = eLAN;
}

LanDevice::~LanDevice()
{
	debugout("LanDevice Delete ...\r\n");
	sockclient = INVALID_SOCKET;
}

int LanDevice::DevOpen(void* pName)
{
	debugout("LanDevice DevOpen ...\r\n");
	/*先打开硬件设备接口*/
	//创建socket
	char ipaddrstring[20];
	WSADATA wsaData;
	int result;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		// WSAStartup 失败
		debugout("error: LanDevice DevOpen by WSAStartup ...\r\n");
		return -1;
	}

	sockclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sockclient)
	{
		WSACleanup();//清空WSACleanup
		return -1;
	}

	//DWORD bytesReturned = 0;
	//int ackFrequency = 1; // 设置为1表示每个接收到的包都发送ACK
	//result = WSAIoctl(sockclient, SIO_TCP_SET_ACK_FREQUENCY, &ackFrequency, sizeof(ackFrequency), NULL, 0, &bytesReturned, NULL, NULL);
	//if (result == SOCKET_ERROR) {
	//	debugout("error: LanDevice DevOpen set SIO_TCP_SET_ACK_FREQUENCY ...\r\n");
	//	// 处理错误
	//}

	// 设置 TCP_NODELAY 选项
	//BOOL enable = TRUE;
	//if (setsockopt(sockclient, IPPROTO_TCP, TCP_NODELAY, (const char*)&enable, sizeof(enable)) == SOCKET_ERROR)/*LWIP 必须设置否则延迟很大*/
	//{
	//	debugout("error: LanDevice DevOpen set TCP_NODELAY ...\r\n");
	//	enable = FALSE;
	//}

	//// 设置接收缓冲区大小
	//int rcvBufferSize = 156; // 设置为1MB，根据需要调整大小
	//if (setsockopt(sockclient, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBufferSize, sizeof(rcvBufferSize)) == SOCKET_ERROR) {
	//	// 设置缓冲区大小失败的处理
	//	closesocket(sockclient);
	//	WSACleanup();
	//	debugout("error: LanDevice DevOpen set SO_RCVBUF ...\r\n");
	//	return -1;
	//}

	//// 设置 TCP_QUICKACK 选项
	//if (setsockopt(sockclient, IPPROTO_TCP, TCP_QUICKACK, (const char*)&enable, sizeof(enable)) == SOCKET_ERROR)/*LWIP 必须设置否则延迟很大*/
	//{
	//	debugout("error: LanDevice DevOpen set TCP_QUICKACK ...\r\n");
	//	enable = FALSE;
	//}

	//连接服务器，建立服务器端套接字地址
	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(LAN_PORT_NUM);//设置TCP 通讯的端口号
	//对于inet_addr()函数，它是把“xxx.xxx.xxx.xxx”形式表示的IPV4地址，转换为IN_ADDR结构体能够接收的形式（unsigned long型，因为IN_ADDR结构体中的负责接收的S_addr成员变量的类型是unsigned long型）
	strcpy_s(ipaddrstring, (const char*)(pName));
	addr.sin_addr.S_un.S_addr = inet_addr(ipaddrstring);//目标地址
	DeviceIndexNum = addr.sin_addr.S_un.S_addr;//
#if(LINK_USE_UNBLOCK_MODE)
	// 设置为非阻塞的socket  
	int iMode = 1;
	ioctlsocket(sockclient, FIONBIO, (u_long FAR*) & iMode);
	// 超时时间  
	struct timeval tm;
	tm.tv_sec = 3;
	tm.tv_usec = 0;
	int ret = -1;
	// 尝试去连接服务端  
	if (-1 != connect(sockclient, (SOCKADDR*)&addr, sizeof(SOCKADDR)))
	{
		ret = 1; // 连接成功  
	}
	else
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(sockclient, &set);

		if (select(-1, NULL, &set, NULL, &tm) <= 0)
		{
			ret = -1; // 有错误(select错误或者超时)  
		}
		else
		{
			int error = -1;
			int optLen = sizeof(int);
			getsockopt(sockclient, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen);
			// 之所以下面的程序不写成三目运算符的形式， 是为了更直观， 便于注释  
			if (0 != error)
			{
				ret = -1; // 有错误  
			}
			else
			{
				ret = 1;  // 无错误  
			}
		}
	}
	// 设回为阻塞socket  
	iMode = 0;
	ioctlsocket(sockclient, FIONBIO, (u_long FAR*) & iMode); //设置为阻塞模式  
#else
	//向服务器发出连接请求，当然我们也可以通过connet函数的返回值判断到底有无连接成功。
	int ret = connect(sockclient, (struct sockaddr*)&addr, sizeof(addr));//使用TCP 连接设备
#endif
	if (SOCKET_ERROR == ret)
	{
		closesocket(sockclient);//关闭socket
		WSACleanup();//清空WSACleanup
		debugout("LanDevice Connect Tcp Fail ...\r\n");
		return ERRCODE_OPENDEV_FAILED;
	}
	int nNetTimeout = 1000;
	setsockopt(sockclient, SOL_SOCKET, SO_SNDTIMEO, (char*)&nNetTimeout, sizeof(int));//设置发送超时时间1S
	setsockopt(sockclient, SOL_SOCKET, SO_RCVTIMEO, (char*)&nNetTimeout, sizeof(int));//设置接收超时时间1S

	/*如果建立成功 启动一个线程 异步接收数据  调用 LanDev_RxProcess*/
	return CreateRecieveThread();
}


int LanDevice::DevClose()
{
	debugout("LanDevice DevClose ...\r\n");

	if (DevRecieveRunFlag)
	{
		ClearProtocol();//先关掉所有的DEV 开辟的协议
		DevRecieveRunFlag = 0;//请求关闭接收线程
		closesocket(sockclient);//关闭socket
		if (WaitForSingleObjectEx(hDevRecieveThreadExitResponseHandler, 1000, true) == WAIT_OBJECT_0)/*等待设备释放*/
		{
			WSACleanup();//清空WSACleanup
		}
	}
	return ERRCODE_NOERR;
}

int LanDevice::DevTrsmit(const uint8_t* data, int len)
{
	debugout("LanDevice DevTrsmit ...\r\n");
	uint8_t txbuf[5120];
	txbuf[0] = 0xAA;
	txbuf[1] = 0x55;
	memcpy(txbuf + 2, data, len);
	txbuf[2 + len] = DataCheckSum(txbuf, len);
	int SendNum = send(sockclient, (char*)txbuf, len + 3, 0);/*使用socket 发送数据*/

	debugout("SendReq:%d  SendRet:%d\r\n",(len + 3),SendNum);
	return ERRCODE_NOERR;
}

int LanDevice::DevRecieve()
{
	debugout("LanDevice DevRecieve ...\r\n");
	uint8_t buf[5120];//50K 
	if (DevDataToPcCov(buf))//接收一条完成的报文
	{		
		//unsigned long dataSize;
		//ioctlsocket(sockclient, FIONREAD, &dataSize);/*避免上位机没有收到ACK 等待过久*/
		//if (dataSize == 0)
		//{
		//	char txbuf = 0x00;
		//	send(sockclient, &txbuf, 1, 0);/*使用socket 发送数据*/
		//}
		for (int i = 0; i < MAX_PROTOCOL_NUM; i++)/*查询每个协议并处理*/
		{
			if (ProtocolTab[i] != NULL)/*使用协议处理数据*/
			{
				BaseProtocol* Protocol = (BaseProtocol*)ProtocolTab[i];
				Protocol->ProtocolDealMsg((buf + 2));//去掉头信息交由协议处理
			}
			/**/
		}
	}
	else
	{
		//unsigned long dataSize;
		//ioctlsocket(sockclient, FIONREAD, &dataSize);
		//if (dataSize == 0)
		//{
		//	char txbuf = 0x00;
		//	send(sockclient, &txbuf, 1, 0);/*使用socket 发送数据*/
		//}
		return ERRCODE_LANRECERR;

	}
	return ERRCODE_NOERR;
}

uint8_t LanDevice::DataCheckSum(const uint8_t* buf, int len)
{
	return 0;
}



bool LanDevice::DevDataToPcCov(uint8_t* buf)
{
	unsigned char* ptr;
	int recnum = recv(sockclient, (char*)buf, 1, 0);//先接收7个字节  接收到长度的位置
	if (recnum == 0 || (uint8_t)buf[0]!=0x55)
		return false;
	recnum = recv(sockclient, (char*)(buf+1), 1, 0);//先接收7个字节  接收到长度的位置
	if (recnum == 0 || (uint8_t)buf[1]!=0xAA)
		return false;

	if ((uint8_t)buf[0] == (uint8_t)0x55 && (uint8_t)buf[1] == (uint8_t)0xAA)//
	{
		recnum = recv(sockclient, (char*)(buf + 2), 6, 0);//先接收7个字节  接收到长度的位置
		if (recnum == 6)
		{
			ptr = (unsigned char*)buf;
			uint16_t msgnum = *(uint16_t*)(buf + 6) + 1;
			recnum = recv(sockclient, (char*)(buf + 8), msgnum, 0);//接收协议剩余的长度
			if (recnum == msgnum)//接收到剩余的数据
			{
				return true;
			}
			else//接收失败
			{
				debugout("Lan Device Recieve Data Timeout ...\r\n ");
				return false;
			}
		}
		else
		{
			debugout("Lan Device Recieve Cmd And Len Timeout ...\r\n ");
			return false;
		}
	}
	else
	{
		//if (recnum == 2)
		//{
		//	debugout("Lan Device Recieve A Unknow Data ->HeadErr ...\r\n ");
		//}
		//else
		//{
		//	//debugout("Lan Device Recieve Timeout ...\r\n ");
		//}
		return false;//没有接收到指定的数据
	}
}


int DevGetCurrentTime()
{

	return 0;
}
