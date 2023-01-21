#ifndef __UART_H__
#define __UART_H__

#define DEFAULT_UART_BUAD 115200

extern UINT8X Uart_TxBuff0[];
extern UINT8X Uart_TxBuff1[];
extern UINT8X Uart_RxBuff0[];
extern UINT8X Uart_RxBuff1[];

extern BOOL Uart_RxDealingWhich;

void UART_Setup(void);
void Config_Uart0(UINT8 *cfg_uart);

void USB_CDC_PushData(void);
void USB_CDC_GetData(void);

#endif
