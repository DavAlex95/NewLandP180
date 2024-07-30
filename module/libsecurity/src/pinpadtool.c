#include "lsecurity.h"
#include "ltool.h"
#include "lui.h"
#include "pinpad.h"
#include "pinpadtool.h"

static char gszBuf[2048];
	
const char PinPad_szCode[][80]={
	{"-1209Get key err"},
	{"-3001Unknown Pinpad"},    
	{"-32010x55 Oper err"},    
	{"-3202Press ESC to exit"},
	{"-3203Aux select fail"},      
	{"-3204Invalid merchant"},    
	{"-3205RF send fail"},      
	{"-3206RF receive fail"},    
	{"-3207Not returned data"},    
	{"-3208receive command fail"},      
	{"-3209Main key wrong"},    
	{"-3210User key wrong"},    
	{"-3211Init aux fail"},
	{"-3212Input parameter err"},    
	{"-3213Timeout"},    
	{"-3214Send command fail"},
	{"-3215Device not supported"},
	{"-3216Device unable to connect"},
	{"-3217Mode not supported"},
	{"-3218Pin encryption not supported"},
	{"-3219Pin decryption not supported"},
	{"-3220Init Pinpad fail"},

	{"-3301command wrong"},    
	{"-3302Timeout"},
	{"-3303Main key not exist"},      
	{"-3304Instruction key err"},    
	{"-3305Battery err"},      
	{"-3306Battery command err"},    
	{"-3307Illegal instruction"},    
	{"-3308CPU err"},      
	{"-3309SRAM err"},    
	{"-3310Pinpad err"},    
	{"-3311Command err"},
	{"-3312CPU card err"},    
	{"-3313Abnormal battery"},    
	{"-3314Main key wrong"},
	{"-3315Other err"},

	{"-3320LRC wrong"},
	{"-3321Data err"},    
	{"-3322Read err"},
	{"-3323Message header err"},      
	{"-3324Calculate LRC err"},    
	{"-3325Key length err"},      
	{"-3326Key number err"},    
	{"-3327Parameter err"},    
	{"-3328Pin length err"},      
	{"-3329Encryption type err"},    
	{"-3330Input-mode err"},

	{"-3350Other err"},

	{"-3401Battery err"},    
	{"-3402NO BAR main key"},
	{"-3403NO main key"},      
	{"-3404Key length err"},    
	{"-3405Key index err"},      
	{"-3406TAG or serial No. err"},    
	{"-3407ODD check err"},    
	{"-3408Check value err"},      
	{"-3409Command length err"},    
	{"-3410Data wrong"},    
	{"-3411No code information"},
	{"-3412Parameter err"},    
	{"-3413Auth code err"},    
	{"-3414Lock err"},
	{"-3415Timeout"},
	{"-3416Other err"},
	{"-3417Data wrong"},
	{"-3418Auth code err"},

	{"-3501Select directory fail"},    
	{"-3502Set UID failed"},
	{"-3503Set root directory auth key fail"},      
	{"-3504Set the sub directory auth key fail"},    
	{"-3505Set pwd fail"},      
	{"-3506Master key, Work key download fail"},    
	{"-3507Check code err"},    
	{"-3508Authentication fail"},      
	{"-3509Issue pinpad fail"},    
	{"-3510Setup and diliver Work key/Auth key fail"},    
	{"-3511Setup PIN/MAC encryption auth key fail"},
	{"-3512Set KVC fail"},    
	{"-3513Send MAC data fail"},   

	{"-3601Clear key area fail"},
	{"-3602Setup key fail"},
	{"-3603Get screen parameter fail"},
	{"-3604Input Pin fail"},
	{"-3605Get password results fail"},
	{"-3606Get MAC value fail"},
	{"-3607DES oper fail"},
	{"-3608Soft DES oper fail"},
	{"-3609No key installed"},
	{""},
};


void ProTraceSecurity(char* lpszFormat, ...)
{
	va_list args;

	va_start(args, lpszFormat);
	vsprintf(gszBuf, lpszFormat, args);
	va_end(args);
}

void ProGetTraceInfo(char* buf)
{
	strcpy(buf, gszBuf);
}

int ProSetSecurityErrCode(int nErr,int nNapiErr)
{
	int i;
	char buf[5+1]={0};
	char Tempbuf[80]={0};
	
	sprintf(buf,"%d",nErr);

	for(i=0;;i++)
	{
		if(strlen(PinPad_szCode[i])==0)
		{
			return APP_FAIL;
		}
		if((memcmp(buf,PinPad_szCode[i],5)==0))
		{
			strcpy(Tempbuf,PinPad_szCode[i]+5);
			PubSetErrorCode(nErr,Tempbuf,nNapiErr);	
			return APP_SUCC;
		}		
	}	
	return APP_FAIL;
}


