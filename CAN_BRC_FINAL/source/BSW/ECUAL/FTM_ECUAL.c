/*
 * FTM_ECUAL.c
 *
 *  Created on: Jun 25, 2018
 *      Author: felic
 */
#include "FTM_ECUAL.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_ftm.h"

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

volatile bool captureFinishedFlag = false;
float * captureArray;
uint32_t * ofArray;
uint16_t CAPTURE_SIZE = 0;
uint32_t captureCounter = 0;
uint32_t of_counter = 0;
void processFtmInterrupt(uint32_t value);

void FTM_ECUAL_Init(uint8_t channelID)
{
	ftm_config_t ftmInfo;
    FTM_GetDefaultConfig(&ftmInfo);
    ftmInfo.prescale = kFTM_Prescale_Divide_1;
    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo, processFtmInterrupt);

    /* Setup dual-edge capture on a FTM channel pair */
    FTM_SetupInputCapture(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL, kFTM_RiseAndFallEdge, 0);

    /* Set the timer to be in free-running mode */
    BOARD_FTM_BASEADDR->MOD = 0xFFFF;
    /* Enable channel interrupt when the second edge is detected */
	FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);

	/* Enable at the NVIC */
	EnableIRQ(FTM_INTERRUPT_NUMBER);
}

void FTM_ECAL_GET_DATA(uint8_t channel, float * arrayFortimer, uint32_t * arrayForOverflow, uint16_t length)
{
	captureArray = arrayFortimer;
	ofArray = arrayForOverflow;
	CAPTURE_SIZE = length;
	captureCounter = 0;
	of_counter = 0;
	FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);

	/* wait for finish capture */
	while (captureFinishedFlag != true)
	{
	}

	FTM_StopTimer(BOARD_FTM_BASEADDR);
	FTM_Deinit(BOARD_FTM_BASEADDR);
}

void processFtmInterrupt(uint32_t value)
{
	if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_CHANNEL_FLAG)
	{
		/* Clear interrupt flag.*/
		FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL_FLAG);
		captureArray[captureCounter] = value;
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
