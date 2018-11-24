//*****************************************************************************
//
//  Prototypes for the IR LED.
//  File:     ir_emitter.h
//  Version:  1.0v
//  Author:   Ronald Rodriguez Ruiz.
//  Date:     September 28, 2016.
//  ---------------------------------------------------------------------------
//  Specifications:
//  Runs on 8-bit AVR Microcontrollers (ATmega series).
//  The IR LED must be connected in the PD4 pin.
//
//*****************************************************************************

#ifndef __EMITTER_H__
#define __EMITTER_H__

//*****************************************************************************
//
//  Prototypes for the API
//
//*****************************************************************************

extern void IR_Emitter_init(void);
extern void IR_send_request(uint8_t command);

#endif
