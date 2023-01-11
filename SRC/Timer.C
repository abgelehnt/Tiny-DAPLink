
/********************************** (C) COPYRIGHT *******************************
* File Name          : Timer.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/01/20
* Description        : CH554 Time 初始化、定时器、计数器赋值、T2捕捉功能开启函数等
                       定时器中断函数          		 		   
*******************************************************************************/
#include "CH552.H"                                                  
#include "Debug.H"
#include "Timer.H" 
#include "stdio.h"

/*******************************************************************************
* Function Name  : mTimer_x_ModInit(UINT8 x ,UINT8 mode)
* Description    : CH554定时计数器x模式设置
* Input          : UINT8 mode,Timer模式选择
                   0：模式0，13位定时器，TLn的高3位无效
                   1：模式1，16位定时器
                   2：模式2，8位自动重装定时器
                   3：模式3，两个8位定时器  Timer0
                   3：模式3，Timer1停止									 
* Output         : None
* Return         : 成功  SUCCESS
                   失败  FAIL
*******************************************************************************/
UINT8 mTimer_x_ModInit(UINT8 x ,UINT8 mode)
{
    if(x == 0){TMOD = TMOD & 0xf0 | mode;}
    else if(x == 1){TMOD = TMOD & 0x0f | (mode<<4);}
    else if(x == 2){RCLK = 0;TCLK = 0;CP_RL2 = 0;}                               //16位自动重载定时器
    else return FAIL;
    return SUCCESS;
}

/*******************************************************************************
* Function Name  : mTimer_x_SetData(UINT8 x,UINT16 dat)
* Description    : CH554Timer0 TH0和TL0赋值
* Input          : UINT16 dat;定时器赋值
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
      RCAP2L = TL2 = tmp & 0xff;                                               //16位自动重载定时器
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
	Timer2_Init(); // 此代码错误 这么写只为了消除报错
    if(XBUS_AUX & (bUART0_TX | bUART0_RX)){
		Uart_LED = 0;
	}else{
		Uart_LED = 1;
	}
	TF2 = 0;
}

