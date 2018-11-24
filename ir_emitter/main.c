//*****************************************************************************
//
//  Test of the Red Eye protocol used by HP 48GX calculators (Emitter).
//  File:     main.c
//  Version:  1.0v
//  Author:   Ronald Rodriguez Ruiz.
//  Date:     September 30, 2016.
//  ---------------------------------------------------------------------------
//  Specifications:
//  The following code was tested in an atmega328p to send predefined commands
//  to another microcontroller running the ir_receiver code. The IR LED should
//  be connected to PD4 for infrared signal transmission.
//
//*****************************************************************************

#include <avr/io.h>
#include <util/delay.h>
#include "ir_emitter.h"

void main()
{
  //
  //  Initialize the IR emitter
  //
  IR_Emitter_init();

  //
  //  Send the clean memory command once
  //
  IR_send_request(CLEAN_MEMORY);
  while(1)
  {
    //
    //  Request the count every second
    //
    IR_send_request(GET_COUNTER);
    _delay_ms(1000);
  }
}
