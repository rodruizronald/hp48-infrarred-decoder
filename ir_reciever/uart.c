//*****************************************************************************
//
//  API functions for the UART.
//  File:     uart.c
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

#include <stdint.h>
#include <stdarg.h>
#include <avr/io.h>
#include <util/setbaud.h>
#include "bitwiseop.h"
#include "uart.h"

//*****************************************************************************
//
//  The following are defines for the UART baud rate High and Low registers.
//
//*****************************************************************************

#define UBRRH_VALUE             0x00
#define UBRRL_VALUE							0xCF

//*****************************************************************************
//
//  Functions for the API
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize UART
//!
//! This function initializes UART for 9600 baud rate (assuming 16 MHz UART
//! clock), 8 bit word length, no parity bits, one stop bit, FIFOs enabled.
//
//*****************************************************************************
void UART_init(void)
{
	//
	//	UART Control and Status Register.
	//	U2X0: Double the UART Transmission Speed (asynchronous communication).
	//	TXEN0: Transmitter Enable.
	//	RXEN0: Receiver Enable.
	//	UMSEL01 - UMSEL00: USART Mode Select -> Asynchronous.
	//	UCSZ01 - UCSZ00: Character Size -> 8 data bits.
	//	USBS0:  Stop Bit Select -> 1 bit.
	//
	_set_bit(UCSR0A, U2X0);
	_set_bit(UCSR0B, RXEN0);
	_set_bit(UCSR0B, RXCIE0);
	_clear_two_bits(UCSR0C, UMSEL01, UMSEL00);
	_set_two_bits(UCSR0C, UCSZ01, UCSZ00);
	_clear_bit(UCSR0C, USBS0);

	//
	//	UBRRnL and UBRRnH – USART Baud Rate Registers (9600 bps).
	//
	_setBit(UBRR0H, UBRRH_VALUE);
	_setBit(UBRR0L, UBRRL_VALUE);
}

//*****************************************************************************
//
//! @brief Write an ASCII character to the Serial Port.
//!
//! @param[in] data ASCII character.
//!
//! @return None.
//
//*****************************************************************************
void UART_write_char(char c)
{
	//
	//	Wait for empty transmit buffer.
	//
	while (!_read_bit(UCSR0A, UDRE0));

	//
	//	Put data into buffer, sends the data.
	//
	UDR0 = c;
}

//*****************************************************************************
//
//! @brief Read an ASCII character from the Serial Port.
//!
//! @return ASCII character.
//
//*****************************************************************************
char UART_read_char(void)
{
	//
	//	Wait for data to be received.
	//
	while (!_read_bit(UCSR0A, RXC0));

	//
	//	Get and return received data from buffer.
	//
	return UDR0;
}

//*****************************************************************************
//
//! @brief Read an String from the Serial Port.
//!
//! This function uses a pointer to an empty buffer (buffer) to hold the
//! incoming bytes that must not exceed the buffer size (buffer_size).
//!
//! @param[in] buffer Pointer to an empty buffer
//! @param[in] buffer_size Size of the empty buffer
//!
//! @return None.
//
//*****************************************************************************
void UART_read_string(char* buffer, uint16_t buffer_size)
{
	char character;
	uint16_t buffer_index = 0;

	//
	//	Read the incoming bytes until a carriage return '\n' is received.
	//
	character = UART_read_char();
  while(character != '\n')
  {
    //
    //	If there is still space on the buffer, write the new characters.
    //
    if(buffer_index < (buffer_size - 1))
    {
    	buffer[buffer_index++] = character;
    }

    character = UART_read_char();
  }

	//
	//	Add the NUL character to close the String.
	//
	buffer[buffer_index] = '\0';
}

//*****************************************************************************
//
//! @brief Write an String to the Serial Port.
//!
//! @param[in] ptr Pointer to a NULL termination String.
//!
//! @return None.
//
//*****************************************************************************
void UART_write_string(char* ptr)
{
  while(*ptr)
  {
  	UART_writeChar(*ptr++);
  }
}

//*****************************************************************************
//
//! @brief Read a 32-bit unsigned number from the Serial Port.
//!
//! This function reads an ASCII input in unsigned decimal format and
//! converts it to a 32-bit unsigned number.
//!
//! @note Valid range is from 0 to 4294967295, any number above that will
//! return an incorrect value.
//!
//! @return 32-bit unsigned number.
//
//*****************************************************************************
uint32_t UART_read_udec(void)
{
	char character;
	uint32_t number = 0;

	//
	//	Read the incoming bytes until a carriage return '\n' is received.
	//
	character = UART_read_char();
	while(character != '\n')
	{
		//
		//	Print ASCII characters between 0-9.
		//
		if((character >= '0') && (character <= '9'))
		{
			//
			//	Get each number into the right position by multiplying
			//  the last number by 10 and adding it to the new one.
			//
			number = 10 * number + (character - '0');
		}

		character = UART_read_char();
	}

	return number;
}

//*****************************************************************************
//
//! @brief Write a 32-bit unsigned number to the Serial Port.
//!
//! This function uses recursion to convert an unsigned decimal number of
//! unspecified length into an ASCII string.
//!
//! @param[in] n 32-bit unsigned number.
//!
//! @return None.
//
//*****************************************************************************
void UART_write_udec(uint32_t n)
{
	if(n >= 10)
	{
		//
		//	Using recursion with a 10th of the actual number as input
		//  will allow to access highest order in it, from then, the
		//  division module will get the consequent digits one-by-one.
		//
		UART_write_udec(n / 10);
		n = n % 10;
	}

	//
	//	Convert n into an ASCII char before writing.
	//
	UART_write_char(n + '0');
}

//*****************************************************************************
//
//! @brief Print a predefined string fromat.
//!
//! This function loops through a predefined string format and insertes the
//! the list arguments in the right positions.
//!
//! @param[in] format String output format.
//! @param[in] ... List of arguments to be inserted within the format.
//!
//! @return None.
//
//*****************************************************************************
void UART_printf(char* format, ...)
{
	char* traverse;
	char* string;

	//
	//	Initialize printf arguments.
	//
	va_list arg;
	va_strat(arg, format);

	//
	//	Loop through the entire string format.
	//
	for (traverse = format; *traverse != '\0'; traverse++)
	{
		//
		//	If the current character is not '%', keep printing.
		//
		if (*traverse != '%')
		{
			UART_write_char(*traverse);
		}
		else
		{
			//
			//	Fetching and printing arguments.
			//
			*traverse++;
			switch (*traverse)
			{
				//
				//	Char.
				//
				case 'c':
					UART_write_char(va_arg(arg, char));
				break;

				//
				//	Integer.
				//
				case 'u':
					UART_write_udec(va_arg(arg, int));
				break;

				//
				//	String.
				//
				case 's':
					UART_write_string(va_arg(arg, char*));
				break;
			}
		}
	}
}
