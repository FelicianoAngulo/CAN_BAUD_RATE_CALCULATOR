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
 * Inicializació de la aplicación
 * */
void BRC_Init(void);
/*
 * Esta función inicia la captura de pulsos en el bus de CAN,
 * el resultado se gurdad en AppPulseWidthArray
 * */
uint32_t BRC_CalculateBaudRate(uint8_t channel);


#endif /* SWC_BRC_APP_H_ */
