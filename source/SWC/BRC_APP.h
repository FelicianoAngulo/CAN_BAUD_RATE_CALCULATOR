/*
 * BRC_APP.h
 *
 *  Created on: Jun 25, 2018
 *      Author: felic
 */

#ifndef SWC_BRC_APP_H_
#define SWC_BRC_APP_H_

#include "fsl_debug_console.h"

typedef enum inputCaptureNumber
{
	IN_CAP0 = 0,
	MAX_IN_CAP
} _inputCaptureNumber;

void BRC_Init();
uint32_t BRC_CalculateBaudRate(uint8_t channel);


#endif /* SWC_BRC_APP_H_ */
