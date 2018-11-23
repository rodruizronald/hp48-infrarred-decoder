//*****************************************************************************
//
//  Prototypes for the UART.
//  File:     uart.h
//  Version:  1.0v
//  Author:   Ronald Rodriguez Ruiz.
//  Date:     May 10, 2016.
//  ---------------------------------------------------------------------------
//  Specifications:
//  Runs on 8-bit AVR Microcontrollers (ATmega series).
//  The UART is configured to operate at 9600 bps, so every external devices
//  connected to the microcontroller to communicate under this protocol must
//  work within the same baud rate.
//
//*****************************************************************************

#ifndef __UART_H__
#define __UART_H__

//*****************************************************************************
//
//  Prototypes for the API
//
//*****************************************************************************

void UART_init(void);
char UART_read_char(void);
void UART_write_char(char data);
uint32_t UART_read_udec(void);
void UART_write_udec(uint32_t n);
void UART_write_string(char* ptr);
void UART_read_string(char* buffer, uint16_t buffer_size);
void UART_printf(char* format, ...);

#endif
