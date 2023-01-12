#include "AT.h"
#include "Uart.h"

#include <string.h>

BOOL TO_IAP;

UINT8 AT_Process(char * Uart_TxBuff){
	// if(!strncmp("AT",Uart_TxBuff,2)){
		// if(!strncmp("RST",&Uart_TxBuff[3],3)){
			// EA = 0;
			// SAFE_MOD = 0x55;
			// SAFE_MOD = 0xAA;
			// GLOBAL_CFG |= bSW_RESET;
		// }else if(!strncmp("IAP",&Uart_TxBuff[3],3)){
			// TO_IAP = 1;
		// }else if(!strncmp("CHIP_ID",&Uart_TxBuff[3],7)){
			// char str[8],i;
			
			// GetChipID(str);
			// for(i=0;i<8;i++){
				// Uart_RxBuff0[i] = str[i];
				// Uart_RxBuff1[i] = str[i];
			// }
			// UEP1_T_LEN = 8;
			// UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;//Ê¹ÄÜ·¢ËÍ
		// }
		// return 1;
	// }
	return 0;
}