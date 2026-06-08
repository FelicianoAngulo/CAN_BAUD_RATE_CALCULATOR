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

/* Interface with appication Baud Rate Calculator */
#include "BRC_APP.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*Definitions for using SW3 button as input*/
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
 * @brief Interrupt service function for the SW3 button.
 *
 * The handler clears the GPIO interrupt flag and sets a request flag
 * to begin CAN baud rate capture in the main loop.
 */
void BOARD_SW_IRQ_HANDLER(void)
{
    /* Clear external interrupt flag. */
    GPIO_PortClearInterruptFlags(BOARD_SW_GPIO, 1U << BOARD_SW_GPIO_PIN);
    /* Signal the main loop that a capture request has arrived. */
    request = 1;
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}


/*!
 * @brief Main application entry point.
 *
 * Initializes board peripherals and the CAN baud rate calculator application,
 * then waits for the SW3 button to trigger capture and measurement.
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

    /* Initialize CAN baud rate calculator application. */
    BRC_Init();

    /* Configure SW3 port as falling-edge interrupt input. */
	PORT_SetPinInterruptConfig(BOARD_SW_PORT, BOARD_SW_GPIO_PIN, kPORT_InterruptFallingEdge);
	EnableIRQ(BOARD_SW_IRQ);
	GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_config);

	printf("\r\n$$$$$$$$ CAN BAUD RATE CALCULATOR $$$$$$$$$$$$\r\n");
	printf("\r\nPress SW3 button to calculate CAN BaudRate\r\n");

    while (1)
    {
    	if(request)
    	{
    		/* Perform a CAN baud rate capture and calculation. */
    		BRC_CalculateBaudRate(0);
    		request = 0;
    		printf("\r\nPress SW3 button to calculate CAN BaudRate\r\n");
    	}
    }
}
