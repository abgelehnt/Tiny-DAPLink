#include "AT.h"

#include <string.h>

extern BOOL TO_IAP;

void AT_Process(PUINT8 Uart_TxBuff){
	if(!strncmp("AT",Uart_TxBuff,2)){
		if(!strncmp("RST",&Uart_TxBuff[3],3)){
			EA = 0;
			SAFE_MOD = 0x55;
			SAFE_MOD = 0xAA;
			GLOBAL_CFG |= bSW_RESET;
		}else if(!strncmp("IAP",&Uart_TxBuff[3],3)){
			TO_IAP = 1;
		}
	}
}