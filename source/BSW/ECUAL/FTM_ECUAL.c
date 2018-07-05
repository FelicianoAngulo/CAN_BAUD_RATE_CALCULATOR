/*
 * FTM_ECUAL.c
 *
 *  Created on: Jun 25, 2018
 *      Author: felic
 */
#include "FTM_ECUAL.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_ftm.h"
#include "stdlib.h"
#include "clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Index of used Peripheral*/
#define BOARD_FTM_BASEADDR FTM0
/*Work frequency of FTM module*/
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
/* FTM channel used for input capture */
#define BOARD_FTM_INPUT_CAPTURE_CHANNEL kFTM_Chnl_0
/* Interrupt number and interrupt handler for the FTM base address used */
#define FTM_INTERRUPT_NUMBER FTM0_IRQn
/*Function for handle interrupts*/
#define FTM_INPUT_CAPTURE_HANDLER FTM0_IRQHandler

/* Interrupt to enable and flag to read */
#define FTM_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl0InterruptEnable | kFTM_TimeOverflowInterruptEnable
#define FTM_CHANNEL_FLAG kFTM_Chnl0Flag
#define FTM_OFI_FLAG kFTM_TimeOverflowFlag

/*Se usa para guardar el ancho de pulso*/
uint32_t * pulseWidthArray;
/* se usa para guardar los valoes del timer en cada interrupción del FTM */
float * captureArray;
//float captureArray[arrayLength];
/*se usa para gurdar el valor del contador de overflow del timer*/
uint32_t * ofArray;
//uint32_t ofArray[arrayLength];
/*Se utiiza para señalizar cuando se han terminado las capturas*/
volatile bool captureFinishedFlag = false;
/*almacenará el valor necesario de capturas*/
uint16_t CAPTURE_SIZE = 0;
/*se usa para llevar el control del numero de capturas*/
uint32_t captureCounter = 0;
/*Se usa para contabilizar las veces que ocurre un overflow del time*/
uint32_t of_counter = 0;

/*convierte los valores del contador a pulsos con unidades de tiempo uS*/
uint8_t convertToTime(void);
/*redondea un numero al entero mas cercano*/
float roundNearest(float val);

/*Inicializa el module de FTM*/
void FTM_ECUAL_Init(uint8_t channelID)
{
	ftm_config_t ftmInfo;
    FTM_GetDefaultConfig(&ftmInfo);
    ftmInfo.prescale = kFTM_Prescale_Divide_1;
    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

    /* Setup dual-edge capture on a FTM channel pair */
    FTM_SetupInputCapture(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL,
    		kFTM_RiseAndFallEdge, 0);

    /* Set the timer to be in free-running mode */
    //BOARD_FTM_BASEADDR->MOD = 0xFFFF;
    FTM_SetTimerPeriod(BOARD_FTM_BASEADDR, 0xFFFF);
    /* Enable at the NVIC */
    EnableIRQ(FTM_INTERRUPT_NUMBER);
    /*inicializa la memoria para los arreglos*/
	if(!captureArray)
		captureArray = (float *)malloc(sizeof(float) * CAPTURE_SIZE);
	if(!ofArray)
		ofArray = (uint32_t *)malloc(sizeof(uint32_t) * CAPTURE_SIZE);
}

/*Inicia la captura de pulsos con el canal correspondiente*/
uint8_t FTM_ECAL_GET_DATA(uint8_t channel, uint32_t * arrayForPulses, uint16_t length)
{
	pulseWidthArray = arrayForPulses;
	CAPTURE_SIZE = length + 1;
	captureCounter = 0;
	of_counter = 0;
	captureFinishedFlag = false;
	/* Enable channel interrupt when the second edge is detected */
	FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
	FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);

	/* wait for finish capture */
	while (captureFinishedFlag != true)
	{
	}

	FTM_StopTimer(BOARD_FTM_BASEADDR);
	//FTM_Deinit(BOARD_FTM_BASEADDR);
	/*Regresa 1 si todo fue exitoso, 0 si ha fallado*/
	return convertToTime();
}

/*convierte los valores del contador a pulsos con unidades de tiempo uS*/
uint8_t convertToTime(void)
{
	float currentPulseWidth;
	float div = FTM_SOURCE_CLOCK;
	div = (div / 1) / 1000000;
	//div = 1000000000 / div;
	float fac = 0.016666666;

	for(uint16_t i = 1; i < CAPTURE_SIZE; i++)
	{
		if(ofArray[i] == 0)
		{
			/*se calcura el ancho de pulso con la formula ancho = t2 - t1
			 * pero se considera si el contador se ha desbordado.
			 * */
			if(captureArray[i] <= captureArray[i - 1])
			{
				currentPulseWidth = 65536 - captureArray[i - 1] + captureArray[i];
			}
			else
			{
				currentPulseWidth = captureArray[i] - captureArray[i - 1] + 1;
			}
		}
		else
		{
			currentPulseWidth = (65536 * ofArray[i]) - captureArray[i - 1] + captureArray[i];
		}
		//Calculate pulseWidth in microseconds
		//currentPulseWidth = (currentPulseWidth / div) / 3;
		currentPulseWidth = currentPulseWidth * fac;
		currentPulseWidth = roundNearest(currentPulseWidth);
		if(currentPulseWidth > 0)
		{// value was calculated correctly
			pulseWidthArray[i - 1] = (uint32_t)currentPulseWidth;
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

/*MAnejadr de la interrupcion del FTM*/
void FTM_INPUT_CAPTURE_HANDLER(void)
{
	if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_CHANNEL_FLAG)
	    {
	        /* Clear interrupt flag.*/
	        FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL_FLAG);
	        //captureArray[captureCounter] = FTM_GetCapturedRegValue(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL);
	        captureArray[captureCounter] = BOARD_FTM_BASEADDR->CONTROLS[BOARD_FTM_INPUT_CAPTURE_CHANNEL].CnV;
	        ofArray[captureCounter] = of_counter;
	        of_counter = 0;
	        captureCounter++;

	        if(captureCounter == CAPTURE_SIZE)
			{
				FTM_DisableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
				captureFinishedFlag = true;
			}
	    }
	    else if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_OFI_FLAG) == FTM_OFI_FLAG)
		{
			FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_OFI_FLAG);
			//printf("overflow flag\r\n");
			of_counter++;
		}
}

/*redondea un numero al entero mas cercano*/
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
