//*****************************************************************************
//
//  Test of the Red Eye protocol used by HP 48GX calculators.
//  File:     main.c
//  Version:  1.0v
//  Author:   Ronald Rodriguez Ruiz.
//  Date:     September 30, 2016.
//  ---------------------------------------------------------------------------
//  Specifications:
//  The following code was tested in an atmega328p to read 12 data bytes sent
//  by a HP 48GX calculator. To read the frame sent by the calculatator a TSOP
//  1733 was used as IR sensor. The sensor must be connected to PD2, otherwise
//  the Inpute Capture mode will not work.
//
//*****************************************************************************

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ir_reciever.h"
#include "uart.h"

void main()
{
  uint8_t* data;

  //
  //  Initialize the IR reciver
  //
  IR_Reciever_init();

  //
  //  Enable global interrupts.
  //
  sei();

  while(1)
  {
    //
    //  Check if there is any data available
    //
    if(IR_is_data_available())
    {
      //
      //  Get the data from the IR reciever and print it out
      //
      data = IR_get_data();
      for (uint8_t i = 0; i < 12; i++)
      {
        UART_printf("Byte: %u\n", *data++);
      }
    }
  }
}
