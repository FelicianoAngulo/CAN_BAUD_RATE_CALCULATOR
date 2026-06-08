/*
 * BRC_APP.h
 *
 *  Created on: Jun 25, 2018
 *      Author: Feliciano Angulo (feliciano.angulo.angulo@gmail.com)
 */

#ifndef SWC_BRC_APP_H_
#define SWC_BRC_APP_H_

#include "fsl_debug_console.h"

typedef enum inputCaptureNumber
{
	IN_CAP0 = 0,
	MAX_IN_CAP
} _inputCaptureNumber;

/*
 * Initialize the CAN baud rate calculator application.
 * This prepares internal state and initializes underlying capture hardware.
 */
void BRC_Init(void);
/*
 * Capture CAN bus timing pulses and calculate the baud rate.
 *
 * Parameters:
 *   channel - input capture channel index to use for the measurement.
 *
 * Returns:
 *   Calculated baud rate in bps on success, or 0 on failure.
 */
uint32_t BRC_CalculateBaudRate(uint8_t channel);


#endif /* SWC_BRC_APP_H_ */
