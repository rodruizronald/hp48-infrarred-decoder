//*****************************************************************************
//
//  Definitions for bitwise operations.
//  File:     bitwiseop.h
//  Version:  1.0v
//  Author:   Ronald Rodriguez Ruiz.
//  Date:     November 05, 2015.
//  ---------------------------------------------------------------------------
//  Specifications:
//  Runs on all microcontrollers (8-bit, 16-bit, 32-bit).
//  The _BV macro is defined in the AVR Libc, specifically in the io.h header
//  file, and therefore must be included in the working project to avoid
//  compialtion errors.
//
//*****************************************************************************

#ifndef __BITWISEOP_H__
#define __BITWISEOP_H__

//*****************************************************************************
//
//  The following are defines for bitwise operations to handle bits/bytes
//
//*****************************************************************************

#define _read_bit(byte, bit)  \
                 (byte & _BV(bit))

#define _toggle_bit(byte, bit) \
             			 (byte ^= _BV(bit))

#define _clear_bit(byte, bit) \
              		(byte &= ~_BV(bit))

#define _clear_two_bits(byte, frist, second) \
                       (byte &= ~_BV(frist) & ~_BV(second))

#define _set_bit(byte, bit) \
                (byte |= _BV(bit))

#define _set_two_bits(byte, frist, second) \
                     (byte |= _BV(frist) | _BV(second))

#endif  // __BITWISEOP_H__
