/*
 * PPSTime.c
 *
 * The main function and upper level program flow
 *
 *  Version:    1.0
 *  Created on: 28.12.2024
 *  Author:     Pasi
 */

/****************************************************************
 * Includes
 ****************************************************************/
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <BBBiolib.h>

#include "PPSTime.h"
#include "tools.h"
#include "UART.h"

#define CMDBUFFERSIZE 256   // UART receive buffer size
#define TIMELOGDELAY 300000 // Time log received after PPS rising edge in us. 300ms

/**
 * \brief Main
 *
 * This is where the program does its thing.
 *
 * \return 0 on success, -1 on failure
 *
 */
int main(void)
{
	char commandbuffer[CMDBUFFERSIZE];
	struct timespec ppstime;
	long double utcseconds;

	 // Configure IO pins
    iolib_init();
    iolib_setdir(9, 23, DigitalIn); // PPS input pin

    // Initialize UART
    if (uartInit() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

    // Time log command
    if (uartTimelogCmd() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

    // Wait for the next rising edge of PPS input pin
    if (waitPPSHigh() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	clock_gettime(CLOCK_MONOTONIC, &ppstime);	// Time stamp PPS rising edge
	usleep(TIMELOGDELAY); // Wait for Time Log data 10ms + data transfer time 150ms

    // Wait for Time log input
    if (uartTimelogRead(commandbuffer, CMDBUFFERSIZE) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

    // Parse and check time log and return UTC GPS seconds
    if (parseTimelog(commandbuffer, &utcseconds) == EXIT_FAILURE)
    {
		return EXIT_FAILURE;
    }

    // Update system time from GPS seconds
    gpsSectoSystemTime(utcseconds, ppstime);

    // Close UART
    if (uartClose() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	iolib_free();

	printf("Time synchronized succesfully\n");
    return EXIT_SUCCESS;
}


