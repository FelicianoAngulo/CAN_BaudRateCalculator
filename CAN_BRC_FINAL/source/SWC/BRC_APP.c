/*
 * BRC_APP.c
 *
 *  Created on: Jun 25, 2018
 *      Author: felic
 */
#include "BRC_APP.h"
#include "FTM_ECUAL.h"
#include "stdlib.h"
#include "clock_config.h"

uint8_t convertToTime(void);
float roundNearest(float val);
/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define arrayLength 200
float * appCaptureArray;
uint32_t * pulseWidthArray;
uint32_t * overflowArray;
uint32_t baudeRates[MAX_IN_CAP] = {0};
uint8_t captureChannelList[MAX_IN_CAP] =
{
	IN_CAP0
};

struct canMsg
{
	float bit_time;
	uint8_t isFD;
	uint32_t ID;
	uint8_t DLC;
	uint8_t DATA[8];
}CAN_MSG;

uint32_t br_calculated = 0;

void BRC_Init()
{
	/*initialize FTM ECUAL */
	for(uint8_t i = 0; i < MAX_IN_CAP; i++)
	{
		FTM_ECUAL_Init(captureChannelList[i]);
	}
	appCaptureArray = (float *)malloc(sizeof(float) * arrayLength);
	pulseWidthArray = (uint32_t *)malloc(sizeof(uint32_t) * arrayLength);
	overflowArray = (uint32_t *)malloc(sizeof(uint32_t) * arrayLength);
}


uint32_t BRC_CalculateBaudRate(uint8_t channel)
{
	if(channel < MAX_IN_CAP)
	{
		FTM_ECAL_GET_DATA(channel, appCaptureArray, overflowArray, (uint16_t)arrayLength);
	}
	/*extract bit time*/
	return 0;
}

uint8_t convertToTime(void)
{
	float currentPulseWidth;
	float div = FTM_SOURCE_CLOCK;
	div = (div / 1) / 1000000;
	//div = 1000000000 / div;
	float fac = 0.016666666;

	for(uint16_t i = 1; i < arrayLength; i++)
	    {
	    	if(overflowArray[i] == 0)
	    	{
	    		if(appCaptureArray[i] <= appCaptureArray[i - 1])
	    		{
	    			currentPulseWidth = 65536 - appCaptureArray[i - 1] + appCaptureArray[i];
	    		}
	    		else
	    		{
	    			currentPulseWidth = appCaptureArray[i] - appCaptureArray[i - 1] + 1;
	    		}
	    	}
	    	else
	    	{//65536
	    		//printf("OVERFLOW:%d\r\n", overflowArray[i]);
	    		currentPulseWidth = (65536 * overflowArray[i]) - appCaptureArray[i - 1] + appCaptureArray[i];
	    	}
	    	//Calculate pulseWidth in milliseconds
	    	if(currentPulseWidth > 0)
	    	{// value was calculated correctly
	    		//currentPulseWidth = (currentPulseWidth / div) / 3;
	    		currentPulseWidth = currentPulseWidth * fac;
	    		currentPulseWidth = roundNearest(currentPulseWidth);
	    		pulseWidthArray[i - 1] = currentPulseWidth;
	    		//print value
	    		////printf("%d uS\r\n", (uint32_t)currentPulseWidth);

	    		if(currentPulseWidth < CAN_MSG.bit_time)
	    		{
	    			CAN_MSG.bit_time = (uint32_t)currentPulseWidth;
	    		}
	    		currentPulseWidth = 0;
	    	}
	    	else
	    	{
	    		printf("\r\nERORR %d\r\n", currentPulseWidth);
	    	}
	    }
	    /*check baud rate*/
	    br_calculated = (uint32_t)(1 / (CAN_MSG.bit_time / 1000000));
	    printf("\r\nBAUDRATE = %dKbps\r\n", br_calculated / 1000);
	return 1;
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
