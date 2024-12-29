/*
 * tools.c
 *
 * Various tool functions
 *
 *  Version:    1.0
 *  Created on: 28.12.2024
 *  Author:     Pasi
 */

/****************************************************************
 * Includes
 ****************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <BBBiolib.h>
#include <time.h>

#include "tools.h"

/**
 * \brief Wait PPS rising edge
 *
 * Wait until PPS is low. Then wait for PPS rising edge.
 * Return 0 when rising edge has been detected.
 * Failure if signal stays low or high for more than 3 seconds
 *
 * \return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 *
 */
int waitPPSHigh()
{
	#define PPSTIMEOUT (3000000/SLEEPTIMER) // 3 second timeout
	#define SLEEPTIMER 1000 // Sleep delay in microseconds. 1ms delay = 1ms error

	int i;

	// wait until PPS is low
	i=0;
	while (is_high(9, 23))
	{
		// Timeout check
		if(i > PPSTIMEOUT)
		{
	 	    perror("PPS signal stuck high.");
			return EXIT_FAILURE;
		}
		i++;
		usleep(SLEEPTIMER); // Don't hang
	}

	// Wait for a rising edge on PPS signal
	i=0;
	while (is_low(9, 23))
	{
		// Timeout check
		if(i > PPSTIMEOUT)
		{
	 	    perror("PPS signal stuck low.");
			return EXIT_FAILURE;
		}
		i++;
		usleep(SLEEPTIMER); // Don't hang
	}

    return EXIT_SUCCESS;
}

/**
 * \brief Parse time log
 *
 * Parse and check time log and return UTC GPS seconds
 * Check CRC and clock status.
 * Retrieve following information from time log.
 * - GPS reference weeks
 * - GPS reference seconds
 * - clock status
 * - offset
 * - UTC offset
 * Calculate offsets and UTC time. Convert UTC GPS time to seconds.
 *
 * \param logbuffer - Time log string to be parsed
 * \param oututcseconds - Return calculated UTC time in seconds
 *
 * \return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 *
 */
