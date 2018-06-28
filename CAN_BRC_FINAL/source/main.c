/*
 * main.c
 *
 *  Created on: Jun 25, 2018
 *      Author: felic
 */

#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "BRC_APP.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin, clock, debug console init */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Print a note to terminal */
    //PRINTF("\r\nMain function started\r\n");

    /* initialize CAN BAUD RATE CALCULATOR application */
    BRC_Init();
    /* call method for calculate baud rate */
    BRC_CalculateBaudRate(0);
    while (1)
    {
    }
}
