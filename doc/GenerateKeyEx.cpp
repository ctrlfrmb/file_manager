//////////////////////////////////////////////////////////////////////////
///Example for a SeedKey.Dll used in CANoe.DiVa
///See also CANoe.DiVa help==>Proceedings/Entry masks/"General" entry mask
//////////////////////////////////////////////////////////////////////////


#include <windows.h>
#include "GenerateKeyEx.h"


// KeyGeneration.cpp : Defines the entry point for the DLL application.
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}





KEYGENALGO_API VKeyGenResultEx GenerateKeyEx(
  const unsigned char* ipSeedArray,          /* Array for the seed [in] */
  unsigned int iSeedArraySize,               /* Length of the array for the seed [in] */
  const unsigned int iSecurityLevel,         /* Security level [in] */
  const char* ipVariant,                     /* Name of the active variant [in] */  
  unsigned char* iopKeyArray,                /* Array for the key [in, out] */
  unsigned int iMaxKeyArraySize,             /* Maximum length of the array for the key [in] */
  unsigned int& oActualKeyArraySize)         /* Length of the key [out] */
{
  //Copy seed from parameter to a integer 
  //Note: The byte order in the seed array is equal to the byte order in the bus message
  unsigned long seed=0;
  unsigned long seed1=0;
  seed=ipSeedArray[3];
  seed=seed | (ipSeedArray[2]<<8);
  seed=seed | (ipSeedArray[1]<<16);
  seed=seed | (ipSeedArray[0]<<24);

  unsigned long Key1=0; 
  unsigned long Key2=0;
  unsigned long Key=0;

  //begin calculate key from seed------------------------------------------------------------
  //for security access with Services 0x27 01 ->0x27 02
  if (iSecurityLevel==0x01)
  {
    if(seed!=0)
	{
		//obtain Key1[4]
	
		Key1=seed^0x74F71517;

		//obtain seed1
		seed1=(((seed & 0xaaaaaaaa) >> 1) | ((seed & 0x55555555) << 1));
		seed1 = (((seed1 & 0xcccccccc) >> 2) | ((seed1 & 0x33333333) << 2));
		seed1 = (((seed1 & 0xf0f0f0f0) >> 4) | ((seed1 & 0x0f0f0f0f) << 4));
		seed1 = (((seed1 & 0xff00ff00) >> 8) | ((seed1 & 0x00ff00ff) << 8));
		seed1 =((seed1 >> 16) | (seed1 << 16));

		//obtain Key2[4]
	
		Key2=seed1^0x74F71517;

		//add Key1 and Key2
	
		Key=(Key1)+(Key2);
	}
  }  

 //for security access with Services 0x27 11 -> 0x27 12
  else if (iSecurityLevel==0x11) 
  {  
    if(seed!=0)
	{
		//obtain Key1[4]
	
		Key1=seed^0x74F7837F;

		//obtain seed1
		seed1=(((seed & 0xaaaaaaaa) >> 1) | ((seed & 0x55555555) << 1));
		seed1 = (((seed1 & 0xcccccccc) >> 2) | ((seed1 & 0x33333333) << 2));
		seed1 = (((seed1 & 0xf0f0f0f0) >> 4) | ((seed1 & 0x0f0f0f0f) << 4));
		seed1 = (((seed1 & 0xff00ff00) >> 8) | ((seed1 & 0x00ff00ff) << 8));
		seed1 =((seed1 >> 16) | (seed1 << 16));

		//obtain Key2[4]
	
		Key2=seed1^0x74F7837F;
      /*  Key2 = ((((seed1 >> 24)& 0xff) & 0x74)
			    | (((seed1 >> 16)& 0xff) ^ 0xf7)
				| (((seed1 >> 8)& 0xff ) & 0x83)
				| ((seed1 & 0xff) ^ 0x7f));*/


		//add Key1 and Key2
	
		Key=(Key1)+(Key2);
	}   
  }  

 /*//for security access with Services 0x27 05 -> 0x27 06
  if (iSecurityLevel==0x05) 
  {  
    key=seed*3;   
  }  */
  //end calculate key from seed------------------------------------------------------------

  //Copy key to the output buffer 
  //Note: The first byte of the key array will be the first key byte of the bus message
  iopKeyArray[3] = Key & 0xff;
  iopKeyArray[2] = (Key>>8)& 0xff;
  iopKeyArray[1] = (Key>>16)& 0xff;
  iopKeyArray[0] = (Key>>24)& 0xff;
  //setting length of key
  oActualKeyArraySize = 4;     
  return KGRE_Ok;
}