/********************************** (C) COPYRIGHT *******************************
* File Name          : TouchKey.C
* Author             : WCH
* Version            : V1.2
* Date               : 2019/07/22
* Description        : Touchkey driver c file.
*******************************************************************************/
#include "CH552.H"                                                          
#include "Debug.H"
#include "stdio.H"
#include "TouchKey.H"
#include "Uart.h"
#include "AT.h"
#include "Keyboard.h"
/*******************************************************************************
Input channel as below:

 bTKC_CHAN2 |bTKC_CHAN1	|bTKC_CHAN0	|	CHANNEL				| PIN		
 -----------+-----------+-----------+-----------------------+------------
 	0		|	0		|	0		|	no channel			| ****		
 	0		|	0		|	1		|	TIN0				| P1.0		
 	0		|	1		|	0		|	TIN1				| P1.1		
 	0		|	1		|	1		|	TIN2				| P1.4		
 	1		|	0		|	0		|	TIN3				| P1.5		
 	1		|	0		|	1		|	TIN4				| P1.6		
 	1		|	1		|	0		|	TIN5				| P1.7		
	1		|	1		|	1		|	Enable touch core	| no channel
	
*******************************************************************************/
	
UINT8 	TK_Code[TOUCH_NUM] = {								/* Arrange the input channel */			
	0x03, 															  /* CH2 */
};		

UINT16 	Key_FreeBuf[TOUCH_NUM];
UINT16  Key_DataBuf[TOUCH_NUM];
UINT8V 	Touch_IN = 0;												  /* BIT6 & BIT7 reserved, other bit means touch state */
volatile UINT8 Press_Flag = 0; 
/*******************************************************************************
* Function Name  : TK_SelectChannel
* Description    : Select TK input channel.
* Input          : ch : input channel.
* Return         : SUCCESS/FAIL.
*******************************************************************************/
UINT8 TK_SelectChannel( UINT8 ch )
{
	if ( ch <= TOUCH_NUM )
	{
		TKEY_CTRL = ( TKEY_CTRL & 0XF8) | TK_Code[ch];
		return SUCCESS;
	}

	return	FAIL;
}

/*******************************************************************************
* Function Name  : TK_Init
* Description    : Init input channel. Float input, if it used for touchkey
* Input          : channel   : IO bits, configure the Hi-z mode for touch detection.
				   queryFreq : Scan frequence. 0: 1ms , 1: 2ms.
				   ie		 : Enable interrupt, 0:disable ; i: enable.
* Return         : Return FAIL, if channel's error.
*******************************************************************************/
UINT8 TK_Init( UINT8 channel , UINT8 queryFreq, UINT8 ie )
{

	UINT8 	i,j;
	UINT16 	sum;
	UINT16 	OverTime;
	if ( ( channel & (BIT2+BIT3) ) != 0 )							/* not include BIT2 & BIT3 */
	{
		return FAIL;
	}
	
	P1_DIR_PU &= ~channel;
	P1_MOD_OC &= ~channel;
	
	if( queryFreq != 0 ) 
	{
		TKEY_CTRL |= bTKC_2MS ;
	}
	
	/* Get Key_FreeBuf. Save the data in flash or macro define. */
	/* DO NOT get Key_FreeBuf in Mass Production. */
	for ( i = 0; i < TOUCH_NUM; i++ )
	{
		
		sum = 0;
		j = SAMPLE_TIMES;
		TK_SelectChannel( i );
//		TKEY_CTRL |= TK_Code[i];
		while( j-- )
		{
			OverTime = 0;
			while( ( TKEY_CTRL & bTKC_IF ) == 0 )
			{
				if( ++OverTime == 0 )
				{
					return FAIL;
				}
			}
			sum += TKEY_DAT;													/*  */
		}
		Key_FreeBuf[i] = sum / SAMPLE_TIMES;
		
//		if (!Uart_RxDealingWhich){
//			str = Uart_RxBuff0;
//		}else{
//			str = Uart_RxBuff1;
//		}

//		str[0] = 'F';
//		str[1] = 'B';
//		HEX_TO_ASCII(str[2],i);
//		str[3] = ':';
//		HEX_TO_ASCII(str[4],Key_FreeBuf[i]/16/16/16);
//		HEX_TO_ASCII(str[5],Key_FreeBuf[i]/16/16%16);
//		HEX_TO_ASCII(str[6],Key_FreeBuf[i]/16%16);
//		HEX_TO_ASCII(str[7],Key_FreeBuf[i]%16);
//		str[8] = '\n';

//		UEP1_T_LEN = 9;
//		UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;//使能发送
//		Uart_RxDealingWhich = ~Uart_RxDealingWhich;
	}
	if( ie != 0 )																/* Enable interrupt ?  */		
	{
		IE_TKEY = 1;     												
	}		
	
	return SUCCESS;
}

