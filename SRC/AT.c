#include "AT.h"
#include "Uart.h"

#include <string.h>


BOOL TO_IAP;

//UINT8C Chip_ID[] _at_ 0x3FFE;

void GetChipID(UINT8 * str){
	UINT8 i,temp;
	UINT32 id = *(UINT8C *)(0x3FFF);
	id += *(UINT8C *)(0x3FFE) * 256l;
	id += *(UINT8C *)(0x3FFD) * 256l * 256l;
	id += *(UINT8C *)(0x3FFC) * 256l * 256l * 256l;
	for(i=0;i<8;i++){
		str[i] = 48;
	}
	for(i=0;i<8;i++){
		temp = id % 16;
		if (temp < 10){
			temp += 48;
		}else{
			temp += 55;
		}
		str[i] = temp;
		id /= 16;
	}
}


UINT8 AT_Process(PUINT8 Uart_TxBuff){
	if(!strncmp("AT",Uart_TxBuff,2)){
		if(!strncmp("RST",&Uart_TxBuff[3],3)){
			EA = 0;
			SAFE_MOD = 0x55;
			SAFE_MOD = 0xAA;
			GLOBAL_CFG |= bSW_RESET;
		}else if(!strncmp("IAP",&Uart_TxBuff[3],3)){
			TO_IAP = 1;
		}else if(!strncmp("CHIP_ID",&Uart_TxBuff[3],7)){
			char str[8],i;
			
			GetChipID(str);
			for(i=0;i<8;i++){
				Uart_RxBuff0[i] = str[i];
				Uart_RxBuff1[i] = str[i];
			}
			UEP1_T_LEN = 8;
			UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;//Ê¹ÄÜ·¢ËÍ
		}
		return 1;
	}
	return 0;
}