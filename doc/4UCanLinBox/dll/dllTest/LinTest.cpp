#include"stdio.h"
#include "FkVciDll.h"
#include "windows.h"
#pragma comment(lib,"FkVciDll.lib")

#define LIN_TESTCH 0

#define TEST_MASTER_WRITE 0
#define TEST_MASTER_SCH  1


//通道2 设置为主机  通道1设置为从机   将两个DB9的LIN 连起来
void LinMixTestSequeue()
{
	HANDLE DeviceHandle;
	HANDLE LinChanl1Handle = NULL;
	HANDLE LinChanl2Handle;

	printf("LinTest---------------------------\r\n");//测试非配对情况下 是否正常link
	/*Step1 Open Dev*/
	DeviceHandle = FkVciOpenDev(0, 4, 0);
	if (DeviceHandle == NULL)
	{
		printf("OpenDev Failed Test Faill!!!!!!\r\n");
		return;
	}
		
	///*Step2 Open Chanl */
	LinChanl1Handle = FkVciInitLIN(DeviceHandle, 0, 0, 19200);//设置主机
	if (LinChanl1Handle == NULL)
	{
		printf("OpenLin1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle);
		return;
	}
	Sleep(100);
	LinChanl2Handle = FkVciInitLIN(DeviceHandle, 1, 1, 19200);//设置Slave
	if (LinChanl2Handle == NULL)
	{
		printf("OpenLin2 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle);
		return;
	}
	/*Step3 Close Chanl */
	FkVciLinDataType MasterTrsmitBuf[5];
	for (int i = 0; i < 5; i++)
	{
		MasterTrsmitBuf[i].MsgType = LIN_EX_MSG_TYPE_MW;
		MasterTrsmitBuf[i].ChekcType = i%2;
		MasterTrsmitBuf[i].LinID = i+1;
		MasterTrsmitBuf[i].DLC = 2;
		MasterTrsmitBuf[i].Data[0] = 0x01;
		MasterTrsmitBuf[i].Data[1] = 0x02;
		MasterTrsmitBuf[i].TimesampL = 10;
	}
	MasterTrsmitBuf[2].MsgType = LIN_EX_MSG_TYPE_MR;//主机读
	MasterTrsmitBuf[4].MsgType = LIN_EX_MSG_TYPE_MR;//主机读
	FkVciLinDataType MasterReceiveBuf[5];
	//配置Slave 响应节点
	FkVciLinDataType SlaveTrsmitBuf[5];
	SlaveTrsmitBuf[0].MsgType = LIN_EX_MSG_TYPE_SW;
	SlaveTrsmitBuf[0].ChekcType = 1;
	SlaveTrsmitBuf[0].LinID = 3;
	SlaveTrsmitBuf[0].DLC = 2;
	SlaveTrsmitBuf[0].Data[0] = 0x01;
	SlaveTrsmitBuf[0].Data[1] = 0x02;
	SlaveTrsmitBuf[0].TimesampL = 10;
	FkVciLinSlaveWriteMsg(LinChanl2Handle, &SlaveTrsmitBuf[0]);//把配置数据传输下去
	SlaveTrsmitBuf[1].MsgType = LIN_EX_MSG_TYPE_SW;
	SlaveTrsmitBuf[1].ChekcType = 1;
	SlaveTrsmitBuf[1].LinID = 5;
	SlaveTrsmitBuf[1].DLC = 4;
	SlaveTrsmitBuf[1].Data[0] = 0x01;
	SlaveTrsmitBuf[1].Data[1] = 0x02;
	SlaveTrsmitBuf[1].Data[2] = 0x01;
	SlaveTrsmitBuf[1].Data[3] = 0x02;
	SlaveTrsmitBuf[1].TimesampL = 10;
	FkVciLinSlaveWriteMsg(LinChanl2Handle, &SlaveTrsmitBuf[1]);//把配置数据传输下去
	FkVciLinDataType SlaveReceiveBuf[5];

	Sleep(100);

	FkVciClearReceiveBuf(LinChanl1Handle);
	FkVciClearReceiveBuf(LinChanl2Handle);
	int ReadNum;
#if(TEST_MASTER_WRITE)
	int WrRet = FkVciTrsmitLIN(LinChanl1Handle, MasterTrsmitBuf, MasterReceiveBuf, 5);//使用通道1 主机发送
	//使用从节点接收数据
	Sleep(10);
	int MsgNum = FkVciGetReceiveNum(LinChanl2Handle, 0);
	ReadNum = FkVciRecieveLIN(LinChanl2Handle, SlaveReceiveBuf, 5, 0);
	if (WrRet != MsgNum || WrRet==0)
	{
		printf("Lin1TxNum:%d  Lin2RxNum%d \r\n", WrRet, MsgNum);


		for (int loop = 0; loop < WrRet; loop++)
		{
			if (memcmp(MasterReceiveBuf[loop].Data, SlaveReceiveBuf[loop].Data,8) != 0)
			{
				printf("MasterWriteMsg:LinID=%d  LinDlc=%d  LinData=%02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X Lin_Cs=%02X\r\n",
					MasterReceiveBuf[loop].LinID, MasterReceiveBuf[loop].DLC, MasterReceiveBuf[loop].Data[0], MasterReceiveBuf[loop].Data[1], MasterReceiveBuf[loop].Data[2], MasterReceiveBuf[loop].Data[3],
					MasterReceiveBuf[loop].Data[4], MasterReceiveBuf[loop].Data[5], MasterReceiveBuf[loop].Data[6], MasterReceiveBuf[loop].Data[7], MasterReceiveBuf[loop].Check);
				printf("SlaveReceiveBuf:LinID=%d  LinDlc=%d  LinData=%02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X Lin_Cs=%02X\r\n",
					SlaveReceiveBuf[loop].LinID, SlaveReceiveBuf[loop].DLC, SlaveReceiveBuf[loop].Data[0], SlaveReceiveBuf[loop].Data[1], SlaveReceiveBuf[loop].Data[2], SlaveReceiveBuf[loop].Data[3],
					SlaveReceiveBuf[loop].Data[4], SlaveReceiveBuf[loop].Data[5], SlaveReceiveBuf[loop].Data[6], SlaveReceiveBuf[loop].Data[7], SlaveReceiveBuf[loop].Check);
				printf("LinTestFail!!!\r\n");
			}
		}


		printf("LinTestFail!!!\r\n");
		FkVciResetLIN(LinChanl1Handle);
		FkVciResetLIN(LinChanl2Handle);
		FkVciCloseDev(DeviceHandle);
		return;


	}
	else
	{
			for (int loop = 0; loop < 5; loop++)
			{
				if (memcmp(MasterReceiveBuf[loop].Data, SlaveReceiveBuf[loop].Data, 8) != 0)
				{
					printf("MasterWriteMsg:LinID=%d  LinDlc=%d  LinData=%02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X Lin_Cs=%02X\r\n",
						MasterReceiveBuf[loop].LinID, MasterReceiveBuf[loop].DLC, MasterReceiveBuf[loop].Data[0], MasterReceiveBuf[loop].Data[1], MasterReceiveBuf[loop].Data[2], MasterReceiveBuf[loop].Data[3],
						MasterReceiveBuf[loop].Data[4], MasterReceiveBuf[loop].Data[5], MasterReceiveBuf[loop].Data[6], MasterReceiveBuf[loop].Data[7], MasterReceiveBuf[loop].DLC);
					printf("SlaveReceiveBuf:LinID=%d  LinDlc=%d  LinData=%02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X Lin_Cs=%02X\r\n",
						SlaveReceiveBuf[loop].LinID, SlaveReceiveBuf[loop].DLC, SlaveReceiveBuf[loop].Data[0], SlaveReceiveBuf[loop].Data[1], SlaveReceiveBuf[loop].Data[2], SlaveReceiveBuf[loop].Data[3],
						SlaveReceiveBuf[loop].Data[4], SlaveReceiveBuf[loop].Data[5], SlaveReceiveBuf[loop].Data[6], SlaveReceiveBuf[loop].Data[7], SlaveReceiveBuf[loop].DLC);
					
					
					printf("LinTestFail!!!\r\n");
					//FkVciResetLIN(LinChanl1Handle);
					//FkVciResetLIN(LinChanl2Handle);
					//FkVciCloseDev(DeviceHandle);
					//return;
				}
			}
	}

#endif

#if(TEST_MASTER_SCH)//主机使用调度模式发送
	int Ret = FkVciLinSchStart(LinChanl1Handle, MasterTrsmitBuf, 5);
	Sleep(1000);
	int MasterReadNum = FkVciGetReceiveNum(LinChanl1Handle, 0);//主机读取数据条数目
	ReadNum = FkVciRecieveLIN(LinChanl1Handle, SlaveReceiveBuf, 5, 0);
	int SlaverReadNum = FkVciGetReceiveNum(LinChanl2Handle, 0);//从机读取数据条数
	ReadNum = FkVciRecieveLIN(LinChanl2Handle, SlaveReceiveBuf, 5, 0);

	FkVciClearReceiveBuf(LinChanl1Handle);
	FkVciClearReceiveBuf(LinChanl2Handle);
	//更改主/从机中的数据
	FkVciLinSlaveWriteMsg(LinChanl1Handle, &MasterTrsmitBuf[0]);//把配置数据传输下去

	FkVciLinSchStop(LinChanl1Handle);
	Sleep(100);
	Ret = FkVciLinSchStart(LinChanl1Handle, MasterTrsmitBuf, 5);

#endif

	printf("LinTestSucess!!!\r\n");
Sleep(500);
//FkVciResetLIN(LinChanl1Handle);
//FkVciResetLIN(LinChanl2Handle);
FkVciCloseDev(DeviceHandle);

}


void LinBcsTestSequeue()
{
	HANDLE DeviceHandle;
	HANDLE LinChanl1Handle = NULL;
	HANDLE LinChanl2Handle;

	printf("LinTest---------------------------\r\n");//测试非配对情况下 是否正常link
	/*Step1 Open Dev*/
	DeviceHandle = FkVciOpenDev(0, 4, 0);
	if (DeviceHandle == NULL)
	{
		printf("OpenDev Failed Test Faill!!!!!!\r\n");
		return;
	}

	///*Step2 Open Chanl */
	LinChanl1Handle = FkVciInitLIN(DeviceHandle, 0, 0, 19200);//设置主机
	if (LinChanl1Handle == NULL)
	{
		printf("OpenLin1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle);
		return;
	}

	/*Step3 Close Chanl */
	FkVciLinDataType MasterTrsmitBuf[6];

	MasterTrsmitBuf[0].MsgType = LIN_EX_MSG_TYPE_MW;
	MasterTrsmitBuf[0].ChekcType = 1;
	MasterTrsmitBuf[0].LinID = 0x13;
	MasterTrsmitBuf[0].DLC = 2;
	MasterTrsmitBuf[0].Data[0] = 0xFA;
	MasterTrsmitBuf[0].Data[1] = 0xFF;
	MasterTrsmitBuf[0].TimesampL = 10;

	MasterTrsmitBuf[1].MsgType = LIN_EX_MSG_TYPE_MW;
	MasterTrsmitBuf[1].ChekcType = 1;
	MasterTrsmitBuf[1].LinID = 0x0D;
	MasterTrsmitBuf[1].DLC = 4;
	MasterTrsmitBuf[1].Data[0] = 0x7E;
	MasterTrsmitBuf[1].Data[1] = 0x71;
	MasterTrsmitBuf[1].Data[2] = 0x10;
	MasterTrsmitBuf[1].Data[3] = 0x00;
	MasterTrsmitBuf[1].TimesampL = 10;

	MasterTrsmitBuf[2].MsgType = LIN_EX_MSG_TYPE_MW;
	MasterTrsmitBuf[2].ChekcType = 1;
	MasterTrsmitBuf[2].LinID = 0x11;
	MasterTrsmitBuf[2].DLC = 8;
	MasterTrsmitBuf[2].Data[0] = 0x00;
	MasterTrsmitBuf[2].Data[1] = 0x80;
	MasterTrsmitBuf[2].Data[2] = 0xFE;
	MasterTrsmitBuf[2].Data[3] = 0xFE;
	MasterTrsmitBuf[2].Data[4] = 0xFE;
	MasterTrsmitBuf[2].Data[5] = 0xFE;
	MasterTrsmitBuf[2].Data[6] = 0xFF;
	MasterTrsmitBuf[2].Data[7] = 0xFF;
	MasterTrsmitBuf[2].TimesampL = 10;

	MasterTrsmitBuf[3].MsgType = LIN_EX_MSG_TYPE_MR;
	MasterTrsmitBuf[3].LinID = 0x3A;
	MasterTrsmitBuf[3].TimesampL = 10;

	MasterTrsmitBuf[4].MsgType = LIN_EX_MSG_TYPE_MR;
	MasterTrsmitBuf[4].LinID = 0xE;
	MasterTrsmitBuf[4].TimesampL = 10;

	MasterTrsmitBuf[5].MsgType = LIN_EX_MSG_TYPE_MR;
	MasterTrsmitBuf[5].LinID = 0xF;
	MasterTrsmitBuf[5].TimesampL = 10;
	FkVciLinDataType MasterReceiveBuf[6];

	while (1)
	{
		int WrRet = FkVciTrsmitLIN(LinChanl1Handle, MasterTrsmitBuf, MasterReceiveBuf, 6);

		for (int loop = 0; loop < 6; loop++)
		{

			printf("SlaveReceiveBuf:LinID=%d  LinDlc=%d  LinData=%02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X Lin_Cs=%02X\r\n",
				MasterReceiveBuf[loop].LinID, MasterReceiveBuf[loop].DLC, MasterReceiveBuf[loop].Data[0], MasterReceiveBuf[loop].Data[1], MasterReceiveBuf[loop].Data[2], MasterReceiveBuf[loop].Data[3],
				MasterReceiveBuf[loop].Data[4], MasterReceiveBuf[loop].Data[5], MasterReceiveBuf[loop].Data[6], MasterReceiveBuf[loop].Data[7], MasterReceiveBuf[loop].DLC);
		}

	}
	

	//FkVciResetLIN(LinChanl1Handle);
	//FkVciResetLIN(LinChanl2Handle);
	FkVciCloseDev(DeviceHandle);

}
