/*
 * BRC_APP.c
 *
 *  Created on: Jun 25, 2018
 *      Author: Feliciano Angulo (feliciano.angulo.angulo@gmail.com)
 */
#include "BRC_APP.h"
#include "FTM_ECUAL.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ARRAY_LENGTH 200
#define MAX_CAN_DATA 8
#define MAX_FRAME_LEN   131
#define MIN_FRAME_LEN   34
#define INDEX_RTR       12
#define INDEX_RTR_X     32
#define INDEX_IDE       13
#define INDEX_DATA      19
uint8_t INDEX_DLC = 15;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*
 * Analiza los datos recibidos para detectar y validar que exista un frame de CAN
 * */
uint8_t checkCANframe(void);
/*
 * Recorre el arreglo AppPulseWidthArray
 * para obtener el valor minimo de sus elementos.
 * dicho valor sera considerado como el bit time.
 * */
uint8_t checkBitTime(void);
/*
 * Imprime el resultado con los detalles del Mensaje leido.
 * */
void printResults(void);
/*
 * limpia los valosres de las variables usadas antes de iniciar un nueva captura.
 * */
void cleanOldData(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Este arreglo conendra los  */
//uint32_t * AppPulseWidthArray;
uint32_t AppPulseWidthArray[ARRAY_LENGTH];
uint32_t baudeRates[MAX_IN_CAP] = {0};
uint32_t br_calculated = 0;
uint8_t canDataArray[MAX_FRAME_LEN];

uint8_t captureChannelList[MAX_IN_CAP] =
{
	IN_CAP0
};

/*estructura para guardar los datos de un frame de can despues de leerlos*/
struct canMsg
{
	uint32_t bit_time;
	uint8_t isFD;
	uint32_t ID;
	uint8_t DLC;
	uint8_t DATA[MAX_CAN_DATA];
}CAN_MSG;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * Inicializació de la aplicación
 * */
void BRC_Init(void)
{
	/* Inicializar el bit time en un vlaor lo suficiente alto par aque sea mayor que un posibe bit time capturado.*/
	CAN_MSG.bit_time = 0xFFFF;
	/*initialize FTM ECUAL */
	for(uint8_t i = 0; i < MAX_IN_CAP; i++)
	{
		/* llamada al ECUAL ara inicializar el FTM de todos los canales disponibles.*/
		FTM_ECUAL_Init(captureChannelList[i]);
	}
	//printf("BRC Init\r\n");
}

/*
 * Esta función inicia la captura de pulsos en el bus de CAN,
 * el resultado se gurdad en AppPulseWidthArray
 * */
uint32_t BRC_CalculateBaudRate(uint8_t channel)
{
	uint8_t success = 0;
	uint16_t tryCounter = 100;
	/*Realizar tantos intentos como el valor en tryCounter
	 * mientras el resultado no sea exitoso
	 * */
	for(uint8_t i = 0; i < tryCounter; i++)
	{
		if(channel < MAX_IN_CAP)
		{
			if(FTM_ECAL_GET_DATA(channel, &AppPulseWidthArray[0], (uint16_t)ARRAY_LENGTH))
			{
				if(checkCANframe())
				{
					success = 1;
					break;
				}
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
		printf("####Fail to get Baud Rate after %d attempts ####\r\n", tryCounter);
		return 0;
	}
}

/*
 * Imprime el resultado con los detalles del Mensaje leido.
 * */
void printResults(void)
{
	/*check baud rate*/
	br_calculated = (1000000 / CAN_MSG.bit_time);
	printf("\r\nBIT TIME = %duS\r\n", CAN_MSG.bit_time);
	printf("\r\nBAUDRATE = %dKbps\r\n", br_calculated / 1000);
	printf("\r\nID: 0x%x \r\n", CAN_MSG.ID);
	printf("\r\nDLC: %d \r\n", CAN_MSG.DLC);
	for(uint8_t i = 0; i < CAN_MSG.DLC; i++)
	{
		printf("\r\nDATO[%d]: %d \r\n", i, CAN_MSG.DATA[i]);
	}
}

/*
 * Analiza los datos recibidos en AppPulseWidthArray
 * decodifica el contenido para validar si exise algun mensaje de CAN
 * además verifica el bit time
 * */
uint8_t checkCANframe(void)
{
	uint32_t interframe_length = 0;
	uint32_t frameStartIndex = 0;
	uint32_t frameStopIndex = 0;
	uint8_t bit_counter = 0;
	uint8_t bus_level = 0;
	uint8_t data_counter = 0;
	uint8_t is_stuffing = 0;
	uint8_t index_data = 0;
	uint8_t aux = 0;
	cleanOldData();
	/* Obtener el bit time*/
	if(!checkBitTime())
	{
		return 0;
	}
	interframe_length = CAN_MSG.bit_time * 12;
	/* Obtener la dirección de inicio y fin de un frame de CAN
	 * buscando pulsos mayores o iguales a interframe_length
	 * */
	for(uint32_t i = 0; i < ARRAY_LENGTH; i++)
	{
		if(AppPulseWidthArray[i] >= interframe_length)
		{
			if(!frameStartIndex)
			{
				frameStartIndex = i + 1;
			}
			else
			{
				frameStopIndex = i - 1;
				/*Convertir los pulsos a valores loginos 1/0
				 * remueve el bit stuffing cuando este existe.
				 * */
				for(uint32_t i = frameStartIndex; i <= frameStopIndex; i++)
				{
					bit_counter = AppPulseWidthArray[i] / CAN_MSG.bit_time;
					if(bit_counter < 6)
					{
						/*se usa is_stuffing  para omitir el primer bit(stuffing) cuando este vale 1*/
						for(uint8_t j = is_stuffing; j < bit_counter; j++)
						{
							////printf("%d", bus_level);
							canDataArray[data_counter] = bus_level;
							data_counter++;
						}
						if(bit_counter == 5)
						{
							is_stuffing = 1;
						}
						else
						{
							is_stuffing = 0;
						}
						bus_level = !bus_level;
					}
				}
				if(data_counter > MIN_FRAME_LEN)
				{
					/*cumple longitud mínima para ser un frame de CAN*/
					break;
				}
				else
				{
					/*continuar analizando el arreglo*/
					data_counter = 0;
					frameStartIndex = frameStopIndex + 2;
				}
			}
		}
	}
	// analizar el frame guradado previamente en canDataArray
	/* extract ID */
	if(canDataArray[INDEX_RTR] == 0 && canDataArray[INDEX_IDE] == 0)
	{
		//standard frame
		CAN_MSG.isFD = 0;
		INDEX_DLC = 15;
		for(uint8_t i = 1; i < INDEX_RTR; i++)
		{
			CAN_MSG.ID |= canDataArray[i] << (11 - i);
		}
	}
	else if(canDataArray[INDEX_RTR] == 1 && data_counter > (MIN_FRAME_LEN + 19) && canDataArray[INDEX_RTR_X] == 0 && canDataArray[INDEX_IDE] == 1)
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
	else
	{
		printf("\r\nCAN format Invalid\r\n");
		return 0;
	}
	/*Extract DLC*/
	CAN_MSG.DLC |= canDataArray[INDEX_DLC] << 3;
	CAN_MSG.DLC |= canDataArray[INDEX_DLC + 1] << 2;
	CAN_MSG.DLC |= canDataArray[INDEX_DLC + 2] << 1;
	CAN_MSG.DLC |= canDataArray[INDEX_DLC + 3];
	if(CAN_MSG.DLC > 8)
	{
		printf("\r\nERROR in DLC rule: DLC = %d\r\n", CAN_MSG.DLC);
		return 0;
	}
	/* extract the data. */
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

/*
 * Recorre el arreglo AppPulseWidthArray
 * para obtener el valor minimo de sus elementos.
 * dicho valor sera considerado como el bit time.
 * */
uint8_t checkBitTime(void)
{
	CAN_MSG.bit_time = 0xFFFF;
	for(uint32_t i = 0; i < ARRAY_LENGTH; i++)
	{
		if(AppPulseWidthArray[i] < CAN_MSG.bit_time)
		{
			CAN_MSG.bit_time = AppPulseWidthArray[i];
		}
		//printf("%d\r\n", AppPulseWidthArray[i]);
	}
	if(CAN_MSG.bit_time > 0 && CAN_MSG.bit_time < 1000)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
 * limpia los valosres de las variables usadas antes de iniciar un nueva captura.
 * */
void cleanOldData(void)
{
	CAN_MSG.DLC = 0;
	CAN_MSG.ID = 0;
	CAN_MSG.bit_time = 0;
	CAN_MSG.isFD = 0;
	for(uint8_t i = 0; i < MAX_CAN_DATA; i++)
	{
		CAN_MSG.DATA[i] = 0;
	}
	for(uint8_t i = 0; i < MAX_FRAME_LEN; i++)
	{
		canDataArray[i] = 0;
	}
}
