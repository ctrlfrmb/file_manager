#include"stdio.h"
#include "FkVciDll.h"
#include "windows.h"
#pragma comment(lib,"FkVciDll.lib")

#define TEST_CH1EN 1
#define TEST_CH2EN 1
void CanTestSequeue()
{

	HANDLE DeviceHandle, DeviceHandle1;
	HANDLE CanChanlHandle, CanChanlHandle2;
	HANDLE CanChanlHandle1;
	printf("---------------------------\r\n");//测试非配对情况下 是否正常link
	/*Step1 Open Dev*/
	DeviceHandle = FkVciOpenDev(0, 5, 0);

	DeviceHandle1 = FkVciOpenDev(0, 5, 0);

	if (DeviceHandle == NULL || DeviceHandle!= DeviceHandle1)
	{
		printf("OpenDev Failed Test Faill!!!!!!\r\n");
		return;
	}
	printf("OpenDev Sucess !!!!!!\r\n");
	CanChanlHandle2 = FkVciInitCANFD(DeviceHandle, 0, 500000,2000000);
	CanChanlHandle = FkVciInitCANFD(DeviceHandle, 0, 500000,2000000);
	if (CanChanlHandle == NULL || CanChanlHandle2!= CanChanlHandle)
	{
		printf("OpenCan1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle);
		return;
	}
	printf("OpenCan1 Sucess !!!!!!\r\n");

	DeviceHandle1 = FkVciOpenDev(0, 5, 0);

	if (DeviceHandle == NULL || DeviceHandle != DeviceHandle1)
	{
		printf("OpenDev Failed Test Faill!!!!!!\r\n");
		return;
	}




	/*Step2 Open Chanl */
#if(TEST_CH1EN)
	CanChanlHandle = FkVciInitCANFD(DeviceHandle, 0, 500000,2000000);
	if (CanChanlHandle == NULL)
	{
		printf("OpenCan1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle);
		return;
	}
	printf("OpenCan1 Sucess !!!!!!\r\n");
#endif
#if(TEST_CH2EN)
	CanChanlHandle1 = FkVciInitCANFD(DeviceHandle, 1, 500000,2000000);
	if (CanChanlHandle1 == NULL)
	{
		printf("OpenCan2 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(CanChanlHandle1);
		return;
	}
	printf("OpenCan2 Sucess !!!!!!\r\n");
#endif

	int CntTotal = 0;
	uint32_t iCNT = 0;
	FkVciCanDataType RxMsgTemp;



	while (1)
	{

		int rxNum = FkVciGetReceiveNum(CanChanlHandle, 0);
		CntTotal += rxNum ;		

		int rxNum1 = FkVciGetReceiveNum(CanChanlHandle1, 0);
		CntTotal += rxNum1;

		
		//FkVciClearReceiveBuf(CanChanlHandle);
		//FkVciClearReceiveBuf(CanChanlHandle1);
		
		printf("Total:%d  Chanl1RxNum:%d  Chanl2RxNum:%d\r\n", CntTotal, rxNum, rxNum1);
		if (rxNum > 0)
		{
			while (rxNum--)
			{
				FkVciReceiveCAN(CanChanlHandle, &RxMsgTemp, 1, 0);
			}
		}

		if (rxNum1 > 0)
		{
			while (rxNum1--)
			{
				FkVciReceiveCAN(CanChanlHandle1, &RxMsgTemp, 1, 0);
			}
		}
		for (int loop = 0; loop < 50; loop++)
		{
			////if (1)
			{
				FkVciCanDataType canmsg;
				canmsg.CanID = 0x123;
				canmsg.DLC = 6;
				canmsg.FLAG = 0;
				*(uint32_t*)(canmsg.Data) = iCNT;
				canmsg.Data[4] = 5;
				canmsg.Data[5] = 6;
				canmsg.Data[6] = 7;
				canmsg.Data[7] = 8;
				FkVciTrsmitCAN(CanChanlHandle, &canmsg, 1);
				iCNT++;
			}
			////if (rxNum1 != 0)
			{
				FkVciCanDataType canmsg;
				canmsg.CanID = 0x124;
				canmsg.DLC = 6;
				canmsg.FLAG = 0;
				canmsg.Data[0] = 1;
				canmsg.Data[1] = 2;
				canmsg.Data[2] = 3;
				canmsg.Data[3] = 4;
				canmsg.Data[4] = 5;
				canmsg.Data[5] = 6;
				canmsg.Data[6] = 7;
				canmsg.Data[7] = 8;
				FkVciTrsmitCAN(CanChanlHandle1, &canmsg, 1);
			}
		}

		Sleep(100);
		//for (int loop = 0; loop < 10; loop++)
		//{
		//	//if (rxNum != 0)
		//	//{
		//	//	FkVciCanDataType canmsg;
		//	//	canmsg.CanID = 0x123;
		//	//	canmsg.DLC = 6;
		//	//	canmsg.FLAG = 0;
		//	//	canmsg.Data[0] = 1;
		//	//	canmsg.Data[1] = 2;
		//	//	canmsg.Data[2] = 3;
		//	//	canmsg.Data[3] = 4;
		//	//	canmsg.Data[4] = 5;
		//	//	canmsg.Data[5] = 6;
		//	//	canmsg.Data[6] = 7;
		//	//	canmsg.Data[7] = 8;
		//	//	FkVciTrsmitCAN(CanChanlHandle1, &canmsg, 1);
		//	//}
		//	//if (rxNum1 != 0)
		//	//{
		//	//	FkVciCanDataType canmsg;
		//	//	canmsg.CanID = 0x123;
		//	//	canmsg.DLC = 6;
		//	//	canmsg.FLAG = 0;
		//	//	canmsg.Data[0] = 1;
		//	//	canmsg.Data[1] = 2;
		//	//	canmsg.Data[2] = 3;
		//	//	canmsg.Data[3] = 4;
		//	//	canmsg.Data[4] = 5;
		//	//	canmsg.Data[5] = 6;
		//	//	canmsg.Data[6] = 7;
		//	//	canmsg.Data[7] = 8;
		//	//	FkVciTrsmitCAN(CanChanlHandle2, &canmsg, 1);
		//	//}
		//}

		//Sleep(50);
	}

	int InitRxNum = FkVciGetReceiveNum(CanChanlHandle, 0);

	Sleep(500);
	/*Step3 Close Chanl */
	FkVciCanDataType canmsg;
	canmsg.CanID = 0x123;
	canmsg.DLC = 6;
	canmsg.FLAG = 0;
	canmsg.Data[0] = 1;
	canmsg.Data[1] = 2;
	canmsg.Data[2] = 3;
	canmsg.Data[3] = 4;
	canmsg.Data[4] = 5;
	canmsg.Data[5] = 6;
	canmsg.Data[6] = 7;
	canmsg.Data[7] = 8;
#if(TEST_CH1EN)
	for (int c = 0; c < 10; c++)
	{

		for (int i = 0; i < 40; i++)
		{
			*(uint32_t*)canmsg.Data = i;
			*(uint32_t*)(canmsg.Data+4) = c;
			FkVciTrsmitCAN(CanChanlHandle, &canmsg, 1);
		}
		Sleep(10);
	}
#endif
#if(TEST_CH1EN)
	for (int c = 0; c < 10; c++)
	{

		for (int i = 0; i < 40; i++)
		{
			*(uint32_t*)canmsg.Data = i;
			*(uint32_t*)(canmsg.Data + 4) = c;
			FkVciTrsmitCAN(CanChanlHandle1, &canmsg, 1);
		}
		Sleep(10);
	}
#endif
	Sleep(100);
	/////*Step4 Close Dev*/
#if(TEST_CH1EN&&TEST_CH2EN)
	int rxNum = FkVciGetReceiveNum(CanChanlHandle,0);
	int rxNum1 = FkVciGetReceiveNum(CanChanlHandle1, 0);
	if (rxNum == 400 && rxNum1==400)
	{
		printf("CanTestSuc!!!!!!\r\n");
	}
	else
	{
		printf("Can1RxNum:%d  Can2RxNum:%d\r\n", rxNum, rxNum1);
	}  
	FkVciCanDataType canmsgrx;

	FkVciReceiveCAN(CanChanlHandle1,&canmsgrx,1,10);

#endif
#if(TEST_CH1EN)
	FkVciResetCAN(CanChanlHandle);
#endif
#if(TEST_CH2EN)
	FkVciResetCAN(CanChanlHandle1);
#endif
	FkVciCloseDev(DeviceHandle);

}


void CanTestSequeue1()
{
	HANDLE DeviceHandle;
	HANDLE CanChanlHandle;
	HANDLE CanChanlHandle1;
	printf("---------------------------\r\n");//测试非配对情况下 是否正常link
	/*Step1 Open Dev*/
	DeviceHandle = FkVciOpenDev(0, 1, 0);
	if (DeviceHandle == NULL)
	{
		printf("OpenDev Failed Test Faill!!!!!!\r\n");
		return;
	}
	printf("OpenDev Sucess !!!!!!\r\n");
	/*Step2 Open Chanl */
#if(TEST_CH1EN)
	CanChanlHandle = FkVciInitCAN(DeviceHandle, 0, 500000);
	if (CanChanlHandle == NULL)
	{
		printf("OpenCan1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle);
		return;
	}
	printf("OpenCan1 Sucess !!!!!!\r\n");
#endif

	FkVciCloseDev(DeviceHandle);


}




void CanTestSequeue2()
{
	HANDLE DeviceHandle;
	HANDLE CanChanlHandle;
	HANDLE CanChanlHandle1;
	printf("---------------------------\r\n");//测试非配对情况下 是否正常link
	/*Step1 Open Dev*/
	DeviceHandle = FkVciOpenDev(0, 0, 0);
	if (DeviceHandle == NULL)
	{
		printf("OpenDev Failed Test Faill!!!!!!\r\n");
	}
	CanChanlHandle = FkVciInitCANFD(DeviceHandle, 0, 500000,2000000);
	if (CanChanlHandle == NULL)
	{
		printf("OpenCan1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle);
	}
	CanChanlHandle1 = FkVciInitCANFD(DeviceHandle, 1, 500000, 2000000);
	if (CanChanlHandle1 == NULL)
	{
		printf("OpenCan1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle);
	}


	HANDLE DeviceHandle1;
	HANDLE CanChanlHandle2;
	HANDLE CanChanlHandle3;

	DeviceHandle1 = FkVciOpenDev(0, 1, 0);
	if (DeviceHandle1 == NULL)
	{
		printf("OpenDev Failed Test Faill!!!!!!\r\n");
	}
	CanChanlHandle2 = FkVciInitCANFD(DeviceHandle1, 0, 500000, 2000000);
	if (CanChanlHandle2 == NULL)
	{
		printf("OpenCan1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle1);
	}
	CanChanlHandle3 = FkVciInitCANFD(DeviceHandle1, 1, 500000, 2000000);
	if (CanChanlHandle3 == NULL)
	{
		printf("OpenCan1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle1);
	}

	HANDLE DeviceHandle2;
	HANDLE CanChanlHandle4;
	HANDLE CanChanlHandle5;

	DeviceHandle2 = FkVciOpenDev(0, 2, 0);
	if (DeviceHandle2 == NULL)
	{
		printf("OpenDev Failed Test Faill!!!!!!\r\n");
	}
	CanChanlHandle4 = FkVciInitCANFD(DeviceHandle2, 0, 500000, 2000000);
	if (CanChanlHandle4 == NULL)
	{
		printf("OpenCan1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle2);
	}
	CanChanlHandle5 = FkVciInitCANFD(DeviceHandle2, 1, 500000, 2000000);
	if (CanChanlHandle5 == NULL)
	{
		printf("OpenCan1 Failed Test Faill!!!!!!\r\n");
		FkVciCloseDev(DeviceHandle2);
	}

	int CntTotal = 0;

	int Dev1TxEn = 0;
	int Dev2TxEn = 0;
	int Dev3TxEn = 0;
	while (1)
	{
		int rxNum = FkVciGetReceiveNum(CanChanlHandle, 0);
		int rxNum1 = FkVciGetReceiveNum(CanChanlHandle1, 0);
		int rxNum2 = FkVciGetReceiveNum(CanChanlHandle2, 0);
		int rxNum3 = FkVciGetReceiveNum(CanChanlHandle3, 0);
		int rxNum4 = FkVciGetReceiveNum(CanChanlHandle4, 0);
		int rxNum5 = FkVciGetReceiveNum(CanChanlHandle5, 0);

		FkVciClearReceiveBuf(CanChanlHandle);
		FkVciClearReceiveBuf(CanChanlHandle1);
		FkVciClearReceiveBuf(CanChanlHandle2);
		FkVciClearReceiveBuf(CanChanlHandle3);
		FkVciClearReceiveBuf(CanChanlHandle4);
		FkVciClearReceiveBuf(CanChanlHandle5);

		CntTotal += (rxNum + rxNum1+ rxNum2+ rxNum3+ rxNum4+ rxNum5);
		printf("Total:%d  Chanl1RxNum:%d  Chanl2RxNum:%d  Chanl2RxNum:%d  Chanl3RxNum:%d  Chanl4RxNum:%d  Chanl5RxNum:%d\r\n", CntTotal, rxNum, rxNum1, rxNum2, rxNum3, rxNum4, rxNum5);
		for (int loop = 0; loop < 10; loop++)
		{
				if (1)
				{
					FkVciCanDataType canmsg;
					canmsg.CanID = 0x121;
					canmsg.DLC = 6;
					canmsg.FLAG = 0;
					canmsg.Data[0] = 1;
					canmsg.Data[1] = 2;
					canmsg.Data[2] = 3;
					canmsg.Data[3] = 4;
					canmsg.Data[4] = 5;
					canmsg.Data[5] = 6;
					canmsg.Data[6] = 7;
					canmsg.Data[7] = 8;
					FkVciTrsmitCAN(CanChanlHandle, &canmsg, 1);
					Dev1TxEn = 1;
				}
				if (rxNum1 != 0)
				{
					FkVciCanDataType canmsg;
					canmsg.CanID = 0x122;
					canmsg.DLC = 6;
					canmsg.FLAG = 0;
					canmsg.Data[0] = 1;
					canmsg.Data[1] = 2;
					canmsg.Data[2] = 3;
					canmsg.Data[3] = 4;
					canmsg.Data[4] = 5;
					canmsg.Data[5] = 6;
					canmsg.Data[6] = 7;
					canmsg.Data[7] = 8;
					FkVciTrsmitCAN(CanChanlHandle1, &canmsg, 1);
					Dev1TxEn = 1;
				}
				else
				{
					if (Dev1TxEn)
					{
						Dev1TxEn = 0;
					}
				}
		}
		for (int loop = 0; loop < 10; loop++)
		{
			if (1)
			{
				FkVciCanDataType canmsg;
				canmsg.CanID = 0x123;
				canmsg.DLC = 6;
				canmsg.FLAG = 0;
				canmsg.Data[0] = 1;
				canmsg.Data[1] = 2;
				canmsg.Data[2] = 3;
				canmsg.Data[3] = 4;
				canmsg.Data[4] = 5;
				canmsg.Data[5] = 6;
				canmsg.Data[6] = 7;
				canmsg.Data[7] = 8;
				FkVciTrsmitCAN(CanChanlHandle2, &canmsg, 1);
			}
			if (rxNum3 != 0)
			{
				FkVciCanDataType canmsg;
				canmsg.CanID = 0x124;
				canmsg.DLC = 6;
				canmsg.FLAG = 0;
				canmsg.Data[0] = 1;
				canmsg.Data[1] = 2;
				canmsg.Data[2] = 3;
				canmsg.Data[3] = 4;
				canmsg.Data[4] = 5;
				canmsg.Data[5] = 6;
				canmsg.Data[6] = 7;
				canmsg.Data[7] = 8;
				FkVciTrsmitCAN(CanChanlHandle3, &canmsg, 1);
				Dev2TxEn = 1;
			}
			else
			{
				if (Dev2TxEn)
				{
					Dev2TxEn = 0;
				}
			}
		}
		for (int loop = 0; loop < 10; loop++)
		{

		
			if (1)
			{
				FkVciCanDataType canmsg;
				canmsg.CanID = 0x125;
				canmsg.DLC = 6;
				canmsg.FLAG = 0;
				canmsg.Data[0] = 1;
				canmsg.Data[1] = 2;
				canmsg.Data[2] = 3;
				canmsg.Data[3] = 4;
				canmsg.Data[4] = 5;
				canmsg.Data[5] = 6;
				canmsg.Data[6] = 7;
				canmsg.Data[7] = 8;
				FkVciTrsmitCAN(CanChanlHandle4, &canmsg, 1);
			}
			if (rxNum5 != 0)
			{
				FkVciCanDataType canmsg;
				canmsg.CanID = 0x126;
				canmsg.DLC = 6;
				canmsg.FLAG = 0;
				canmsg.Data[0] = 1;
				canmsg.Data[1] = 2;
				canmsg.Data[2] = 3;
				canmsg.Data[3] = 4;
				canmsg.Data[4] = 5;
				canmsg.Data[5] = 6;
				canmsg.Data[6] = 7;
				canmsg.Data[7] = 8;
				FkVciTrsmitCAN(CanChanlHandle5, &canmsg, 1);
				Dev3TxEn = 1;
			}
			else
			{
				if (Dev3TxEn)
				{
					Dev3TxEn = 0;
				}
			}
		}
		Sleep(50);
	}



	FkVciCloseDev(DeviceHandle);


}