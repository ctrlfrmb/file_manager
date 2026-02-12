#include"stdio.h"
#include "FkVciDll.h"
#pragma comment(lib,"FkVciDll.lib")


extern void AcanTestSequeue();
extern void CanTestSequeue();
extern void LinTestSequeue();
extern void LinMixTestSequeue();
extern void LinSlaveTestSequeue();
extern void CanTestSequeue1();
extern void CanTestSequeue2();
extern void SentTestSequeue2();
extern void LinBcsTestSequeue();
int main()
{
	//int TestMode;
	//while (1)
	//{
	//	printf("Please Enter Test Mode : 0:CAN 1:LIN Other: Exit\r\n");
	//	scanf_s("%d",&TestMode);

	//	if (TestMode == 0)
	//	{
	//		CanTestSequeue();
	//	}
	//	else if (TestMode == 1)
	//	{
	//		LinMixTestSequeue();
	//	}
	//	if (TestMode == 2)
	//	{
	//		CanTestSequeue2();
	//	}
	//	else
	//	{
	//		break;
	//	}

	//}

	//for (int zz = 0; zz < 2; zz++)

	//{
	//		LinMixTestSequeue();
	//		Sleep(10);
	//}
	LinBcsTestSequeue();
	
	return 0;
}