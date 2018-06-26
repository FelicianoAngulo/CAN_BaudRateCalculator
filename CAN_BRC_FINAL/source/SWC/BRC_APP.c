/*
 * BRC_APP.c
 *
 *  Created on: Jun 25, 2018
 *      Author: felic
 */


volatile bool captureFinishedFlag = false;
#define arrayLength 200
float pulseWidth_List[arrayLength];
uint32_t val1_List[arrayLength];
uint32_t val2_List[arrayLength];




uint32_t BRC_CalculateBaudeRate()
{
	uint32_t retVal = 0;
	  uint32_t capture1Val;
	uint32_t capture2Val;
	float pulseWidth;
	uint16_t br_counter_125 = 0;
	uint16_t br_counter_250 = 0;
	uint16_t br_counter_500 = 0;
	uint16_t br_counter_1000 = 0;
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
