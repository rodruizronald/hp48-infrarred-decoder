//*****************************************************************************
//
//  Prototypes for the IR Sensor.
//  File:     ir_reciever.h
//  Version:  1.0v
//  Author:   Ronald Rodriguez Ruiz.
//  Date:     September 24, 2016.
//  ---------------------------------------------------------------------------
//  Specifications:
//  Runs on 8-bit AVR Microcontrollers (ATmega series).
//  The IR Sensor has to operate in a frequency of 33 Khz. It is recommended
//  to implement the Vishay TSOP Series 33 kHz Infrared Receivers.
//
//*****************************************************************************

#ifndef __RECIEVER_H__
#define __RECIEVER_H__

//*****************************************************************************
//
//  Prototypes for the API
//
//*****************************************************************************

extern void IR_Reciever_init(void);
extern uint8_t* IR_get_data(void);
extern bool IR_is_data_available(void);

#endif
