/*
 * The Clear BSD License
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_ftm.h"

#include "pin_mux.h"
#include "clock_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_FTM_BASEADDR FTM0

/* FTM channel used for input capture */
#define BOARD_FTM_INPUT_CAPTURE_CHANNEL kFTM_Chnl_0

/* Interrupt number and interrupt handler for the FTM base address used */
#define FTM_INTERRUPT_NUMBER FTM0_IRQn
#define FTM_INPUT_CAPTURE_HANDLER FTM0_IRQHandler

/* Interrupt to enable and flag to read */
#define FTM_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl0InterruptEnable | kFTM_TimeOverflowInterruptEnable
#define FTM_CHANNEL_FLAG kFTM_Chnl0Flag
#define FTM_OFI_FLAG kFTM_TimeOverflowFlag

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define CAPTURE_SIZE        1000

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool captureFinishedFlag = false;
uint32_t captureArray[CAPTURE_SIZE];
float pulseWidthArray[CAPTURE_SIZE / 2];
uint32_t captureCounter = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
void FTM_INPUT_CAPTURE_HANDLER(void)
{
    if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_CHANNEL_FLAG)
    {
//    	if(captureCounter == 0)// enable fall and Rise edge after first fall
//    	{
//    		FTM_SetupInputCapture(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL, kFTM_RiseAndFallEdge, 0);
//    	}
        /* Clear interrupt flag.*/
        FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL_FLAG);
        captureArray[captureCounter] = BOARD_FTM_BASEADDR->CONTROLS[BOARD_FTM_INPUT_CAPTURE_CHANNEL].CnV;
        captureCounter++;
        if(captureCounter == CAPTURE_SIZE)
		{
			FTM_DisableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
			captureFinishedFlag = true;
		}
    }
    if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_OFI_FLAG) == FTM_OFI_FLAG)
    {
    	FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_OFI_FLAG);
    	printf("overflow flag\r\n");
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    ftm_config_t ftmInfo;
    uint32_t currentPulseWidth;
    // aux vars
    uint32_t intPart = 0;
    uint32_t decimalPart = 0;

    /* Board pin, clock, debug console init */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Print a note to terminal */
    PRINTF("\r\nFTM input capture example\r\n");
    PRINTF("\r\nOnce the input signal is received the input capture value is printed\r\n");

    FTM_GetDefaultConfig(&ftmInfo);
    ftmInfo.prescale = kFTM_Prescale_Divide_16;
    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

    /* Setup dual-edge capture on a FTM channel pair */
    FTM_SetupInputCapture(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL, kFTM_RiseAndFallEdge, 0);

    /* Set the timer to be in free-running mode */
    BOARD_FTM_BASEADDR->MOD = 0xFFFF;

    /* Enable channel interrupt when the second edge is detected */
    FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);

    /* Enable at the NVIC */
    EnableIRQ(FTM_INTERRUPT_NUMBER);

    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);

    while (captureFinishedFlag != true)
    {
    }

    //captureVal = BOARD_FTM_BASEADDR->CONTROLS[BOARD_FTM_INPUT_CAPTURE_CHANNEL].CnV;
    for(uint32_t i = 1; i < CAPTURE_SIZE; i++)
    {
    	if(captureArray[i] > captureArray[i - 1])
    	{
    		currentPulseWidth = captureArray[i] - captureArray[i - 1] + 1;
    	}
    	else if(captureArray[i] < captureArray[i - 1])
    	{//65536
    		currentPulseWidth = 65536 - captureArray[i - 1] + captureArray[i];
    	}
    	else
    	{
    		printf("Error Capture[i] == Capture[i - 1");
    	}
    	//Calculate pulseWidth in milliseconds
    	if(currentPulseWidth > 0)
    	{// value was calculated correctly
    		pulseWidthArray[(i - 1) / 2] = ((float)currentPulseWidth) / (((float)(FTM_SOURCE_CLOCK / 16)) / 1000000);
    		//print value
    		intPart = (uint32_t)pulseWidthArray[(i - 1) / 2];
    		decimalPart = (uint32_t)((pulseWidthArray[(i - 1) / 2] - intPart) * 1000);
    		printf("%d.%d\r\n", intPart, decimalPart);
    		currentPulseWidth = 0;
    	}
    }
    while (1)
    {
    }
}
