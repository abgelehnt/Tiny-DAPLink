/**
 * @author Chi Zhang
 * @date 2023/2/7
 */

#ifndef _USB_H
#define _USB_H

#include "CH552.H"
#include "DAP.h"

#define Fullspeed 1
#define THIS_ENDP0_SIZE 64
#define ENDP4_IN_SIZE 8

sbit P34 = P3 ^ 4;
#define DTR P34

extern UINT8I Ep3Ii, Ep3Io;            //IN 索引
extern UINT8I Ep3Is[DAP_PACKET_COUNT]; //发送包长

extern PUINT8 pDescr; //USB配置标志
extern BOOL Endp3Busy;

extern UINT8X Ep4Buffer[]; //端点4 IN双缓冲区,必须是偶地址

extern UINT8I SetupReq, SetupLen, Count, UsbConfig;

void USBDeviceInit();

#endif //_USB_H
