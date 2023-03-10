#include "CH552.H"
#include "Debug.H"
#include "DAP.h"
#include "Uart.h"
#include "Timer.H"
#include "AT.h"
#include "TouchKey.H"
#include "Keyboard.h"
#include "DataFlash.H"

#define Fullspeed 1
#define WINUSB 1
#define THIS_ENDP0_SIZE 64
#define ENDP4_IN_SIZE 8

UINT8X Ep0Buffer[THIS_ENDP0_SIZE] _at_ 0x0000;  //?˵?0 OUT&IN??????????????ż??ַ

// EP1: UART
// extern UINT8X Uart_TxBuff0[]; // ?˵?1 OUT&IN ˫????????????0x100
// UINT8X Ep1BufferO[THIS_ENDP0_SIZE] _at_ 0x0040; //?˵?1 OUT˫??????,??????ż??ַ Not Change!!!!!!
// UINT8X Ep1BufferI[THIS_ENDP0_SIZE] _at_ 0x0080; //?˵?1 IN˫??????,??????ż??ַ Not Change!!!!!!

// EP2: DAP-CMD
//100,140,180,1C0
UINT8X Ep2BufferO[4 * THIS_ENDP0_SIZE] _at_ 0x0100; //?˵?2 OUT??????

// EP3: DAP-ASK
//200,240,280,2C0
UINT8X Ep3BufferI[4 * THIS_ENDP0_SIZE] _at_ 0x0200; //?˵?3 IN˫??????,??????ż??ַ

UINT8X Ep4Buffer[ENDP4_IN_SIZE+2] _at_ 0x0040; //?˵?4 IN˫??????,??????ż??ַ

BOOL DAP_LED_BUSY;

UINT8I Ep2Oi, Ep2Oo;            //OUT ????
UINT8I Ep3Ii, Ep3Io;            //IN ????
UINT8I Ep3Is[DAP_PACKET_COUNT]; //???Ͱ???

PUINT8 pDescr; //USB???ñ?־
BOOL Endp3Busy = 0;
UINT8I SetupReq, SetupLen, Ready, Count, UsbConfig;
#define UsbSetupBuf ((PUSB_SETUP_REQ)Ep0Buffer)

UINT8C DevDesc[] =
{
    0x12, 0x01, 0x10, 0x02, 0xEF, 0x02, 0x01, THIS_ENDP0_SIZE,
    0x28, 0x0D, 0x04, 0x02, 0x00, 0x01, 0x01, 0x02,
    0x03, 0x01
};
UINT8C CfgDesc[] =
{
//					wTotalLength	bNumInterfaces
    0x09,	0x02,	99,		0x00,	0x03,	0x01,	0x00,	0x80,	0xfa, //??????????
//	0x09,	0x02,	74,		0x00,	0x02,	0x01,	0x00,	0x80,	0xfa, //??????????

    //DAP
//			??????	????				?˵????? ????		??????	Э????	?ַ???????
    0x09,	0x04,	0x00,	0x00,	0x02,	0xff,	0x00,	0x00,	0x04, //?ӿ???????
//					?˵???	Bulk	????????С		bInterval
    0x07,	0x05,	0x02,	0x02,	0x40,	0x00,	0x00, //?˵???????
    0x07,	0x05,	0x83,	0x02,	0x40,	0x00,	0x00,

	//CDC
    0x09,	0x04,	0x01,	0x00,	0x02,	0x02,	0x02,	0x01,	0x05,
	0x05,	0x24,	0x00,	0x10,	0x01,
    0x05,	0x24,	0x01,	0x00,	0x01,
    0x04,	0x24,	0x02,	0x02,
    0x05,	0x24,	0x06,	0x00,	0x01,
    0x07,	0x05,	0x01,	0x02,	0x40,	0x00,	0x00,
    0x07,	0x05,	0x81,	0x02,	0x40,	0x00,	0x00,

	// Keyboard
	0x09,	0x04,	0x02,	0x00,	0x01,	0x03,	0x01,	0x01,	0x06,	//?ӿ???????,????
	0x09,	0x21,	0x11,	0x01,	0x00,	0x01,	0x22,	0x3e,	0x00,	//HID????????
	0x07,	0x05,	0x84,	0x03,	8,		0x00,	0x20,					//?˵???????
};

