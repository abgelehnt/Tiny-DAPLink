/*
 * Copyright (c) 2013-2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * $Date:        1. December 2017
 * $Revision:    V2.0.0
 *
 * Project:      CMSIS-DAP Source
 * Title:        SW_DP.c CMSIS-DAP SW DP I/O
 *
 *---------------------------------------------------------------------------*/

#include "DAP.h"

#include "intrins.h"

#define SW_CLOCK_CYCLE() SWK = 0;_nop_();_nop_();SWK = 1;

#define SW_WRITE_BIT(bits) SWD = (bits)&1; SWK = 0; SWK = 1;

#define SW_READ_BIT(bits)  SWK = 0; bits = SWD; SWK = 1;

#define PIN_SWDIO_OUT_ENABLE() P1_MOD_OC &= ~(1 << 5); DIR_SWD = 1;

#define PIN_SWDIO_OUT_DISABLE() P1_MOD_OC |= (1 << 5); DIR_SWD = 0;
 
#define PIN_SPI_ENABLE() SPI0_CTRL = 0x68;

#define PIN_SPI_DISABLE() SPI0_CTRL = 0x08;

#define PIN_SPI_WRITE(x) SPI0_DATA = (x); \
							while(S0_FREE == 0);

/** Setup SWD I/O pins: SWCLK, SWDIO, and nRESET.
Configures the DAP Hardware I/O pins for Serial Wire Debug (SWD) mode:
 - SWCLK, SWDIO, nRESET to output mode and set to default high level.
 - TDI, TMS, nTRST to HighZ mode (pins are unused in SWD mode).
*/
void PORT_SWD_SETUP(void)
{
    // Set SWCLK HIGH
    //推挽输出
    P1_MOD_OC = P1_MOD_OC & ~(1 << 7);
    P1_DIR_PU = P1_DIR_PU | (1 << 7);
    SWK = 1;
	
    // Set SWDIO HIGH
    //推挽输出
    P1_MOD_OC = P1_MOD_OC & ~(1 << 5);
    P1_DIR_PU = P1_DIR_PU | (1 << 5);
    SWD = 1;
	
	//Input
    P1_MOD_OC = P1_MOD_OC & ~(1 << 6);
    P1_DIR_PU = P1_DIR_PU & ~(1 << 6);
    
    //推挽输出
    RST = 0;
    P1_MOD_OC = P1_MOD_OC & ~(1 << 4);
    P1_DIR_PU = P1_DIR_PU | (1 << 4);
	
		//1T45_DIR_SWK -Output
    DIR_SWK = 1;	
    P3_MOD_OC = P3_MOD_OC & ~(1 << 4);
    P3_DIR_PU = P3_DIR_PU | (1 << 4);
		//1T45_DIR_SWD -Output
    DIR_SWD = 1;	
    P3_MOD_OC = P3_MOD_OC & ~(1 << 2);
    P3_DIR_PU = P3_DIR_PU | (1 << 2);

	SPI0_SETUP = 8;
	SPI0_CTRL = 0x08;
	SPI0_CK_SE = 12;
}

void PORT_SWD_OFF(void)
{
	//Input
    RST = 0;
    P1_MOD_OC = P1_MOD_OC & ~(1 << 5);
    P1_DIR_PU = P1_DIR_PU & ~(1 << 5);
    P1_MOD_OC = P1_MOD_OC & ~(1 << 7);
    P1_DIR_PU = P1_DIR_PU & ~(1 << 7);
    DIR_SWK = 0;
    DIR_SWD = 0;	
}

void SWD_ResetTarget_Soft(void)
{
	UINT8 Data_Buff[4];
	Data_Buff[0] = 0x1E;
	Data_Buff[1] = 0x00;
	Data_Buff[2] = 0x00;
	Data_Buff[3] = 0x00;
	SWD_Transfer(0x00, Data_Buff);//W DP ABORT
	Data_Buff[0] = 0x00;
	SWD_Transfer(0x08, Data_Buff);//W DP SELECT
	Data_Buff[0] = 0x12;
	Data_Buff[3] = 0x23;
	SWD_Transfer(0x01, Data_Buff);//W AP CSW
	Data_Buff[0] = 0x0C;
	Data_Buff[1] = 0xED;
	Data_Buff[2] = 0x00;
	Data_Buff[3] = 0xE0;
	SWD_Transfer(0x05, Data_Buff);//W AP TAR
	Data_Buff[0] = 0x07;
	Data_Buff[1] = 0x00;
	Data_Buff[2] = 0xFA;
	Data_Buff[3] = 0x05;
	SWD_Transfer(0x0D, Data_Buff);//W AP DRW
	SWD_Transfer(0x0E, Data_Buff);//R DP RDBUFF

}

// Generate SWJ Sequence
//   count:  sequence bits count
//   datas:   pointer to sequence bits datas
//   return: none
void SWJ_Sequence(UINT8I count, const UINT8 *datas)
{
    UINT8I val;
    UINT8I n;
    val = 0U;
    n = 0U;
    while (count--)
    {
        if (n == 0U)
        {
            val = *datas++;
            n = 8U;
        }
        if (val & 1U)
        {
            SWD = 1;
        }
        else
        {
            SWD = 0;
        }
        SWK = 0;
        val >>= 1;
        n--;
        SWK = 1;
    }
}

