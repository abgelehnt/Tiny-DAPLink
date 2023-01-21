
/********************************** (C) COPYRIGHT *******************************
* File Name          : Timer.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/01/20
* Description        : CH554 Time ��ʼ������ʱ������������ֵ��T2��׽���ܿ���������
                       ��ʱ���жϺ���          		 		   
*******************************************************************************/
#include "CH552.H"                                                  
#include "Debug.H"
#include "Timer.H" 
#include "stdio.h"

/*******************************************************************************
* Function Name  : mTimer_x_ModInit(UINT8 x ,UINT8 mode)
* Description    : CH554��ʱ������xģʽ����
* Input          : UINT8 mode,Timerģʽѡ��
                   0��ģʽ0��13λ��ʱ����TLn�ĸ�3λ��Ч
                   1��ģʽ1��16λ��ʱ��
                   2��ģʽ2��8λ�Զ���װ��ʱ��
                   3��ģʽ3������8λ��ʱ��  Timer0
                   3��ģʽ3��Timer1ֹͣ									 
* Output         : None
* Return         : �ɹ�  SUCCESS
                   ʧ��  FAIL
*******************************************************************************/
UINT8 mTimer_x_ModInit(UINT8 x ,UINT8 mode)
{
    if(x == 0){TMOD = TMOD & 0xf0 | mode;}
    else if(x == 1){TMOD = TMOD & 0x0f | (mode<<4);}
    else if(x == 2){RCLK = 0;TCLK = 0;CP_RL2 = 0;}                               //16λ�Զ����ض�ʱ��
    else return FAIL;
    return SUCCESS;
}

/*******************************************************************************
* Function Name  : mTimer_x_SetData(UINT8 x,UINT16 dat)
* Description    : CH554Timer0 TH0��TL0��ֵ
* Input          : UINT16 dat;��ʱ����ֵ
* Output         : None
* Return         : None
*******************************************************************************/
void mTimer_x_SetData(UINT8 x,UINT16 dat)
{
    UINT16 tmp;
    tmp = 65536 - dat;	
		if(x == 0){TL0 = tmp & 0xff;TH0 = (tmp>>8) & 0xff;}
		else if(x == 1){TL1 = tmp & 0xff;TH1 = (tmp>>8) & 0xff;}
		else if(x == 2){
      RCAP2L = TL2 = tmp & 0xff;                                               //16λ�Զ����ض�ʱ��
      RCAP2H = TH2 = (tmp>>8) & 0xff;
    }                                                 
}

void Timer2_Init(void){
	mTimer2Clk12DivFsys();
	mTimer_x_ModInit(2,1);
    mTimer_x_SetData(2,0xF80);
    mTimer2RunCTL(1);
    ET2 = 1; 
}

void mTimer2Interrupt( void ) interrupt INT_NO_TMR2 using 3{        
	Timer2_Init(); // �˴������ ��ôдֻΪ����������
    if(XBUS_AUX & (bUART0_TX | bUART0_RX)){
		Uart_LED = 0;
	}else{
		Uart_LED = 1;
	}
	TF2 = 0;
}
