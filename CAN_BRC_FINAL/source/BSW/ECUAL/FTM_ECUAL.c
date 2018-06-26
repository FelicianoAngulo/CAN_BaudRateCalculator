/*
 * FTM_ECUAL.c
 *
 *  Created on: Jun 25, 2018
 *      Author: felic
 */

#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_ftm.h"

void processCapturedValues(uint32_t value);
void FTM_ECUAL_Init()
{
	ftm_config_t ftmInfo;
	ftm_dual_edge_capture_param_t edgeParam;

	FTM_GetDefaultConfig(&ftmInfo);
	ftmInfo.prescale = PRESC_VALUE;
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
}

uint32_t FTM_ECAL_GET_DATA()
{
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
			pulseWidth = (float)((((float)val2_List[i] - (float)val1_List[i])) / ((float)(FTM_SOURCE_CLOCK / DIV_VALUE) / 1000000));
		}
		else if(val2_List[i] < val1_List[i])
		{
			pulseWidth = (float)((((float)(65535 - val1_List[i]) + (float)val2_List[i])) / ((float)(FTM_SOURCE_CLOCK / DIV_VALUE) / 1000000));
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
		PRINTF("Error Reading Can Speed, %d, %d", PRESC_VALUE, DIV_VALUE);
	}
	while(1)
	{

	}
	return retVal;
}

void processCapturedValues(uint32_t value)
{

}
