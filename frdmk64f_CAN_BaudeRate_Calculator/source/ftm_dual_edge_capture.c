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
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The Flextimer instance/channel used for board */
#define DEMO_FTM_BASEADDR FTM0

/* FTM channel pair used for the dual-edge capture, channel pair 0 uses channels 0 and 1 */
#define BOARD_FTM_INPUT_CAPTURE_CHANNEL_PAIR kFTM_Chnl_0

/* Interrupt number and interrupt handler for the FTM instance used */
#define FTM_INTERRUPT_NUMBER FTM0_IRQn
#define FTM_INPUT_CAPTURE_HANDLER FTM0_IRQHandler

/* Interrupt to enable and flag to read; depends on the FTM channel used for dual-edge capture */
#define FTM_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl1InterruptEnable
#define FTM_CHANNEL_FLAG kFTM_Chnl1Flag

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool captureFinishedFlag = false;
#define arrayLength 200
float pulseWidth_List[arrayLength];
uint32_t val1_List[arrayLength];
uint32_t val2_List[arrayLength];
uint32_t valCounter = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
void FTM_INPUT_CAPTURE_HANDLER(void)
{
	FTM_ClearStatusFlags(DEMO_FTM_BASEADDR, FTM_CHANNEL_FLAG);
	val1_List[valCounter] = DEMO_FTM_BASEADDR->CONTROLS[BOARD_FTM_INPUT_CAPTURE_CHANNEL_PAIR * 2].CnV;
	val2_List[valCounter] = DEMO_FTM_BASEADDR->CONTROLS[(BOARD_FTM_INPUT_CAPTURE_CHANNEL_PAIR * 2) + 1].CnV;
	valCounter++;
	if(valCounter == arrayLength)
	{
		FTM_DisableInterrupts(DEMO_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
	}
}

/*!
 * @brief Main function
 */

/* Main function */
int main(void)
{
    ftm_config_t ftmInfo;
    ftm_dual_edge_capture_param_t edgeParam;
    uint32_t capture1Val;
    uint32_t capture2Val;
    float pulseWidth;
	uint16_t br_counter_125 = 0;
	uint16_t br_counter_250 = 0;
	uint16_t br_counter_500 = 0;
	uint16_t br_counter_1000 = 0;

    /* Board pin, clock, debug console init */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Print a note to terminal */
    PRINTF("\r\nCAN BAUDE RATE CALCULATOR\r\n");
    PRINTF("Calculating Baud Rate with BaseClock:%d\r\n", FTM_SOURCE_CLOCK);

    FTM_GetDefaultConfig(&ftmInfo);
    /* Initialize FTM module */
    FTM_Init(DEMO_FTM_BASEADDR, &ftmInfo);

    edgeParam.mode = kFTM_Continuous;//kFTM_OneShot;
    /* Set capture edges to calculate the pulse width of input signal */
    edgeParam.currChanEdgeMode = kFTM_FallingEdge;
    edgeParam.nextChanEdgeMode = kFTM_RisingEdge;


    /* Setup dual-edge capture on a FTM channel pair */
    FTM_SetupDualEdgeCapture(DEMO_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL_PAIR, &edgeParam, 0);

    /* Set the timer to be in free-running mode */
    DEMO_FTM_BASEADDR->MOD = 0xFFFF;

    /* Enable channel interrupt when the second edge is detected */
    FTM_EnableInterrupts(DEMO_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);

    /* Enable at the NVIC */
    EnableIRQ(FTM_INTERRUPT_NUMBER);

	FTM_StartTimer(DEMO_FTM_BASEADDR, kFTM_SystemClock);

	while (valCounter < arrayLength)
	{

	}
	for(uint16_t i = 0; i < arrayLength; i++)
	{
		/* FTM clock source is not prescaled and is
		 * divided by 1000000 as the output is printed in microseconds
		 */
		if(val2_List[i] > val1_List[i])
		{
			pulseWidth = (float)((((float)val2_List[i] - (float)val1_List[i])) / ((float)(FTM_SOURCE_CLOCK / 16) / 1000000));
		}
		else if(val2_List[i] < val1_List[i])
		{
			pulseWidth = (float)((((float)(65535 - val1_List[i]) + (float)val2_List[i])) / ((float)(FTM_SOURCE_CLOCK / 16) / 1000000));
		}
		else
		{
			printf("error!!!!!!\r\n");
		}
		pulseWidth_List[i] = pulseWidth;
		capture1Val = (uint32_t)pulseWidth;

		capture2Val = (uint32_t)((pulseWidth - capture1Val) * 1000);
		printf("%d.%d\r\n", capture1Val, capture2Val);
		if(pulseWidth >= 0.6 && pulseWidth <= 1.4 )
		{
			br_counter_1000++;
		}
		else if(pulseWidth >= 1.6 && pulseWidth <= 2.4)
		{
			br_counter_500++;
		}
		else if(pulseWidth >= 3.6 && pulseWidth <= 4.4)
		{
			br_counter_250++;
		}
		else if(pulseWidth >= 7.6 && pulseWidth <= 8.4)
		{
			br_counter_125++;
		}
    }
	FTM_DisableInterrupts(DEMO_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
	if(br_counter_1000 > 1)
	{
		PRINTF("CAN BaudeRate = 1000Kb");
	}
	else if(br_counter_500 > 1)
	{
		PRINTF("CAN BaudeRate = 500Kb");
	}
	else if(br_counter_250 > 1)
	{
		PRINTF("CAN BaudeRate = 250Kb");
	}
	else if(br_counter_125 > 1)
	{
		PRINTF("CAN BaudeRate = 125Kb");
	}
	else
	{
		PRINTF("Error Reading Can Speed");
	}
	while(1)
	{

	}
}
