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
 * Analyzes the data received to detect and validate the existence of a CAN frame.
 * */
uint8_t checkCANframe(void);
/*
 * Go through the AppPulseWidthArray array
 * to get the minimum value of its elements.
 * this value will be considered as the bit time.
 * */
uint8_t checkBitTime(void);
/*
 * Prints the result with the details of the read message.
 * */
void printResults(void);
/*
 * Cleans the values of the variables used before starting a new capture.
 * */
void cleanOldData(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/* This array will contain the pulse widths */
//uint32_t * AppPulseWidthArray;
uint32_t AppPulseWidthArray[ARRAY_LENGTH];
uint32_t baudeRates[MAX_IN_CAP] = {0};
uint32_t br_calculated = 0;
uint8_t canDataArray[MAX_FRAME_LEN];

uint8_t captureChannelList[MAX_IN_CAP] =
{
	IN_CAP0
};

/* Structure to store the data of a CAN frame after reading it */
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
 * Initialize the CAN baud rate calculator application state.
 * This prepares the message buffer and initializes the FTM input capture module.
 */
void BRC_Init(void)
{
	/* Initialize the bit time to a value high enough to be greater than a possible captured bit time. */
	CAN_MSG.bit_time = 0xFFFF;
	/* Initialize the FTM input capture module for all available capture channels. */
	for(uint8_t i = 0; i < MAX_IN_CAP; i++)
	{
		FTM_ECUAL_Init(captureChannelList[i]);
	}
}

/*
 * Capture CAN waveform pulses, decode a CAN frame if possible, and calculate the baud rate.
 *
 * Parameters:
 *   channel - input capture channel index to use for the measurement.
 *
 * Returns:
 *   Calculated baud rate in bps when successful, or 0 if the capture or decoding failed.
 */
uint32_t BRC_CalculateBaudRate(uint8_t channel)
{
	uint8_t success = 0;
	uint16_t tryCounter = 100;
	/* Attempt multiple captures until a valid CAN frame is decoded or retries expire. */
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
 * Print the decoded CAN frame information and the calculated baud rate.
 */
void printResults(void)
{
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
 * Analyze the captured pulse widths to detect a CAN frame.
 *
 * This function:
 *   - computes the bit time from the captured pulses
 *   - finds the interframe gap boundaries
 *   - converts pulse widths into logical bits
 *   - removes CAN bit stuffing
 *   - extracts the CAN identifier, DLC, and data bytes
 *
 * Returns:
 *   1 if a valid frame was decoded, 0 otherwise.
 */
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
	/* Get the bit time from the captured pulse array. */
	if(!checkBitTime())
	{
		return 0;
	}
	interframe_length = CAN_MSG.bit_time * 12;
	/* Locate CAN frame boundaries by searching for a long interframe gap. */
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
				/* Convert pulse widths into raw CAN bits and remove bit stuffing. */
				for(uint32_t i = frameStartIndex; i <= frameStopIndex; i++)
				{
					bit_counter = AppPulseWidthArray[i] / CAN_MSG.bit_time;
					if(bit_counter < 6)
					{
						for(uint8_t j = is_stuffing; j < bit_counter; j++)
						{
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
					break;
				}
				else
				{
					data_counter = 0;
					frameStartIndex = frameStopIndex + 2;
				}
			}
		}
	}
	/* Validate the CAN frame format and extract identifier bits. */
	if(canDataArray[INDEX_RTR] == 0 && canDataArray[INDEX_IDE] == 0)
	{
		CAN_MSG.isFD = 0;
		INDEX_DLC = 15;
		for(uint8_t i = 1; i < INDEX_RTR; i++)
		{
			CAN_MSG.ID |= canDataArray[i] << (11 - i);
		}
	}
	else if(canDataArray[INDEX_RTR] == 1 && data_counter > (MIN_FRAME_LEN + 19) && canDataArray[INDEX_RTR_X] == 0 && canDataArray[INDEX_IDE] == 1)
	{
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
 * Find the smallest captured pulse width and use it as the nominal CAN bit time.
 *
 * Returns:
 *   1 if a valid bit time was found, 0 otherwise.
 */
uint8_t checkBitTime(void)
{
	CAN_MSG.bit_time = 0xFFFF;
	for(uint32_t i = 0; i < ARRAY_LENGTH; i++)
	{
		if(AppPulseWidthArray[i] < CAN_MSG.bit_time)
		{
			CAN_MSG.bit_time = AppPulseWidthArray[i];
		}
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
 * Reset the decoded CAN frame state before processing a new capture.
 */
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
