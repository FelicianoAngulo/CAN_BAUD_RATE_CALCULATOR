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
 * Inicializa el module FTM ECUAL
 * */
void FTM_ECUAL_Init(uint8_t channelID);
/*
 * Inicia la captura de pulsos con el canal correspondiente
 * */
uint8_t FTM_ECAL_GET_DATA(uint8_t channel, uint32_t * arrayForPulses, uint16_t length);

#endif /* BSW_ECUAL_FTM_ECUAL_H_ */