#define SECONDSINWEEK 604800L
int parseTimelog(char* logbuffer, long double* oututcseconds)
{
	enum returnstate { FAIL, SUCCESS };
	enum returnstate returnval = SUCCESS;
	unsigned char ucbuffer[200];
	unsigned long crcref;
	unsigned long crccalc;
	int crclength;
	const char* clockstatus;
	int refweeks;
	int utcweeks;
	long double refseconds;
	long double offset;
	long double utcoffset;
	long double utcseconds;
	const char* stringtoken;
	char* parsestring;
	const char clockstatusok[] = "VALID"; // Clock data is valid

	/* Example time log string:
	 * #TIMEA,USB1,0,50.5,FINESTEERING,2209,515163.000,02000020,9924,16809;VALID,-2.501488425e-09,6.133312031e-10,-17.99999999630,2022,5,13,23,5,45000,VALID*1100ad64
	 * reference week = 2209
	 * reference seconds = 515163.000
	 * Clock status = VALID
	 * offset = -2.501488425e-09
	 * UTC offset = -17.99999999630
	 *
	 */

	// Time logs for testing
	// Good reference
	//char testlog1[] = "#TIMEA,USB1,0,50.5,FINESTEERING,2209,515163.000,02000020,9924,16809;VALID,-2.501488425e-09,6.133312031e-10,-17.99999999630,2022,5,13,23,5,45000,VALID*1100ad64\r";
	// Failed ID, seconds go negative
	//char testlog2[] = "#TIMEB,USB1,0,50.5,FINESTEERING,2209,1000.000,02000020,9924,16809;VALID,-2.501488425e-09,6.133312031e-10,2000.9999,2022,5,13,23,5,45000,VALID*1100ad64\r";
	// Clock not valid, seconds roll over
	//char testlog3[] = "#TIMEA,USB1,0,50.5,FINESTEERING,2209,515163.000,02000020,9924,16809;CONVERGING,-2.501488425e-09,6.133312031e-10,-17.99999999630,2022,5,13,23,5,45000,VALID*1100ad64\r";

	parsestring = logbuffer; // For testing

	// Retrieve reference CRC and calculate CRC32 from our data
	stringtoken = strtok(parsestring, "*");
	crclength = strlen(stringtoken) - 1;
	memcpy(ucbuffer, parsestring+1, crclength); // Skip # character in the beginning
	stringtoken = strtok(NULL, "\r");
	crcref = strtol(stringtoken, NULL, 16);
	crccalc = calculateBlockCRC32(crclength, ucbuffer);

	// Skip unused fields
	strtok(parsestring, ","); // Skip #TIMEA
	strtok(NULL, ","); // Skip USB1
	strtok(NULL, ","); // Skip 0
	strtok(NULL, ","); // Skip 50.5
	strtok(NULL, ","); // Skip FINESTEERING
	stringtoken = strtok(NULL, ","); // Reference weeks (example 2209)
	refweeks = strtol(stringtoken, NULL, 10);
	stringtoken = strtok(NULL, ","); // Reference seconds (515163.000)
	refseconds = strtold(stringtoken, NULL);
	// Skip the rest of the log header
	strtok(NULL, ";");
	clockstatus = strtok(NULL, ","); // Clock valid or not valid
	stringtoken = strtok(NULL, ","); // Offset
	offset = strtold(stringtoken, NULL);
	strtok(NULL, ","); // skip
	stringtoken = strtok(NULL, ","); // UTC offset
	utcoffset = strtold(stringtoken, NULL);

	// Calculate offsets
	utcweeks = refweeks;
	utcseconds = refseconds - offset - utcoffset;
	if (refseconds >= 0 && utcseconds < 0) // Check if we went to negative
	{
		utcweeks--;
	} // impossible to roll over with long double

	// Add weeks to seconds
	utcseconds = utcseconds + (long double)utcweeks * SECONDSINWEEK;
	*oututcseconds = utcseconds;

	/* For debugging
	printf("Clock status: %s \r\n", clockstatus);
	printf("Ref weeks: %i \r\n", refweeks);
	printf("Ref seconds: %f \r\n", refseconds);
	printf("Offset: %f \r\n", offset);
	printf("UTC offset: %f \r\n", utcoffset);
	printf("UTC seconds: %f \r\n", utcseconds);
	printf("UTC weeks: %i \r\n", utcweeks);
	//*/

	// Check CRC, ID and clock state
	if (crcref != crccalc)
	{
		printf("CRC32 fail. Reference : 0x%.8X, Calculated : 0x%.8X \r\n", (unsigned int)crcref, (unsigned int)crccalc);
		returnval = FAIL;
	}
	if (strcmp(clockstatus, clockstatusok))
	{
		printf("Clock not ready: %s \r\n", clockstatus);
		returnval = FAIL;
	}

	if (returnval == FAIL)
	{
		fprintf(stderr, "Time log data not valid\r\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief CRC value calculation
 *
 * Calculate a CRC value to be used by CRC calculation functions.
 *
 * \param CRC input value
 *
 * \return CRC value
 *
 */
#define CRC32_POLYNOMIAL 0xEDB88320L
unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;

	for (j = 8; j > 0; j--) {

		if (ulCRC & 1)

			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}

/**
 * \brief Calculates the CRC-32 of a block of data
 *
 * Calculates the CRC-32 of a block of data all at once
 *
 * \param ulCount - Number of bytes in the data block
 * \param ucBuffer - Data block
 *
 * \return CRC-32 value
 *
 */
unsigned long calculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer)
{
	unsigned long ulCRC = 0;

	while (ulCount-- != 0) {
		unsigned long ulTemp1;
		unsigned long ulTemp2;
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xFF);
		ulCRC = ulTemp1 ^ ulTemp2;
	}

	return(ulCRC);
}

/**
 * \brief Calculate UTC time and set system time
 *
 * Convert GPS epoch to Unix epoch, calculate difference between
 * PPS rising edge and now. Set system time.
 *
 * \param utcseconds - GPS seconds from modem with offsets (GPS epoch)
 * \param ppstime - Time stamp from PPS rising edge
 *
 */
#define NS_PER_SECOND 1000000000L // Used for time calculations. ns per second
#define UNIXGPSTICKS 315964800L // Ticks between Unix epoch and GPS epoch
void gpsSectoSystemTime(long double utcseconds, struct timespec ppstime)
{
	struct timespec currtime, delta, gpstime;
	double frag;
	double integr;

	clock_gettime(CLOCK_MONOTONIC, &currtime);	// mark the end time
	delta.tv_nsec = currtime.tv_nsec - ppstime.tv_nsec;
	delta.tv_sec  = currtime.tv_sec - ppstime.tv_sec;
    if (delta.tv_sec > 0 && delta.tv_nsec < 0)
    {
    	delta.tv_nsec += NS_PER_SECOND;
    	delta.tv_sec--;
    }
    else if (delta.tv_sec < 0 && delta.tv_nsec > 0)
    {
    	delta.tv_nsec -= NS_PER_SECOND;
    	delta.tv_sec++;
    }

	// Convert to Unix seconds
    // GPS utcseconds starts from 00:00 6th January, 1980 UTC
    // Unix settime starts from 00:00 1st January, 1970 UTC
    utcseconds += UNIXGPSTICKS;

    // utcseconds to seconds and nanoseconds
	frag = modf(utcseconds, &integr);
	gpstime.tv_sec = (long)integr;
	gpstime.tv_nsec = (long)(frag * NS_PER_SECOND);

    // Add ppstime delta to utcseconds
	gpstime.tv_sec += delta.tv_sec;
	gpstime.tv_nsec += delta.tv_nsec;

    // Update system time
	clock_settime(CLOCK_MONOTONIC, &gpstime);	// Update system time

}
