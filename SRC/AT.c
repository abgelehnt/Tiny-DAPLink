/**
 * @author Chi Zhang
 * @date 2023/1/13
 */

#include "AT.h"

#include "Uart.h"
#include "TouchKey.h"
#include "DataFlash.H"
#include "Keyboard.h"

#include <string.h>

BOOL TO_IAP;

UINT8 AT_Process(char * Uart_TxBuff){
	if(!strncmp("AT",Uart_TxBuff,2)){
		if(!strncmp("RST",&Uart_TxBuff[3],3)){
			EA = 0;
			SAFE_MOD = 0x55;
			SAFE_MOD = 0xAA;
			GLOBAL_CFG |= bSW_RESET;
			
		}else if(!strncmp("IAP",&Uart_TxBuff[3],3)){
			TO_IAP = 1;
			
		}else if(!strncmp("CHIP_ID",&Uart_TxBuff[3],7)){
			char * str;
			if (!Uart_RxDealingWhich){
				str = Uart_RxBuff0;
			}else{
				str = Uart_RxBuff1;
			}

			HEX_TO_ASCII(str[0],*(UINT8C *)(0x3FFC)/16);
			HEX_TO_ASCII(str[1],*(UINT8C *)(0x3FFC)%16);
			HEX_TO_ASCII(str[2],*(UINT8C *)(0x3FFD)/16);
			HEX_TO_ASCII(str[3],*(UINT8C *)(0x3FFD)%16);
			
			HEX_TO_ASCII(str[4],*(UINT8C *)(0x3FFE)/16);
			HEX_TO_ASCII(str[5],*(UINT8C *)(0x3FFE)%16);
			HEX_TO_ASCII(str[6],*(UINT8C *)(0x3FFF)/16);
			HEX_TO_ASCII(str[7],*(UINT8C *)(0x3FFF)%16);

			UEP1_T_LEN = 8;
			UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;//Ê¹ÄÜ·¢ËÍ
			Uart_RxDealingWhich = ~Uart_RxDealingWhich;
		}else if(!strncmp("KEY=",&Uart_TxBuff[3],4)){
			UINT8 x;
			ASCII_TO_HEX(x,&Uart_TxBuff[7]);
			WriteDataFlash(0,&x,1);
			TargetKey = x;
		}
		return 1;
	}
	return 0;
}