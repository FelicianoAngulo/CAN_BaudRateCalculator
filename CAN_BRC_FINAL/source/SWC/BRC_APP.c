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

uint8_t checkCANframe(void);
void printResults(void);
uint8_t convertToTime(void);
float roundNearest(float val);
/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define arrayLength 200
#define MAX_FRAME_LEN   131
#define INDEX_RTR       12
#define INDEX_RTR_X     32
#define INDEX_IDE       13
#define INDEX_IDE_X     33
uint8_t INDEX_DLC = 15;
#define INDEX_DATA      19
//float * appCaptureArray;
float appCaptureArray[arrayLength];
//uint32_t * pulseWidthArray;
uint32_t pulseWidthArray[arrayLength];
//uint32_t * overflowArray;
uint32_t overflowArray[arrayLength];
uint32_t baudeRates[MAX_IN_CAP] = {0};
uint8_t captureChannelList[MAX_IN_CAP] =
{
	IN_CAP0
};
uint8_t canDataArray[MAX_FRAME_LEN] = {0xFF};

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
	CAN_MSG.bit_time = 0xFFFF;
//	appCaptureArray = (float *)malloc(sizeof(float) * arrayLength);
//	pulseWidthArray = (uint32_t *)malloc(sizeof(uint32_t) * arrayLength);
//	overflowArray = (uint32_t *)malloc(sizeof(uint32_t) * arrayLength);
	printf("BRC Init\r\n");
}


uint32_t BRC_CalculateBaudRate(uint8_t channel)
{
	uint8_t success = 0;
	for(uint8_t i = 0; i < 100; i++)
	{
		if(channel < MAX_IN_CAP)
		{
			FTM_ECAL_GET_DATA(channel, &appCaptureArray[0], &overflowArray[0], (uint16_t)arrayLength);
		}
		if(convertToTime())
		{
			if(checkCANframe())
			{
				success = 1;
				break;
			}
		}

	}
	if(success)
	{
		printResults();
		return br_calculated;
	}
	else
	{
		printf("\r\n################## ERROR ###################\r\n");
		printf("####Fail to get Baud Rate after 100 attempts ####\r\n");
		return 0;
	}
}

void printResults(void)
{
	/*check baud rate*/
	br_calculated = (uint32_t)(1 / (CAN_MSG.bit_time / 1000000));
	printf("\r\nBAUDRATE = %dKbps\r\n", br_calculated / 1000);
	printf("\r\nID: 0x%x \r\n", CAN_MSG.ID);
	printf("\r\nDLC: %d \r\n", CAN_MSG.DLC);
	for(uint8_t i = 0; i < CAN_MSG.DLC; i++)
	{
		printf("\r\nDATO[%d]: %d \r\n", i, CAN_MSG.DATA[i]);
	}
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
	    		pulseWidthArray[i - 1] = (uint32_t)currentPulseWidth;
	    		//print value
	    		//printf("%d uS\r\n", (uint32_t)currentPulseWidth);

	    		if(currentPulseWidth < CAN_MSG.bit_time)
	    		{
	    			CAN_MSG.bit_time = (uint32_t)currentPulseWidth;
	    		}
	    		currentPulseWidth = 0;
	    	}
	    	else
	    	{
	    		//printf("\r\nERORR %d\r\n", currentPulseWidth);
	    		return 0;
	    	}
	    }
	return 1;
}


uint8_t checkCANframe(void)
{
	uint32_t interframe_length = CAN_MSG.bit_time * 12;
	uint32_t frameStartIndex = 0;
	uint32_t frameStopIndex = 0;
	// getting frame start and stop index
	for(uint32_t i = 0; i < arrayLength; i++)
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
		//printf("\r\nError looking interframe spaces \r\n");
		return 0;
	}
	uint8_t bit_cunter = 0;
	uint8_t bus_level = 0;
	uint8_t data_counter = 0;
	uint8_t is_stuffing = 0;
	////printf("\r\nPrinting CAN FRAME\r\n");
	for(uint32_t i = frameStartIndex; i <= frameStopIndex; i++)
	{
		bit_cunter = pulseWidthArray[i] / CAN_MSG.bit_time;
		if(bit_cunter > 6)
		{
			//printf("\r\nCAN FRAME error detected\r\n");
			return 0;
		}
		for(uint8_t j = is_stuffing; j < bit_cunter; j++)
		{
			////printf("%d", bus_level);
			canDataArray[data_counter] = bus_level;
			data_counter++;
		}
		if(bit_cunter == 5)
		{
			is_stuffing = 1;
		}
		else
		{
			is_stuffing = 0;
		}
		bus_level = !bus_level;
	}
	// analyze data in canDataArray
	/* extract ID */
	if(canDataArray[INDEX_RTR] == 0 && canDataArray[INDEX_IDE] == 0)
	{
		//standard frame
		CAN_MSG.isFD = 0;
		for(uint8_t i = 1; i < INDEX_RTR; i++)
		{
			CAN_MSG.ID |= canDataArray[i] << (11 - i);
		}
	}
	else if(canDataArray[INDEX_RTR] == 1 && canDataArray[INDEX_IDE] == 1)
	{
		//extended frame
		CAN_MSG.isFD = 1;
		INDEX_DLC = 35;
		for(uint8_t i = 1; i < INDEX_RTR_X; i++)
		{
			if(i < INDEX_RTR)
			{
				CAN_MSG.ID |= canDataArray[i] << (29 - i);
			}
			else if(i > INDEX_IDE)
			{
				CAN_MSG.ID |= canDataArray[i] << (29 - i + 2);
			}
		}
	}
	/*Extract DLC*/
	CAN_MSG.DLC |= canDataArray[INDEX_DLC] << 3;
	CAN_MSG.DLC |= canDataArray[INDEX_DLC + 1] << 2;
	CAN_MSG.DLC |= canDataArray[INDEX_DLC + 2] << 1;
	CAN_MSG.DLC |= canDataArray[INDEX_DLC + 3];
	/* extract the data. */
	uint8_t index_data = 0;
	uint8_t aux = 0;
	for(uint8_t i = 0; i < CAN_MSG.DLC; i++)
	{
		index_data = INDEX_DLC + 4 + (8 * i);
		for(uint8_t j = index_data; j < index_data + 8; j++)
		{
			aux = 7 - (j - index_data);
			CAN_MSG.DATA[i] |= canDataArray[j] << aux;
		}
	}
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