/*******************************************************************************
* Function Name  : ABS
* Description    : 求两个数差值的绝对值
* Input          : a,b
* Output         : None
* Return         : 差值绝对值
*******************************************************************************/
UINT16 ABS(UINT16 a,UINT16 b)
{
    if(a>b)
    {
        return (a-b);
    }
    else
    {
        return (b-a);
    }
}


/*******************************************************************************
* Function Name  : TK_Measure
* Description    : Measure touch input.
* Input          : None
* Return         : None
*******************************************************************************/
UINT8 TK_Measure( void )
{
	UINT8 	i,j,num;
	UINT16 err;
	UINT16 	sum;
	UINT16 	OverTime;
	
	for ( i = 0; i < TOUCH_NUM; i++ )
	{
		sum = 0;
		j = SAMPLE_TIMES;
		TK_SelectChannel( i );
        num = TK_Code[i] - 1;
		while( j-- )
		{
			OverTime = 0;
			while( ( TKEY_CTRL & bTKC_IF ) == 0 )
			{
				if( ++OverTime == 0 )
				{
					return FAIL;
				}
			}
			sum += TKEY_DAT;													/*  */
		}
		Key_DataBuf[i] = sum / SAMPLE_TIMES;
//		printf( "Key_DataBuf[%d]=%d\t", (UINT16)(i), (UINT16)Key_DataBuf[i] );
		err =  ABS(Key_FreeBuf[i],Key_DataBuf[i]);
		 if( err > DOWM_THRESHOLD_VALUE )   
		 {
			 if((Press_Flag & (1<<i)) == 0) 
			 {
				 Keyboard_Press();
				// printf("ch %d pressed,value:%d\n",(UINT16)num, Key_DataBuf[i]);
			 }	
              Press_Flag |= (1<<i);			 
		 }
		 else if( err < UP_THRESHOLD_VALUE )                                //说明抬起或者未按下
		{
			if(Press_Flag & (1<<i))                                       //刚抬起
			{
				Press_Flag &= ~(1<<i);
				// printf("ch %d up,value:%d\n",(UINT16)num, Key_DataBuf[i]);
			}
		}
	}
//	printf( "\n" );
	
	return SUCCESS;
}



/*******************************************************************************
* Function Name  : TK_int_ISR
* Description    : Touch key interrupt routing for touch key scan.
* Input          : None.
* Return         : None.
*******************************************************************************/
void TK_int_ISR( void ) interrupt INT_NO_TKEY using 1
{
	static UINT8 ch = 0;
	UINT16 KeyData;
	KeyData = TKEY_DAT;
	
	if( KeyData < ( Key_FreeBuf[ch] - TH_VALUE ) )
	{
		Touch_IN |=  1 << ( TK_Code[ch] - 1 );
	}
	
//	printf( "ch[%d]=%d\t", (UINT16)(TK_Code[ch] - 1), (UINT16)KeyData );


	if( ++ch >= TOUCH_NUM )
	{
//		printf("\n");
		ch = 0;
	}	
	TK_SelectChannel( ch );


}
/* End of file. */