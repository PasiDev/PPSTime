/*
 * tools.h
 *
 * Various tool functions
 *
 *  Version:    1.0
 *  Created on: 28.12.2024
 *  Author:     Pasi
 */

#ifndef _TOOLS_H
#define _TOOLS_H

#include <stdint.h>

/****************************************************************
 * Prototypes
 ****************************************************************/
int waitPPSHigh();
int parseTimelog(char*, long double*);
void gpsSectoSystemTime(long double, struct timespec);
unsigned long calculateBlockCRC32(unsigned long, unsigned char*);

#endif /* _TOOLS_H */
