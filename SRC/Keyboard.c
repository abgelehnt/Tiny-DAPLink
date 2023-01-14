/**
 * @author Chi Zhang
 * @date 2023/1/13
 */

#include "Keyboard.h"

#include "string.h"
#include "Uart.h"
#include "AT.h"

extern UINT8X Ep4Buffer[];

BOOL Keyboard_Flag = 0;
UINT8I TargetKey;

void Keyboard_Press(void){
	if(TargetKey == 0xFF)
		return;
	Keyboard_Flag = 0;
	Ep4Buffer[2] = TargetKey;
	UEP4_T_LEN = 8;                                             //上传数据长度
	UEP4_CTRL = UEP4_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;                //有数据时上传数据并应答ACK

	while(Keyboard_Flag == 0)
		;    /*等待上一包传输完成*/
	Ep4Buffer[2] = 0;
	UEP4_T_LEN = 8;                                             //上传数据长度
	UEP4_CTRL = UEP4_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;                //有数据时上传数据并应答ACK
}
