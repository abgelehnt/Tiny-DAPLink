/**
 * @author Chi Zhang
 * @date 2023/1/13
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "CH552.H"

extern BOOL Keyboard_Flag;
extern UINT8I TargetKey;

void Keyboard_Press(void);	// 不能在中断中执行

#endif //_KEYBOARD_H