UINT16I USB_STATUS = 0;
//cdc????
UINT8I LineCoding[7] = {0x00, 0xe1, 0x00, 0x00, 0x00, 0x00, 0x08}; //??ʼ????????Ϊ57600??1ֹͣλ????У?飬8????λ??

/*?ַ????????? ??*/
// ??????????
UINT8C MyLangDescr[] = {0x04, 0x03, 0x09, 0x04};
// ??????Ϣ:ARM
UINT8C MyManuInfo[] = {0x08, 0x03, 'A', 0x00, 'R', 0x00, 'M', 0x00};
// ??Ʒ??Ϣ: DAPLink CMSIS-DAP
UINT8C MyProdInfo[] =
{
    36,
    0x03,
    'D', 0, 'A', 0, 'P', 0, 'L', 0, 'i', 0, 'n', 0, 'k', 0, ' ', 0,
    'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0,
    'P', 0
};

// ?豸???к?
UINT8X SerNumber[] =
{
    0x12,
    0x03,
    '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
};

// ?ӿ?: CMSIS-DAP v2
UINT8C MyInterface[] =
{
    26,
    0x03,
    'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0,
    'P', 0, ' ', 0, 'v', 0, '2', 0
};
// ?ӿڣ?USB-CDC
UINT8C CDC_String[] =
{
    30,
    0x03,
    'D', 0, 'A', 0, 'P', 0, 'L', 0, 'i', 0, 'n', 0, 'k', 0, '-', 0, 
	'C', 0, 'D', 0, 'C', 0, 'E', 0, 'x', 0, 't', 0
};
// ?ӿڣ?DAPLink-Keyboard
UINT8C Keyboard_String[] =
{
    34,
    0x03,
    'D', 0, 'A', 0, 'P', 0, 'L', 0, 'i', 0, 'n', 0, 'k', 0, '-', 0, 
	'K', 0, 'e', 0, 'y', 0, 'b', 0, 'o', 0, 'a', 0, 'r', 0, 'd', 0
};

UINT8C USB_BOSDescriptor[] =
{
    0x05,                                      /* bLength */
    0x0F,                                      /* bDescriptorType */
    0x28, 0x00,                                /* wTotalLength */
    0x02,                                      /* bNumDeviceCaps */
    0x07, 0x10, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x1C,                                      /* bLength */
    0x10,                                      /* bDescriptorType */
    0x05,                                      /* bDevCapabilityType */
    0x00,                                      /* bReserved */
    0xDF, 0x60, 0xDD, 0xD8,                    /* PlatformCapabilityUUID */
    0x89, 0x45, 0xC7, 0x4C,
    0x9C, 0xD2, 0x65, 0x9D,
    0x9E, 0x64, 0x8A, 0x9F,
    0x00, 0x00, 0x03, 0x06,    /* >= Win 8.1 *//* dwWindowsVersion*/
    0xAA, 0x00,                                /* wDescriptorSetTotalLength */
    0x20,                                      /* bVendorCode */
    0x00                                       /* bAltEnumCode */
};

