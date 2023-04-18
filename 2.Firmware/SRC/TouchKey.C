#include "TouchKey.H"
#include "Debug.H"
#include "DataFlash.H"
#include "AT.h"
#include "Keyboard.h"

UINT16 Key_FreeBuf;
UINT16 Key_DataBuf;
bit Press_Flag = 0;
bit TK_FlashFreeFlag = 0;

void TK_Init(void) {
	P1_DIR_PU &= ~0X02;
	P1_MOD_OC &= ~0X02;
	TKEY_CTRL = (TKEY_CTRL & 0XF8) | 0x02; // CH2
	TKEY_CTRL |= bTKC_2MS;

	ReadDataFlash(0x10, 2, (UINT8 *) &Key_FreeBuf);
	if (Key_FreeBuf == 0xFFFF) {
		TK_FlashKeyBuf();
	}
}

void TK_FlashKeyBuf(void) {
	UINT8 j;
	UINT16 sum;
	UINT16 OverTime;
	sum = 0;
	j = SAMPLE_TIMES;
	while (j--) {
		OverTime = 0;
		while ((TKEY_CTRL & bTKC_IF) == 0) {
			if (++OverTime == 0) {
				return;
			}
		}
		sum += TKEY_DAT;                                                    /*  */
	}
	Key_DataBuf = sum / SAMPLE_TIMES;
	Key_FreeBuf = Key_DataBuf;

	WriteDataFlash(0x10, (UINT8 *) &Key_FreeBuf, 2);
}

/*******************************************************************************
* Function Name  : ABS
* Description    : 求两个数差值的绝对值
* Input          : a,b
* Output         : None
* Return         : 差值绝对值
*******************************************************************************/
UINT16 ABS(UINT16 a, UINT16 b) {
	if (a > b) {
		return (a - b);
	} else {
		return (b - a);
	}
}

UINT8 TK_Measure(void) {
	UINT8 j;
	UINT16 err;
	UINT16 sum;
	UINT16 OverTime;

	sum = 0;
	j = SAMPLE_TIMES;
	while (j--) {
		OverTime = 0;
		while ((TKEY_CTRL & bTKC_IF) == 0) {
			if (++OverTime == 0) {
				return FAIL;
			}
		}
		sum += TKEY_DAT;                                                    /*  */
	}
	Key_DataBuf = sum / SAMPLE_TIMES;
	err = ABS(Key_FreeBuf, Key_DataBuf);
	if (err > DOWM_THRESHOLD_VALUE) {
		if (!Press_Flag) {
			Keyboard_Press();
		}
		Press_Flag = 1;
	} else if (err < UP_THRESHOLD_VALUE)                                //说明抬起或者未按下
	{
		if (Press_Flag)                                       //刚抬起
		{
			Press_Flag = 0;
		}
	}

	return SUCCESS;
}