// Generate SWD Sequence
//   info:   sequence information
//   swdo:   pointer to SWDIO generated datas
//   swdi:   pointer to SWDIO captured datas
//   return: none
void SWD_Sequence(UINT8I info, const UINT8 *swdo, UINT8 *swdi)
{
    UINT8I val;
    UINT8I bits;
    UINT8I n, k;

    n = info & SWD_SEQUENCE_CLK;
    if (n == 0U)
    {
        n = 64U;
    }

    if (info & SWD_SEQUENCE_DIN)
    {
        while (n)
        {
            val = 0U;
            for (k = 8U; k && n; k--, n--)
            {
                SW_READ_BIT(bits);
                val >>= 1;
                val |= bits << 7;
            }
            val >>= k;
            *swdi++ = (UINT8)val;
        }
    }
    else
    {
        while (n)
        {
            val = *swdo++;
            for (k = 8U; k && n; k--, n--)
            {
                SW_WRITE_BIT(val);
                val >>= 1;
            }
        }
    }
}

// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   datas:    DATA[31:0]
//   return:  ACK[2:0]
UINT8 SWD_Transfer(UINT8 req, char data * datas)
{
    UINT8 ack;
    UINT8 bits;
    UINT8 val;
    UINT8 parity;

    UINT8 m, n;

	bit bi;
	SWD = 1;
    /* Packet req */
	ACC = req&0x0f;
    parity = P;
	PIN_SPI_ENABLE();
	PIN_SPI_WRITE(0x81 | (parity) << 5 | (req&0x0f) << 1);
	PIN_SPI_DISABLE();

    /* Turnaround */
	PIN_SWDIO_OUT_DISABLE();
    for (n = turnaround; n; n--)
    {
        SW_CLOCK_CYCLE();
    }

    /* Acknowledge res */
    SW_READ_BIT(bits);
    ack = bits << 0;
    SW_READ_BIT(bits);
    ack |= bits << 1;
    SW_READ_BIT(bits);
    ack |= bits << 2;

    if (ack == DAP_TRANSFER_OK)
    {
        /* OK res */
        /* Data transfer */
        if (req & DAP_TRANSFER_RnW)
        {
            /* Read datas */
            val = 0U;
            parity = 0U;
            for (m = 0; m < 4; m++)
            {
                for (n = 8U; n; n--)
                {
                    SW_READ_BIT(bits); /* Read RDATA[0:31] */
                    val >>= 1;
                    val |= bits << 7;
                }
				ACC = val;
				parity |= P;
				parity <<= 1;
                if (datas)
                {
                    datas[m] = val;
                }
            }
			ACC = parity;
			bi = P;
            SW_READ_BIT(bits); /* Read Parity */
            if ((bi ^ bits) & 1U)
            {
                ack = DAP_TRANSFER_ERROR;
            }

            /* Turnaround */
            for (n = turnaround; n; n--)
            {
                SW_CLOCK_CYCLE();
            }
			PIN_SWDIO_OUT_ENABLE();
        }
        else
        {
            /* Turnaround */
            for (n = turnaround; n; n--)
            {
                SW_CLOCK_CYCLE();
            }
			PIN_SWDIO_OUT_ENABLE();
            /* Write datas */
            parity = 0U;

			PIN_SPI_ENABLE();
            for (m = 0; m < 4; m++)
            {
				SPI0_DATA = datas[m];
				ACC = datas[m];
				parity |= P;
				parity <<= 1;
				while(S0_FREE == 0);
            }
			PIN_SPI_DISABLE();
			ACC = parity;
            SW_WRITE_BIT(P); /* Write Parity Bit */
        }
        /* Idle cycles */
        n = idle_cycles;
        if (n)
        {
            SWD = 0;
            for (; n; n--)
            {
                SW_CLOCK_CYCLE();
            }
        }
        SWD = 1;
        return ((UINT8)ack);
    }

    if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT))
    {
        /* WAIT or FAULT res */
        if (data_phase && ((req & DAP_TRANSFER_RnW) != 0U))
        {
            for (n = 32U + 1U; n; n--)
            {
                SW_CLOCK_CYCLE(); /* Dummy Read RDATA[0:31] + Parity */
            }
        }
        /* Turnaround */
        for (n = turnaround; n; n--)
        {
            SW_CLOCK_CYCLE();
        }
		PIN_SWDIO_OUT_ENABLE();
        if (data_phase && ((req & DAP_TRANSFER_RnW) == 0U))
        {
            SWD = 0;
            for (n = 32U + 1U; n; n--)
            {
                SW_CLOCK_CYCLE(); /* Dummy Write WDATA[0:31] + Parity */
            }
        }
        SWD = 1;
        return ((UINT8)ack);
    }

    /* Protocol error */
    for (n = turnaround + 32U + 1U; n; n--)
    {
        SW_CLOCK_CYCLE(); /* Back off datas phase */
    }
	PIN_SWDIO_OUT_ENABLE();
    SWD = 1;
    return ((UINT8)ack);
}
