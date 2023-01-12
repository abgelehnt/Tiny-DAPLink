#include "CH552.H"

extern BOOL TO_IAP;

#define HEX_TO_ASCII(target,x) if((x)<10){(target)=(x)+'0';}else{(target)=(x)+'A'-10;}

UINT8 AT_Process(PUINT8 Uart_TxBuff);
