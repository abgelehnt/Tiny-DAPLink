#ifndef    _TOUCH_KEY_H
#define    _TOUCH_KEY_H

#include "CH552.H"

#define SAMPLE_TIMES (0x05)
#define DOWM_THRESHOLD_VALUE          0x400                      //按下阈值定义
#define UP_THRESHOLD_VALUE            0x200                        //抬起阈值

extern UINT16 Key_FreeBuf;
extern UINT16 Key_DataBuf;
extern bit Press_Flag;

extern bit TK_FlashFreeFlag;

void TK_Init(void);

UINT8 TK_Measure(void);

void TK_FlashKeyBuf(void);



#endif
