/*
 * UART.c
 *
 * UART functions
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
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include "tools.h"

// Preserved UART settings
static struct termios oldcomconfig;
// File descriptor for UART
static int ttys1;

/**
 * \brief Initialize UART communication
 *
 * Initialize UART communication to ttys1 and open it.
 * Global: oldcomconfig - Stored UART settings
 * Global: ttys1 - File descriptor for UART
 *
 * \return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 *
 */
int uartInit()
{
	struct termios comconfig;

    // UART1 P9.24,P9.26 /dev/ttyS1, 9600bps, np, 8, 1, nh, echo off, break on
    ttys1 = open("/dev/ttyS1",O_RDWR | O_NOCTTY); // Open for reading and writing, not as controlling tty
    if(ttys1 < 0)
    {
 	   perror("/dev/ttyS1 failed to open.");
 	   return EXIT_FAILURE;
    }
    if(tcgetattr(ttys1,&oldcomconfig) < 0)
    {
 	   perror("tcgetattr of /dev/ttyS1 failed:");
 	   return EXIT_FAILURE;
    }
    if(tcgetattr(ttys1,&comconfig) < 0)
    {
 	   perror("tcgetattr of /dev/ttyS1 failed:");
 	   return EXIT_FAILURE;
    }
	bzero(&comconfig, sizeof(comconfig)); // clear struct for new port settings
    if(cfsetspeed(&comconfig,B9600) < 0) // Baudrate 9600
    {
 	   perror("cfsetspeed failed:");
 	   return EXIT_FAILURE;
    }
    comconfig.c_cflag &= ~PARENB;   // No Parity
    comconfig.c_cflag &= ~CSIZE;    // Clear data size bits
    comconfig.c_cflag |= CS8;       // 8 Bits
    comconfig.c_cflag &= ~CSTOPB;   // 1 Stop Bit
    comconfig.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow control
    comconfig.c_cflag &= ~CRTSCTS;  // Disable RTS/CTS hardware flow control
    comconfig.c_iflag &= ~IGNBRK;   // Break on
    comconfig.c_lflag &= ~ECHO;     // Disable echo
    comconfig.c_lflag &= ~ECHOE;    // Disable erasure
    comconfig.c_lflag &= ~ECHONL;   // Disable new-line echo
    comconfig.c_lflag &= ~ISIG;     // Disable interpretation of INTR, QUIT and SUSP
    comconfig.c_lflag &= ~ICANON;   // Disable canonical mode
    comconfig.c_cflag |= CREAD;     // Allow input reading
    comconfig.c_cflag |= CLOCAL;    // Ignore modem control lines
    comconfig.c_oflag &= ~OPOST;    // Prevent special interpretation of output bytes (e.g. newline chars)
    comconfig.c_oflag &= ~ONLCR;    // Prevent conversion of newline to carriage return/line feed
    comconfig.c_cc[VTIME] = 5;      // Wait for up to 0.5s, returning as soon as any data is received. Expected delay is 10ms + packet time ~200ms
    comconfig.c_cc[VMIN] = 0;
    if(tcsetattr(ttys1,TCSANOW,&comconfig) < 0)
    {
 	   perror("tcsetattr failed:");
 	   return EXIT_FAILURE;
    }
    if(tcflush(ttys1, TCIFLUSH) < 0)
    {
 	   perror("tcflush failed:");
 	   return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * \brief Closes UART ttys1
 *
 * Restores previous UART settings and closes UART file.
 * Global: oldcomconfig - Stored UART settings
 * Global: ttys1 - File descriptor for UART
 *
 * \return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 *
 */
int uartClose()
{
    if(tcsetattr(ttys1,TCSADRAIN,&oldcomconfig) < 0)
    {
 	   perror("UART1 tcsetattr failed:");
 	   return EXIT_FAILURE;
    }
    if(close(ttys1) < 0)
    {
 	   perror("UART1 close failed");
 	   return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * \brief Send Time Log command
 *
 * Send Time Log command to ttys1 UART port.
 * Global: ttys1 - File descriptor for UART
 *
 * \return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 *
 */
int uartTimelogCmd()
{
	char timelogcmd[] = "LOG COM1 TIMESYNCA ONTIME 1\r";

	if(write(ttys1, timelogcmd, sizeof(timelogcmd)) < 0)
    {
 	   perror("UART1 write failed:");
 	   return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
 * \brief Read Time Log
 *
 * Read Time Log to a fixed length buffer. 1ms per character.
 * Estimated size of the log is 150 characters.
 * UART is configured to wait data for 500ms
 * Global: ttys1 - File descriptor for UART
 *
 * \param  logbuffer Buffer for the read data
 * \param  size Size of the buffer
 * \return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 *
 */
int uartTimelogRead(char* logbuffer, int size)
{
	if(read(ttys1, logbuffer, size) > 0)
	{
	 	perror("UART1 read failed:");
	    return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