UINT8C WINUSB_Descriptor[] =
{
    0x0A, 0x00,                                 /* wLength */
    0x00, 0x00,                                 /* wDescriptorType */
    0x00, 0x00, 0x03, 0x06,                     /* dwWindowsVersion*/
    0xAA, 0x00,                                 /* wDescriptorSetTotalLength */
    /* ... */
    0x08, 0x00,
    0x02, 0x00,
    0x00, 0x00,
    0xA0, 0x00,
    /* ... */
    0x14, 0x00,                                 /* wLength */
    0x03, 0x00,                                 /* wDescriptorType */
    'W', 'I', 'N', 'U', 'S', 'B', 0, 0,         /* CompatibleId*/
    0, 0, 0, 0, 0, 0, 0, 0,                     /* SubCompatibleId*/
    0x84, 0x00,                                 /* wLength */
    0x04, 0x00,                                 /* wDescriptorType */
    0x07, 0x00,                                 /* wPropertyDataType */
    0x2A, 0x00,                                 /* wPropertyNameLength */
    'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0,
    'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0,
    'G', 0, 'U', 0, 'I', 0, 'D', 0, 's', 0, 0, 0,
    0x50, 0x00,                                 /* wPropertyDataLength */
    '{', 0,
    'C', 0, 'D', 0, 'B', 0, '3', 0, 'B', 0, '5', 0, 'A', 0, 'D', 0, '-', 0,
    '2', 0, '9', 0, '3', 0, 'B', 0, '-', 0,
    '4', 0, '6', 0, '6', 0, '3', 0, '-', 0,
    'A', 0, 'A', 0, '3', 0, '6', 0, '-',
    0, '1', 0, 'A', 0, 'A', 0, 'E', 0, '4', 0, '6', 0, '4', 0, '6', 0, '3', 0, '7', 0, '7', 0, '6', 0,
    '}', 0, 0, 0, 0, 0,
};

/*HID?౨????????*/
UINT8C KeyRepDesc[62] = {
	0x05,0x01,0x09,0x06,0xA1,0x01,0x05,0x07,
	0x19,0xe0,0x29,0xe7,0x15,0x00,0x25,0x01,
	0x75,0x01,0x95,0x08,0x81,0x02,0x95,0x01,
	0x75,0x08,0x81,0x01,0x95,0x03,0x75,0x01,
	0x05,0x08,0x19,0x01,0x29,0x03,0x91,0x02,
	0x95,0x05,0x75,0x01,0x91,0x01,0x95,0x06,
	0x75,0x08,0x26,0xff,0x00,0x05,0x07,0x19,
	0x00,0x29,0x91,0x81,0x00,0xC0
};

