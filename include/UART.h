/*
 * UART.h
 *
 * UART functions
 *
 *  Version:    1.0
 *  Created on: 28.12.2024
 *  Author:     Pasi
 */

#ifndef _UART_H
#define _UART_H

/****************************************************************
 * Prototypes
 ****************************************************************/
int uartInit();
int uartClose();
int uartTimelogCmd();
int uartTimelogRead(char*, int);

#endif /* _UART_H */
