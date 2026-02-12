#include "pch.h"
#include "MSE80.h"
#include "FkToLQVciDll.h"
#include <fstream>
#include"UdsLoad.h"
using namespace std;
#include"stdio.h"



MSE80::MSE80()
{
	EcuType = UAES_MSE80;
}
MSE80 ::~MSE80()
{
	EcuType = 0;
}

int MSE80::FlashDown_Init(void* param)
{
	if (param == NULL)
		return 1;
	isOpenFlag = 1;
	Mse80InitDataType* dPtr = (Mse80InitDataType*)param;
	strcpy_s(RealseDataPath, dPtr->RealseFilePath);
	return 0;

}

int MSE80::FlashDown_SecurityAccess(uint32_t CanTxId, uint8_t secLevel, uint8_t seedLength, uint8_t keyLength)
{
	uint8_t txbuf[16];
	uint8_t rxbuf[16];
	uint16_t rxlen;
	/*请求密钥*/
	txbuf[0] = 0x27;
	txbuf[1] = (secLevel << 1) - 1;
	NowSidNum = txbuf[0];
	int UdsRet = FkVciUdsRequest(IsoChanlHandle, CanTxId, txbuf, 2, rxbuf, &rxlen);
	if (UdsRet != 0 || rxbuf[0] != 0x67)
	{
		if (UdsRet != 0)
		{
			sprintf_s(ErrInfo, "Request SID 0x27 Negative Responsed ,NegativeCode:%X", UdsRet);
			return UdsRet;
		}
		sprintf_s(ErrInfo, "Request SID 0x27 Unkonw RSID Responsed ,RSID:%X", rxbuf[0]);
		return rxbuf[0];
	}
	/*Use You Self Sec */
	HINSTANCE hDll;//句柄
	//TCHAR chCurDir[MAX_PATH] = { 0 };
	//GetCurrentDirectory(MAX_PATH, chCurDir);
	hDll = LoadLibraryEx("MSE80Security.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);//动态加载DLL模块句柄
	//hDll = LoadLibraryEx("SecurityAccess_M6_V1.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (hDll)
	{
		typedef uint32_t(*UAES_Calc_KeyFun)(uint32_t seed);
		UAES_Calc_KeyFun SecAcc_CalcKey = NULL;//函数指针
		SecAcc_CalcKey = (UAES_Calc_KeyFun)GetProcAddress(hDll, "UAES_Calc_Key");//得到所加载DLL模块中函数的地址

		if (SecAcc_CalcKey)
		{
			uint32_t Seed = rxbuf[2] << 24 | rxbuf[3] << 16 | rxbuf[4] << 8 | rxbuf[5];
			uint32_t Key;
			Key = SecAcc_CalcKey(Seed);
			txbuf[0] = 0x27;
			txbuf[1] = (secLevel << 1);
			txbuf[2] = (uint8_t)((Key >> 24) & 0xff);
			txbuf[3] = (uint8_t)((Key >> 16) & 0xff);
			txbuf[4] = (uint8_t)((Key >> 8) & 0xff);
			txbuf[5] = (uint8_t)((Key) & 0xff);
		}
		else
		{
			//printf("GetProcAddress Fail !!!!!!\r\n");
			sprintf_s(ErrInfo, "GetProcAddress Fail");
			FreeLibrary(hDll);//释放已经加载的DLL模块
			return 0xF7;
		}
		FreeLibrary(hDll);//释放已经加载的DLL模块
	}
	else
	{
		sprintf_s(ErrInfo, "Load Dll Fail");
		return 0xF8;
	}

	/*验证密钥*/
	txbuf[0] = 0x27;
	txbuf[1] = secLevel << 1;
	UdsRet = FkVciUdsRequest(IsoChanlHandle, CanTxId, txbuf, 6, rxbuf, &rxlen);
	if (UdsRet != 0 || rxbuf[0] != 0x67)
	{
		if (UdsRet != 0)
		{
			sprintf_s(ErrInfo, " Check SID 0x27 Negative Responsed ,NegativeCode:%X", UdsRet);
			return UdsRet;
		}
		sprintf_s(ErrInfo, " Check SID 0x27 Unkonw RSID Responsed ,RSID:%X", rxbuf[0]);
		return rxbuf[0];
	}
	return ERRCODE_NOERR;
}


int MSE80::FlashDown_EraseFlash(uint32_t CanTxId, uint16_t Rid, uint32_t StartAdr, uint32_t Lenth)
{
	uint8_t UdsTxBuf[64];
	uint8_t UdsRxBuf[64];
	uint16_t UdsRxLen;
	UdsTxBuf[0] = 0x31;
	UdsTxBuf[1] = 0x01;
	UdsTxBuf[2] = (uint8_t)(Rid >> 8 & 0xff);
	UdsTxBuf[3] = (uint8_t)(Rid >> 0 & 0xff);
	UdsTxBuf[4] = 0x44;

	UdsTxBuf[5] = (uint8_t)(StartAdr >> 24 & 0xff);
	UdsTxBuf[6] = (uint8_t)(StartAdr >> 16 & 0xff);
	UdsTxBuf[7] = (uint8_t)(StartAdr >> 8 & 0xff);
	UdsTxBuf[8] = (uint8_t)(StartAdr >> 0 & 0xff);

	UdsTxBuf[9] = (uint8_t)(Lenth >> 24 & 0xff);
	UdsTxBuf[10] = (uint8_t)(Lenth >> 16 & 0xff);
	UdsTxBuf[11] = (uint8_t)(Lenth >> 8 & 0xff);
	UdsTxBuf[12] = (uint8_t)(Lenth >> 0 & 0xff);
	NowSidNum = UdsTxBuf[0];
	uint8_t UdsRet = FkVciUdsRequest(IsoChanlHandle, CanTxId, UdsTxBuf, 13, UdsRxBuf, &UdsRxLen);
	if (UdsRet != 0 || UdsRxBuf[0] != 0x71)
	{
		if (UdsRet != 0)
		{
			sprintf_s(ErrInfo, " SID 0x31 Negative Responsed ,NegativeCode:%X", UdsRet);
			return UdsRet;
		}
		sprintf_s(ErrInfo, " SID 0x31 Unkonw RSID Responsed ,RSID:%X", UdsRxBuf[0]);
		return UdsRxBuf[0];
	}
	return ERRCODE_NOERR;
}

int MSE80::FlashDownOnce(char* PathStr)
{

	if (isOpenFlag == 0)
		return 1;
	int SequeueRet = 0;
	//Step1
	sprintf_s(Info, "Request Enter ExternMode ");
	if (FlashDown_SessionControl(0x7E0, 0x03) != 0)
	{
		sprintf_s(Info, "Enter ExternMode Fail");
	}
	sprintf_s(Info, "Enter ExternMode Success");


	sprintf_s(Info, "ECU Reset To Boot ");
	SequeueRet = FlashDown_ResetControl(RequestID, 0x60);
	if (SequeueRet != 0)
	{
		sprintf_s(Info, "ResetControl Fail");
		return SequeueRet;
	}
	sprintf_s(Info, "ResetControl Sucess");
	Sleep(500);

	//Step5
	sprintf_s(Info, "Request Enter ProgramMode ");
	SequeueRet = FlashDown_SessionControl(RequestID, 0x02);
	if (SequeueRet != 0)
	{
		sprintf_s(Info, "Enter ProgramMode Fail");
		return SequeueRet;
	}
	sprintf_s(Info, "Enter ProgramMode Success");

	//Step6
	sprintf_s(Info, "Request SecurityAccess ");
	SequeueRet = FlashDown_SecurityAccess(RequestID, 0x04, 4, 4);
	if (SequeueRet != 0)
	{
		sprintf_s(Info, "SecurityAccess Fail Ret:%d", SequeueRet);
		return SequeueRet;
	}
	sprintf_s(Info, "SecurityAccess Success");

	/******************************************Program Data**************************/
	sprintf_s(Info, "Erase %s ", "0x090C0000L Len0x000C0000L");
	SequeueRet = FlashDown_EraseFlash(RequestID, 0xFF00, 0x090C0000L, 0x000C0000L);
	if (SequeueRet != 0)
	{
		sprintf_s(Info, "Erase %s Fail", "0x090C0000L Len0x000C0000L");
		return SequeueRet;
	}
	//Step 11 
	sprintf_s(Info, "DownLoad %s Flash ", "0x090C0000L Len0x000C0000L");
	SequeueRet = FlashDown_ProgramMemory(RequestID, 0x090C0000L, 0x40000L, 0x000C0000L, PathStr);
	if (SequeueRet != 0)
	{
		sprintf_s(Info, "DownLoad %s Fail", "0x090C0000L Len0x000C0000L");
		return SequeueRet;
	}
	sprintf_s(Info, "DownLoad %s Sucess", "0x090C0000L Len0x000C0000L");

	/********************************************************************/
	sprintf_s(Info, "Erase %s ", "0x09080000L Len0x00040000L");
	SequeueRet = FlashDown_EraseFlash(RequestID, 0xFF00, 0x09080000L, 0x00040000L);
	if (SequeueRet != 0)
	{
		sprintf_s(Info, "Erase %s Fail", "0x09080000L Len0x00040000L");
		return SequeueRet;
	}
	//Step 11 
	sprintf_s(Info, "DownLoad %s Flash ", "0x09080000L Len0x00040000L");
	SequeueRet = FlashDown_ProgramMemory(RequestID, 0x09080000L, 0x00L, 0x00040000L, PathStr);
	if (SequeueRet != 0)
	{
		sprintf_s(Info, "DownLoad %s Fail", "0x09080000L Len0x00040000L");
		return SequeueRet;
	}
	sprintf_s(Info, "DownLoad %s Sucess", "0x09080000L Len0x00040000L");

	//Step 17
	sprintf_s(Info, "ECU Reset ");
	SequeueRet = FlashDown_ResetControl(RequestID, 0x01);
	if (SequeueRet != 0)
	{
		sprintf_s(Info, "ResetControl Fail");
		return SequeueRet;
	}
	sprintf_s(Info, "ResetControl Sucess");
	return 0;
}


int MSE80::FlashDown_Sequeue()
{

	TrsmitCompleteSize = 0;
	TrsmitTotolSize = 0x100000;

	int RelaseFlashRet = FlashDownOnce(RealseDataPath);
	return RelaseFlashRet;
}


int MSE80::DiagRequest(uint32_t DiagId, uint32_t EcuId, uint8_t* pDiagReqData, uint16_t DiagReqDataLen, uint8_t* pDiagRspData, uint16_t* pDiagRspDataLen)
{
	int Ret;
	if (DiagId == 0x630)
	{
		Ret = FkVciSetFilterISO(IsoChanlHandle, 0x630, 0x730);
		Sleep(50);
	}
	else
	{
		Ret = FkVciSetFilterISO(IsoChanlHandle, 0x7e0, 0x7e8);
		Sleep(50);
	}
	
	int DiagRet = FkVciUdsRequest(IsoChanlHandle, DiagId, pDiagReqData, DiagReqDataLen, pDiagRspData, pDiagRspDataLen);
	Ret = FkVciSetFilterISO(IsoChanlHandle, RequestID, ResponseID);
	return DiagRet;
}
