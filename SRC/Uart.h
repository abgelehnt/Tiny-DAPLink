#ifndef __UART_H__
#define __UART_H__

#define USE_UART0 1
#define USE_UART1 0

#ifndef UART_BUAD
	#define UART_BUAD 115200
#endif


void UART_Setup(void);

extern UINT8X Uart_TxBuff0[];

void USB_CDC_PushData(void);
void USB_CDC_GetData(void);

#endif
