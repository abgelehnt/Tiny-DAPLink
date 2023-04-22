/* ÄÚ´æ±í
 * 0x00: Keyboard_Key
 * 0x10: KeyboardH
 * 0x11: KeyboardL
 * 0x12: DOWM_THRESHOLD_VALUEH
 * 0x13: DOWM_THRESHOLD_VALUEL
 * 0x14: UP_THRESHOLD_VALUEH
 * 0x15: UP_THRESHOLD_VALUEL
*/
#ifndef	DATAFLASH_H
#define	DATAFLASH_H

/*******************************************************************************
* Function Name  : WriteDataFlash(UINT16 Addr,PUINT8 buf,UINT8 len)
* Description    : DataFlashÐ´
* Input          : UINT16 Addr£¬PUINT16 buf,UINT8 len 
* Output         : None 
* Return         : 
*******************************************************************************/
UINT8 WriteDataFlash(UINT8 Addr,PUINT8 buf,UINT8 len);

/*******************************************************************************
* Function Name  : ReadDataFlash(UINT8 Addr,UINT8 len,PUINT8 buf)
* Description    : ¶ÁDataFlash
* Input          : UINT16 Addr PUINT8 buf
* Output         : None
* Return         : UINT8 l
*******************************************************************************/
UINT8 ReadDataFlash(UINT8 Addr,UINT8 len,PUINT8 buf);


#endif