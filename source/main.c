/*
 * main.c
 *
 *  Created on: Jun 25, 2018
 *      Author: Feliciano Angulo (feliciano.angulo.angulo@gmail.com)
 */

#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_common.h"

/* Interface con aplicación Baud Rate Calculator */
#include "BRC_APP.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*Definiciones para el uso del botón SW3 cómo entrada*/
#define BOARD_SW_GPIO BOARD_SW3_GPIO
#define BOARD_SW_PORT BOARD_SW3_PORT
#define BOARD_SW_GPIO_PIN BOARD_SW3_GPIO_PIN
#define BOARD_SW_IRQ BOARD_SW3_IRQ
#define BOARD_SW_IRQ_HANDLER BOARD_SW3_IRQ_HANDLER
#define BOARD_SW_NAME BOARD_SW3_NAME

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t request = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Interrupt service function of switch.
 *
 * This function toggles the LED
 */
void BOARD_SW_IRQ_HANDLER(void)
{
    /* Clear external interrupt flag. */
    GPIO_PortClearInterruptFlags(BOARD_SW_GPIO, 1U << BOARD_SW_GPIO_PIN);
    /* Change state of button. */
    request = 1;
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}


/*!
 * @brief Main function
 */
int main(void)
{
	/* Define the init structure for the input switch pin */
	gpio_pin_config_t sw_config = {
		kGPIO_DigitalInput, 0,
	};
    /* Board pin, clock, debug console init */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* initialize CAN BAUD RATE CALCULATOR application */
    BRC_Init();
    /* Configurar el puerto para el SW3 como entrada. */
	PORT_SetPinInterruptConfig(BOARD_SW_PORT, BOARD_SW_GPIO_PIN, kPORT_InterruptFallingEdge);
	EnableIRQ(BOARD_SW_IRQ);
	GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_config);

	printf("\r\n$$$$$$$$ CAN BAUD RATE CALCUALATOR $$$$$$$$$$$$\r\n");
	printf("\r\nPresione botón SW3 para calcular BaudRate de CAN\r\n");

    while (1)
    {
    	if(request)
    	{
    		/* call method for calculate baud rate */
    		BRC_CalculateBaudRate(0);
    		request = 0;
    		printf("\r\nPresione botón SW3 para calcular BaudRate de CAN\r\n");
    	}
    }
}
