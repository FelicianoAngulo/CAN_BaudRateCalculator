/*
 * FTM_ECUAL.h
 *
 *  Created on: Jun 25, 2018
 *      Author: felic
 */

#ifndef BSW_ECUAL_FTM_ECUAL_H_
#define BSW_ECUAL_FTM_ECUAL_H_

#include "stdint.h"
typedef struct
{
	uint16_t * pulseData;
	uint16_t datalenght;
} caputureType;

void FTM_ECUAL_Init(uint8_t channelID);
void FTM_ECAL_GET_DATA(uint8_t channel, float * captureArray, uint16_t length);

#endif /* BSW_ECUAL_FTM_ECUAL_H_ */
