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
#define CAPTURE_SIZE        500

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
float roundNearest(float val);
void checkCANframe(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool captureFinishedFlag = false;
uint32_t captureArray[CAPTURE_SIZE];
uint32_t ofArray[CAPTURE_SIZE];
uint32_t pulseWidthArray[CAPTURE_SIZE];
uint32_t captureCounter = 0;
uint32_t of_counter = 0;
float bit_time = 0xFFFF;
uint32_t br_calculated = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
void FTM_INPUT_CAPTURE_HANDLER(void)
{
    if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_CHANNEL_FLAG)
    {
        /* Clear interrupt flag.*/
        FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL_FLAG);
        captureArray[captureCounter] = BOARD_FTM_BASEADDR->CONTROLS[BOARD_FTM_INPUT_CAPTURE_CHANNEL].CnV;
        ofArray[captureCounter] = of_counter;
        of_counter = 0;
        captureCounter++;

        if(captureCounter == CAPTURE_SIZE)
		{
			FTM_DisableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
			captureFinishedFlag = true;
		}
    }
    else if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_OFI_FLAG) == FTM_OFI_FLAG)
	{
		FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_OFI_FLAG);
		//printf("overflow flag\r\n");
		of_counter++;
	}
}

/*!
 * @brief Main function
 */
int main(void)
{
    ftm_config_t ftmInfo;
    float currentPulseWidth;
    float div = FTM_SOURCE_CLOCK;
    div = (div / 1) / 1000000;
    //div = 1000000000 / div;
    float fac = 0.016666666;


    /* Board pin, clock, debug console init */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Print a note to terminal */
    PRINTF("\r\nFTM input capture example\r\n");
    PRINTF("\r\nOnce the input signal is received the input capture value is printed\r\n");

    FTM_GetDefaultConfig(&ftmInfo);
    ftmInfo.prescale = kFTM_Prescale_Divide_1;
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
    	if(ofArray[i] == 0)
    	{
    		if(captureArray[i] <= captureArray[i - 1])
    		{
    			currentPulseWidth = 65536 - captureArray[i - 1] + captureArray[i];
    		}
    		else
    		{
    			currentPulseWidth = captureArray[i] - captureArray[i - 1] + 1;
    		}
    	}
    	else
    	{//65536
    		//printf("OVERFLOW:%d\r\n", ofArray[i]);
    		currentPulseWidth = (65536 * ofArray[i]) - captureArray[i - 1] + captureArray[i];
    	}
    	//Calculate pulseWidth in milliseconds
    	if(currentPulseWidth > 0)
    	{// value was calculated correctly
    		//currentPulseWidth = (currentPulseWidth / div) / 3;
    		currentPulseWidth = currentPulseWidth * fac;
    		currentPulseWidth = roundNearest(currentPulseWidth);
    		pulseWidthArray[i - 1] = currentPulseWidth;
    		//print value
    		printf("%d uS\r\n", (uint32_t)currentPulseWidth);

    		if(currentPulseWidth < bit_time)
    		{
    			bit_time = (uint32_t)currentPulseWidth;
    		}
    		currentPulseWidth = 0;
    	}
    	else
    	{
    		printf("\r\nERORR %d\r\n", currentPulseWidth);
    	}
    }
    /*check baud rate*/
    br_calculated = (uint32_t)(1 / (bit_time / 1000000));
    printf("\r\nBAUDRATE = %dKbps\r\n", br_calculated / 1000);
    checkCANframe();
    while (1)
    {
    }
}

void checkCANframe(void)
{
	uint32_t interframe_length = bit_time * 12;
	uint32_t frameStartIndex = 0;
	uint32_t frameStopIndex = 0;
	for(uint32_t i = 1; i < CAPTURE_SIZE; i++)
	{
		if(pulseWidthArray[i] >= interframe_length)
		{
			if(!frameStartIndex)
			{
				frameStartIndex = i + 1;
			}
			else
			{
				frameStopIndex = i - 1;
				break;
			}
		}
	}
	if(!frameStartIndex || !frameStopIndex)
	{
		printf("\r\nError looking into CAN frame\r\n");
		return;
	}
	uint8_t bit_cunter = 0;
	uint8_t bus_level = 0;
	printf("\r\nPrinting CAN FRAME\r\n");
	for(uint32_t i = frameStartIndex; i <= frameStopIndex; i++)
	{
		bit_cunter = pulseWidthArray[i] / bit_time;
		if(bit_cunter > 6)
		{
			printf("\r\nCAN FRAME error detected\r\n");
		}
		for(uint8_t j = 0; j < bit_cunter; j++)
		{
			printf("%d", bus_level);
		}
		if(bus_level)
		{
			bus_level = 0;
		}
		else
		{
			bus_level = 1;
		}
	}
}

float roundNearest(float val)
{
	float int_part = (float)(uint32_t)val;
	float dec_part = val - int_part;
	if(dec_part > 0.5)
	{
		int_part += 1;
	}
	return int_part;
}
