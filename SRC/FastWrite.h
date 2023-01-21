/**
 * @author Chi Zhang
 * @date 2023/1/13
 */

#ifndef _FASTWRITE_H
#define _FASTWRITE_H

#include "CH552.H"

void XRAM_TO_DATA(UINT8 idata * target,UINT8 pointer);
void DATA_TO_XRAM(UINT8 pointer,UINT8 idata * target);

#endif //_FASTWRITE_H
