#ifndef    _TOUCH_KEY_H
#define    _TOUCH_KEY_H

#include "CH552.H"

#define SAMPLE_TIMES (0x05)

extern UINT16 Key_FreeBuf;
extern UINT16 Key_DataBuf;
extern bit Press_Flag;

extern bit TK_FlashFreeFlag;

void TK_Init(void);

UINT8 TK_Measure(void);

void TK_FlashKeyBuf(void);



#endif
