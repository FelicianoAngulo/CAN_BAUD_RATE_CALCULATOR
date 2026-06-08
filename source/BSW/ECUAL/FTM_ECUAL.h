/*
 * FTM_ECUAL.h
 *
 *  Created on: Jun 25, 2018
 *      Author: Feliciano Angulo (feliciano.angulo.angulo@gmail.com)
 */

#ifndef BSW_ECUAL_FTM_ECUAL_H_
#define BSW_ECUAL_FTM_ECUAL_H_

#include "stdint.h"
typedef struct
{
	uint16_t * pulseData;
	uint16_t datalenght;
} caputureType;

/*
 * Initialize the FTM ECUAL module for input capture.
 */
void FTM_ECUAL_Init(uint8_t channelID);
/*
 * Capture pulse width measurements from the selected channel.
 *
 * Parameters:
 *   channel - input capture channel index
 *   arrayForPulses - destination buffer for pulse widths in microseconds
 *   length - number of pulse widths to capture
 */
uint8_t FTM_ECAL_GET_DATA(uint8_t channel, uint32_t * arrayForPulses, uint16_t length);

#endif /* BSW_ECUAL_FTM_ECUAL_H_ */