/*******************************************************************************
* Function Name  : USBDeviceInit()
* Description    : USB?豸ģʽ????,?豸ģʽ???????շ??˵????ã??жϿ???
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceInit()
{
    IE_USB = 0;
    USB_CTRL = 0x00;        // ???趨USB?豸ģʽ
    UDEV_CTRL = bUD_PD_DIS; // ??ֹDP/DM????????
#ifndef Fullspeed
    UDEV_CTRL |= bUD_LOW_SPEED; //ѡ??????1.5Mģʽ
    USB_CTRL |= bUC_LOW_SPEED;
#else
    UDEV_CTRL &= ~bUD_LOW_SPEED; //ѡ??ȫ??12Mģʽ??Ĭ?Ϸ?ʽ
    USB_CTRL &= ~bUC_LOW_SPEED;
#endif

    UEP0_DMA = Ep0Buffer;                                      //?˵?0???ݴ?????ַ
    UEP4_1_MOD &= ~(bUEP4_RX_EN | bUEP4_TX_EN);                //?˵?0??64?ֽ??շ???????
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;                 //OUT???񷵻?ACK??IN???񷵻?NAK

    UEP1_DMA = Uart_TxBuff0;									//?˵?1???ݴ?????ַ
    UEP4_1_MOD |= bUEP1_TX_EN | bUEP1_RX_EN;					//?˵?1???ͽ???ʹ??
    UEP4_1_MOD |= bUEP1_BUF_MOD;								//?˵?1?շ???˫64?ֽڻ?????
    UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK; 					//?˵?1??IN???񷵻?NAK??OUT????ACK

    UEP2_DMA = Ep2BufferO;                                     //?˵?2???ݴ?????ַ
    UEP3_DMA = Ep3BufferI;                                     //?˵?2???ݴ?????ַ
    UEP2_3_MOD |= (bUEP3_TX_EN | bUEP2_RX_EN);                   //?˵?2???ͽ???ʹ??
    UEP2_3_MOD &= ~(bUEP2_BUF_MOD | bUEP3_BUF_MOD);            //?˵?2?շ???64?ֽڻ?????
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK; //?˵?2?Զ???תͬ????־λ??IN???񷵻?NAK??OUT????ACK
    UEP3_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_NAK;//?˵?3?Զ???תͬ????־λ??IN???񷵻?NAK??OUT????NACK

    UEP4_1_MOD |= bUEP4_TX_EN; // ?˵?4????ʹ??

    USB_DEV_AD = 0x00;
    USB_CTRL |= bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN; // ????USB?豸??DMA?????ж??ڼ??жϱ?־δ????ǰ?Զ?????NAK
    UDEV_CTRL |= bUD_PORT_EN;                              // ????USB?˿?
    USB_INT_FG = 0xFF;                                     // ???жϱ?־
    USB_INT_EN = bUIE_SUSPEND | bUIE_TRANSFER | bUIE_BUS_RST;
    IE_USB = 1;
}

typedef void( *goISP)( void );
goISP ISP_ADDR=0x3800;

void DeviceInterrupt(void) interrupt INT_NO_USB //USB?жϷ???????,ʹ?üĴ?????1
{
    UINT8 len;
    if (UIF_TRANSFER) //USB???????ɱ?־
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
        case UIS_TOKEN_OUT | 2: //endpoint 2# ?˵??????´? DAP-CMD
            if (U_TOG_OK)         // ??ͬ???????ݰ???????
            {
                Ep2Oi += 64;
                UEP2_DMA_L = Ep2Oi;
            }
            break;

        case UIS_TOKEN_IN | 3: //endpoint 3# ?˵??????ϴ? DAP_ASK
            Endp3Busy = 0;
            UEP3_T_LEN = 0;      //Ԥʹ?÷??ͳ???һ??Ҫ????
            UEP3_CTRL = UEP3_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK; //Ĭ??Ӧ??NAK
            break;

        case UIS_TOKEN_IN | 1: //endpoint 1# ?˵??????ϴ? CDC
			UEP1_T_LEN = 0;
			UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;
			USB_CDC_PushData();
            break;

        case UIS_TOKEN_OUT | 1: //endpoint 1# ?˵??????´? CDC
            if (U_TOG_OK)         // ??ͬ???????ݰ???????
            {
				USB_CDC_GetData();
            }
            break;
		case UIS_TOKEN_IN | 4: //endpoint 1# ?˵??ж??ϴ?
			UEP4_T_LEN = 0;
			UEP4_CTRL ^= bUEP_T_TOG;
			UEP4_CTRL = UEP4_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //Ĭ??Ӧ??NAK
			Keyboard_Flag = 1;
			break;

        case UIS_TOKEN_SETUP | 0: //SETUP????
            len = USB_RX_LEN;
            if (len == (sizeof(USB_SETUP_REQ)))
            {
                SetupLen = UsbSetupBuf->wLengthL;
                if (UsbSetupBuf->wLengthH)
                    SetupLen = 0xFF; // ?????ܳ???
                len = 0;           // Ĭ??Ϊ?ɹ??????ϴ?0????
                SetupReq = UsbSetupBuf->bRequest;
                switch (UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK)
                {
					case USB_REQ_TYP_STANDARD:
						switch (SetupReq) //??????
						{
							case USB_GET_DESCRIPTOR:
								switch (UsbSetupBuf->wValueH){
								case 1:             //?豸??????
									pDescr = DevDesc; //???豸???????͵?Ҫ???͵Ļ?????
									len = sizeof(DevDesc);
									break;
								case 2:             //??????????
									pDescr = CfgDesc; //???豸???????͵?Ҫ???͵Ļ?????
									len = sizeof(CfgDesc);
									break;
								case 3: // ?ַ?????????
									switch (UsbSetupBuf->wValueL)
									{
										case 0:
											pDescr = (PUINT8)(&MyLangDescr[0]);
											len = sizeof(MyLangDescr);
											break;
										case 1:
											pDescr = (PUINT8)(&MyManuInfo[0]);
											len = sizeof(MyManuInfo);
											break;
										case 2:
											pDescr = (PUINT8)(&MyProdInfo[0]);
											len = sizeof(MyProdInfo);
											break;
										case 3:
											HEX_TO_ASCII(SerNumber[2],*(UINT8C *)(0x3FFC)/16);
											HEX_TO_ASCII(SerNumber[4],*(UINT8C *)(0x3FFC)%16);
											HEX_TO_ASCII(SerNumber[6],*(UINT8C *)(0x3FFD)/16);
											HEX_TO_ASCII(SerNumber[8],*(UINT8C *)(0x3FFD)%16);
											
											HEX_TO_ASCII(SerNumber[10],*(UINT8C *)(0x3FFE)/16);
											HEX_TO_ASCII(SerNumber[12],*(UINT8C *)(0x3FFE)%16);
											HEX_TO_ASCII(SerNumber[14],*(UINT8C *)(0x3FFF)/16);
											HEX_TO_ASCII(SerNumber[16],*(UINT8C *)(0x3FFF)%16);
											
											pDescr = SerNumber;
											len = sizeof(SerNumber);
											break;
										case 4:
											pDescr = (PUINT8)(&MyInterface[0]);
											len = sizeof(MyInterface);
											break;
										case 5:
											pDescr = (PUINT8)(&CDC_String[0]);
											len = sizeof(CDC_String);
											break;
										case 6:
											pDescr = (PUINT8)(&Keyboard_String[0]);
											len = sizeof(Keyboard_String);
											break;
										default:
											len = 0xFF; // ??֧?ֵ??ַ?????????
											break;
									}
									break;
								case 15:
									pDescr = (PUINT8)(&USB_BOSDescriptor[0]);
									len = sizeof(USB_BOSDescriptor);
									break;
								case 0x22:                                          //??????????
									if(UsbSetupBuf->wIndexL == 2)                   //?ӿ?0??????????
									{
										pDescr = KeyRepDesc;                        //????׼???ϴ?
										len = sizeof(KeyRepDesc);
									}
									else
									{
										len = 0xff;           //??????ֻ??2???ӿڣ????仰??????????ִ??
									}
									break;
								default:
									len = 0xff; //??֧?ֵ????????߳???
									break;
							}
							break;
						case USB_SET_ADDRESS:
							SetupLen = UsbSetupBuf->wValueL; //?ݴ?USB?豸??ַ
							break;
						case USB_GET_CONFIGURATION:
							Ep0Buffer[0] = UsbConfig;
							if (SetupLen >= 1)
							{
								len = 1;
							}
							break;
						case USB_SET_CONFIGURATION:
							UsbConfig = UsbSetupBuf->wValueL;
							if (UsbConfig)
							{
								Ready = 1; //set config????һ??????usbö?????ɵı?־
							}
							break;
						case 0x0A:
							break;
						case USB_CLEAR_FEATURE:                                                       //Clear Feature
							if ((UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) // ?˵?
							{
								switch (UsbSetupBuf->wIndexL)
								{
								case 0x82:
									UEP2_CTRL = UEP2_CTRL & ~(bUEP_T_TOG | MASK_UEP_T_RES) | UEP_T_RES_NAK;
									break;
								case 0x81:
									UEP1_CTRL = UEP1_CTRL & ~(bUEP_T_TOG | MASK_UEP_T_RES) | UEP_T_RES_NAK;
									break;
								case 0x02:
									UEP2_CTRL = UEP2_CTRL & ~(bUEP_R_TOG | MASK_UEP_R_RES) | UEP_R_RES_ACK;
									break;
								default:
									len = 0xFF; // ??֧?ֵĶ˵?
									break;
								}
							}
							else
							{
								len = 0xFF; // ???Ƕ˵㲻֧??
							}
							break;
						case USB_SET_FEATURE:                             /* Set Feature */
							if ((UsbSetupBuf->bRequestType & 0x1F) == 0x00) /* ?????豸 */
							{
								if ((((UINT16)UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x01)
								{
									if (CfgDesc[7] & 0x20)
									{
										/* ???û???ʹ?ܱ?־ */
									}
									else
									{
										len = 0xFF; /* ????ʧ?? */
									}
								}
								else
								{
									len = 0xFF; /* ????ʧ?? */
								}
							}
							else if ((UsbSetupBuf->bRequestType & 0x1F) == 0x02) /* ???ö˵? */
							{
								if ((((UINT16)UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x00)
								{
									switch (((UINT16)UsbSetupBuf->wIndexH << 8) | UsbSetupBuf->wIndexL)
									{
									case 0x82:
										UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL; /* ???ö˵?2 IN STALL */
										break;
									case 0x02:
										UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL; /* ???ö˵?2 OUT Stall */
										break;
									case 0x81:
										UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL; /* ???ö˵?1 IN STALL */
										break;
									default:
										len = 0xFF; /* ????ʧ?? */
										break;
									}
								}
								else
								{
									len = 0xFF; /* ????ʧ?? */
								}
							}
							else
							{
								len = 0xFF; /* ????ʧ?? */
							}
							break;
						case USB_GET_STATUS:
							pDescr = (PUINT8)&USB_STATUS;
							if (SetupLen >= 2)
							{
								len = 2;
							}
							else
							{
								len = SetupLen;
							}
							break;
						default:
							len = 0xff; //????ʧ??
							break;
                    }

                    break;
                case USB_REQ_TYP_CLASS: /*HID??????*/
                    if ((UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_INTERF)
                    {
                        switch (SetupReq)
                        {
                        case 0x20://Configure
                            break;
                        case 0x21://currently configured
                            pDescr = LineCoding;
                            len = sizeof(LineCoding);
                            break;
                        case 0x22://generates RS-232/V.24 style control signals
                            break;
                        default:
                            len = 0xFF; /*???֧??*/
                            break;
                        }
                    }
                    break;
                case USB_REQ_TYP_VENDOR:
                    if ((UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)
                    {
                        switch (SetupReq)
                        {
                        case 0x20:                         //GetReport
                            if (UsbSetupBuf->wIndexL == 0x07)
                            {
                                pDescr = WINUSB_Descriptor; //???豸???????͵?Ҫ???͵Ļ?????
                                len = sizeof(WINUSB_Descriptor);
                            }
                            break;
                        default:
                            len = 0xFF; /*???֧??*/
                            break;
                        }
                    }
                    break;
                default:
                    len = 0xFF;
                    break;
                }
                if (len != 0 && len != 0xFF)
                {
                    if (SetupLen > len)
                    {
                        SetupLen = len; //?????ܳ???
                    }
                    len = SetupLen >= THIS_ENDP0_SIZE ? THIS_ENDP0_SIZE : SetupLen; //???δ??䳤??
                    memcpy(Ep0Buffer, pDescr, len);                                 //?????ϴ?????
                    SetupLen -= len;
                    pDescr += len;
                }
            }
            else
            {
                len = 0xff; //?????ȴ???
            }
            if (len == 0xff)
            {
                SetupReq = 0xFF;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL; //STALL
            }
            else if (len <= THIS_ENDP0_SIZE) //?ϴ????ݻ???״̬?׶η???0???Ȱ?
            {
                UEP0_T_LEN = len;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK; //Ĭ?????ݰ???DATA1??????Ӧ??ACK
            }
            else
            {
                UEP0_T_LEN = 0;                                                      //??Ȼ??δ??״̬?׶Σ???????ǰԤ???ϴ?0???????ݰ??Է???????ǰ????״̬?׶?
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK; //Ĭ?????ݰ???DATA1,????Ӧ??ACK
            }
            break;
        case UIS_TOKEN_IN | 0: //endpoint0 IN
            switch (SetupReq)
            {
            case USB_GET_DESCRIPTOR:
            case 0x20:
                len = SetupLen >= THIS_ENDP0_SIZE ? THIS_ENDP0_SIZE : SetupLen; //???δ??䳤??
                memcpy(Ep0Buffer, pDescr, len);                                 //?????ϴ?????
                SetupLen -= len;
                pDescr += len;
                UEP0_T_LEN = len;
                UEP0_CTRL ^= bUEP_T_TOG; //ͬ????־λ??ת
                break;
            case USB_SET_ADDRESS:
                USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            default:
                UEP0_T_LEN = 0; //״̬?׶??????жϻ?????ǿ???ϴ?0???????ݰ????????ƴ???
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            }
            break;
        case UIS_TOKEN_OUT | 0: // endpoint0 OUT
            len = USB_RX_LEN;
            if (SetupReq == 0x20) //???ô???????
            {
                if (U_TOG_OK)
                {
                    memcpy(LineCoding, UsbSetupBuf, USB_RX_LEN);
					Config_Uart0(LineCoding);
                    UEP0_T_LEN = 0;
                    UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_ACK;  // ׼???ϴ?0??
                }
            }
            else if (SetupReq == 0x09)
            {
                if (Ep0Buffer[0])
                {
                }
                else if (Ep0Buffer[0] == 0)
                {
                }
            }
            UEP0_CTRL ^= bUEP_R_TOG; //ͬ????־λ??ת
            break;
        default:
            break;
        }
        UIF_TRANSFER = 0; //д0?????ж?
    }
    if (UIF_BUS_RST) //?豸ģʽUSB???߸?λ?ж?
    {
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        UEP1_CTRL = bUEP_AUTO_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
        UEP2_CTRL = bUEP_AUTO_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
        USB_DEV_AD = 0x00;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
        Endp3Busy = 0;
        UIF_BUS_RST = 0; //???жϱ?־
    }
    if (UIF_SUSPEND) //USB???߹???/????????
    {
        UIF_SUSPEND = 0;
        if (USB_MIS_ST & bUMS_SUSPEND) //????
        {
			while ( XBUS_AUX & bUART0_TX )
				;									// wait until uart0 complete transmitting.
			LED = 0;
			PCON |= PD;								//sleep
        }
    }
    else
    {
        //???????ж?,?????ܷ?????????
        USB_INT_FG = 0xFF; //???жϱ?־
    }
}

