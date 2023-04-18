/**
 * @author Chi Zhang
 * @date 2023/1/13
 */

#include "AT.h"

#include "Uart.h"
#include "TouchKey.H"
#include "DataFlash.H"
#include "Keyboard.h"

#include <string.h>

#pragma RB(1)

BOOL TO_IAP;

void CDC_Print(char * targetString){
	char * str;
	if (!Uart_RxDealingWhich){
		str = Uart_RxBuff0;
	}else{
		str = Uart_RxBuff1;
	}

	strcpy(str,targetString);

	UEP1_T_LEN = strlen(targetString);
	UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;
	Uart_RxDealingWhich = ~Uart_RxDealingWhich;
}

UINT8 AT_Process(char xdata * Uart_TxBuff){
	if(!strncmp("DAT+",Uart_TxBuff,4)){
		Uart_TxBuff += 4;
		if(!strncmp("RST",&Uart_TxBuff[0],3)){
			EA = 0;
			SAFE_MOD = 0x55;
			SAFE_MOD = 0xAA;
			GLOBAL_CFG |= bSW_RESET;
			
		}else if(!strncmp("IAP",&Uart_TxBuff[0],3)){
			TO_IAP = 1;
			
		}else if(!strncmp("CHIP_ID",&Uart_TxBuff[0],7)){
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
			UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;
			Uart_RxDealingWhich = ~Uart_RxDealingWhich;
		}else if(!strncmp("KEY=",&Uart_TxBuff[0],4)){
			UINT8 x;
			ASCII_TO_HEX(x,&Uart_TxBuff[4]);
			WriteDataFlash(0,&x,1);
			TargetKey = x;
		}else if(!strncmp("KEY?",&Uart_TxBuff[0],4)){
			UINT8 str[5] = {0};
			HEX_TO_ASCII(str[0],TargetKey/16);
			HEX_TO_ASCII(str[1],TargetKey%16);
			str[2] = '\n';
			str[3] = '\r';
			CDC_Print(str);
		}else if(!strncmp("AUTHOR",&Uart_TxBuff[0],6)){
			CDC_Print("GitHub:abgelehnt/Tiny-DAPLink\n\r");
		}else if(!strncmp("KEY_VALUE",&Uart_TxBuff[0],9)){
			UINT8 str[13] = {0};
			HEX_TO_ASCII(str[0],Key_FreeBuf/16/16/16);
			HEX_TO_ASCII(str[1],Key_FreeBuf/16/16%16);
			HEX_TO_ASCII(str[2],Key_FreeBuf/16%16);
			HEX_TO_ASCII(str[3],Key_FreeBuf%16);
			str[4] = ':';
			HEX_TO_ASCII(str[5],Key_DataBuf/16/16/16);
			HEX_TO_ASCII(str[6],Key_DataBuf/16/16%16);
			HEX_TO_ASCII(str[7],Key_DataBuf/16%16);
			HEX_TO_ASCII(str[8],Key_DataBuf%16);
			str[9] = ':';
			HEX_TO_ASCII(str[10],0);
			str[11] = '\n';
			str[12] = '\0';
			CDC_Print(str);
		}else if(!strncmp("KEY_FLASH",&Uart_TxBuff[0],9)){
			TK_FlashFreeFlag = 1;
		}
		return 1;
	}
	return 0;
}