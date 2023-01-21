/**
 * @author Chi Zhang
 * @date 2023/1/13
 */

#ifndef _AT_H
#define _AT_H

#include "CH552.H"

extern BOOL TO_IAP;

#define HEX_TO_ASCII(target,x) if((x)<10){(target)=(x)+'0';}else{(target)=(x)+'A'-10;}
#define ASCII_TO_HEX(target,x)	if((x)[0]>='0' && (x)[0] <= '9'){target = ((x)[0]-'0')*16;}	\
								else{target = ((x)[0]-'A'+10)*16;}							\
								if((x)[1]>='0' && (x)[1] <= '9'){target += (x)[1]-'0';}			\
								else{target += (x)[1]-'A'+10;}

UINT8 AT_Process(UINT8 xdata * Uart_TxBuff);

#endif //_AT_H