UINT8 LED_Timer;

void main(void)
{
    CfgFsys();   //CH559ʱ??ѡ??????
    mDelaymS(5); //?޸???Ƶ?ȴ??ڲ??????ȶ?,?ؼ?
   
    USBDeviceInit(); //USB?豸ģʽ??ʼ??
	UART_Setup();
    P1_MOD_OC = P1_MOD_OC & ~(1 << 4);
    P1_DIR_PU = P1_DIR_PU | (1 << 4);
    P1_MOD_OC = P1_MOD_OC & ~(1 << 5);
    P1_DIR_PU = P1_DIR_PU | (1 << 5);
	
	//Timer2_Init();
	TK_Init( BIT4,1,0 );
	ReadDataFlash(0,1,&TargetKey);
	memset(Ep4Buffer,0,8);

	SAFE_MOD = 0x55;
	SAFE_MOD = 0xAA;
	WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO;	//USB or RXD0 can wake it up
	
	
    EA = 1;          //??????Ƭ???ж?
    UEP1_T_LEN = 0;  //Ԥʹ?÷??ͳ???һ??Ҫ????
    UEP2_T_LEN = 0;  //Ԥʹ?÷??ͳ???һ??Ҫ????
    Ready = 0;

    Ep2Oi = 0;
    Ep2Oo = 0;
    Ep3Ii = 0;
    Ep3Io = 0;
    Endp3Busy = 0;
	DAP_LED_BUSY = 0;
	LED_Timer = 0;
	

	
    while (!UsbConfig) {;};

    while (1)
    {
        DAP_Thread();

        if (Endp3Busy != 1 && Ep3Ii != Ep3Io){
            Endp3Busy = 1;
            UEP3_T_LEN = Ep3Is[0];//Ep3Io>>6];
            UEP3_DMA_L = Ep3Io;
            
            UEP3_CTRL = UEP3_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK; //??????ʱ?ϴ????ݲ?Ӧ??ACK
            Ep3Io += 64;
        }
		
		
		if(DAP_LED_BUSY)
		{
			LED = 1;
		}
		else
		{
			LED_Timer++;
			if(LED_Timer==0x09)
			{
				LED = 0;
			}else if(LED_Timer==0xFF)
			{
				LED_Timer = 0;
				LED = 1;
			}	
			TK_Measure();
		}

		if(TO_IAP) {
			EA = 0;
			USB_CTRL = 0;
			UDEV_CTRL = 0x80;
			mDelaymS(100);
			(ISP_ADDR)();
		}
    }
}
