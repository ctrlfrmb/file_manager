#include"stdio.h"
#include "FkVciDll.h"
#include "windows.h"
#pragma comment(lib,"FkVciDll.lib")


void SentTestSequeue2()
{
	HANDLE DeviceHandle;
	HANDLE SentChanlHandle;
	HANDLE SentChanlHandle1;
	printf("---------------------------\r\n");//测试非配对情况下 是否正常link
	/*Step1 Open Dev*/
	DeviceHandle = FkVciOpenDev(0, 4, 0);
	if (DeviceHandle == NULL)
	{
		printf("OpenDev Failed Test Faill!!!!!!\r\n");
	}
	SentChanlHandle = FkVciInitSent(DeviceHandle, 0, 2,6,10);
	if (SentChanlHandle == NULL)
	{
		printf("OpenCan1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle);
	}
	SentChanlHandle1 = FkVciInitSent(DeviceHandle, 1, 2, 3, 10);
	if (SentChanlHandle1 == NULL)
	{
		printf("OpenCan1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle);
	}

	Sleep(1000);

	int RecCh1Num = FkVciGetReceiveNum(SentChanlHandle,0);
	FkVciSentDataType SentRecTempDataBuf[10];
	int RecNum = FkVciRecievSent(SentChanlHandle, SentRecTempDataBuf,10,10);
	FkVciSentDataType SentRecTempDataBuf1[10];
	int RecCh2Num = FkVciGetReceiveNum(SentChanlHandle1, 0);

	RecNum = FkVciRecievSent(SentChanlHandle1, SentRecTempDataBuf1, 10, 10);




	FkVciCloseDev(DeviceHandle);
	Sleep(10);

}