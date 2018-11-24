//*****************************************************************************
//
//  API and private functions for the IR LED.
//  File:     ir_emitter.c
//  Version:  1.0v
//  Author:   Ronald Rodriguez Ruiz.
//  Date:     September 28, 2016.
//  ---------------------------------------------------------------------------
//  Specifications:
//  Runs on 8-bit AVR Microcontrollers (ATmega series).
//  The IR LED must be connected in the PD4 pin.
//
//*****************************************************************************

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "ir_emitter.h"
#include "bitwiseop.h"

//*****************************************************************************
//
//  The following are defines for the "Rey Eye" protocol timing.
//
//*****************************************************************************

#define IR_LED                                      PD4

//
//  Number of cycles the led is switching on and off within one pulse (1 bit).
//
#define CYCLES                                      8

//
//  The frequency at which the led operates is 33 Khz, so period = 30.3 us.
//
#define PERIOD                                      30.3

//
//  The duration of the three half bits at the beginning of a frame (us).
//
#define HALF_BIT_TIME                               427.25

//
//  The time required at the start and end of a transmision (ms).
//
#define STOP_TIME                                   2.84
#define START_TIME                                  31.95

//
//  These low level timing defines the duration of the different '0's within the frame
//
#define LOW_LEVEL1_TIME                             HALF_BIT_TIME - (CYCLES * CYCLES_FREQUENCY)
#define LOW_LEVEL2_TIME                             HALF_BIT_TIME
#define LOW_LEVEL3_TIME                             LOW_LEVEL1_TIME + LOW_LEVEL2_TIME
#define LOW_LEVEL4_TIME                             LOW_LEVEL1_TIME + (2 * LOW_LEVEL2_TIME)

//*****************************************************************************
//
//  The following are enumerations for the pulse level duration.
//
//*****************************************************************************

enum Pulse_Level
{
  HIGH_LEVEL,
  LOW_LEVEL1,
  LOW_LEVEL2,
  LOW_LEVEL3,
  LOW_LEVEL4
};

//*****************************************************************************
//
//  The following are enumerations for the commands available.
//
//*****************************************************************************

enum Commands
{
  GET_COUNTER,
  CLEAN_MEMORY
};

//*****************************************************************************
//
//  The following arrays hold the required levels to form an specific
//  ascii character based on the protocol.
//
//*****************************************************************************

static const uint8_t g_ascii_ESC[] =
{
  LOW_LEVEL2, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3
};

static const uint8_t g_ascii_DP[] =
{
  LOW_LEVEL2, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3
};

static const uint8_t g_ascii_Y[] =
{
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL3
};

static const uint8_t g_ascii_P[] =
{
  LOW_LEVEL2, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1
};

static const uint8_t g_ascii_3[] =
{
  LOW_LEVEL2, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3
};

static const uint8_t g_ascii_M[] =
{
  LOW_LEVEL2, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3
};

static const uint8_t g_ascii_I[] =
{
  LOW_LEVEL2, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3
};

static const uint8_t g_ascii_O[] =
{
  LOW_LEVEL2, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3
};

static const uint8_t g_ascii_F[] =
{
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL1
};

static const uint8_t g_ascii_FF[] =
{
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL1
};

static const uint8_t g_ascii_EOT[] =
{
  LOW_LEVEL2, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1
};

static const uint8_t g_ascii_C[] =
{
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3
};

static const uint8_t g_ascii_N[] =
{
  LOW_LEVEL2, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL1
};

static const uint8_t g_ascii_G[] =
{
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL4,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3,
  HIGH_LEVEL, LOW_LEVEL3
};

static const uint8_t g_ascii_DEL[] =
{
  LOW_LEVEL2, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL4, HIGH_LEVEL,
  LOW_LEVEL1, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3, HIGH_LEVEL,
  LOW_LEVEL3
};

static const uint8_t g_half_start_bits[] =
{
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL1,
  HIGH_LEVEL, LOW_LEVEL1
};

//*****************************************************************************
//
//  The following arrays hold the required ascii character to form an specific
//  command.
//
//*****************************************************************************

static const uint8_t* g_start_cmd[] =
{
  g_ascii_ESC, g_ascii_DP
};

static const uint8_t* g_stop_cmd[] =
{
  g_ascii_FF, g_ascii_EOT
};

static const uint8_t* g_get_couter_cmd[] =
{
  g_ascii_Y, g_ascii_P,
  g_ascii_3, g_ascii_M,
  g_ascii_I, g_ascii_O,
  g_ascii_F
};

static const uint8_t* g_clean_memory_cmd[] =
{
  g_ascii_C, g_ascii_N,
  g_ascii_F, g_ascii_G,
  g_ascii_DEL
};

//*****************************************************************************
//
//  The following arrays hold the length of each command.
//
//*****************************************************************************

static const uint8_t g_start_cmd_len[] =
{
  sizeof(g_ascii_ESC), sizeof(g_ascii_DP)
};

static const uint8_t g_stop_cmd_len[] =
{
  sizeof(g_ascii_FF), sizeof(g_ascii_EOT)
};

static const uint8_t g_get_counter_cmd_len[] =
{
  sizeof(g_ascii_Y), sizeof(g_ascii_P),
  sizeof(g_ascii_3), sizeof(g_ascii_M),
  sizeof(g_ascii_I), sizeof(g_ascii_O),
  sizeof(g_ascii_F)
};

static const uint8_t g_clean_memory_cmd_len[] =
{
  sizeof(g_ascii_C), sizeof(g_ascii_N),
  sizeof(g_ascii_F), sizeof(g_ascii_G),
  sizeof(g_ascii_DEL)
};

//*****************************************************************************
//
//  Functions for the API
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize hardware resources of the application.
//!
//! @return None.
//
//*****************************************************************************
void IR_Emitter_init(void)
{
  //
  //  Configure the IR LED as output and set low
  //
  _setBit(DDRD, IR_LED);
  _clearBit(PORTD, IR_LED);
}

//*****************************************************************************
//
//! @brief Sends a request to the electronic people counter.
//!
//! This function performs a complete transmision of one resquested command,
//! including opening and closing commands.
//!
//! @param[in] command Action requested.
//!
//! @return None.
//
//*****************************************************************************
void IR_send_request(uint8_t command)
{

  //
  //  Open the transmission with the start command
  //
  command_transmission(g_start_cmd, g_start_cmd_len, sizeof(g_start_cmd_len));
  _delay_ms(START_TIME);

  //
  //  Check which command should be send next
  //
  switch (command)
  {
    case GET_COUNTER:
      command_transmission(g_get_couter_cmd, g_get_couter_cmd_len, sizeof(g_get_couter_cmd_len));
    break;

    case CLEAN_MEMORY:
      command_transmission(g_clean_memory_cmd, g_clean_memory_cmd_len, sizeof(g_clean_memory_cmd_len));
    break;
  }

  //
  //  Close the transmission with the stop command
  //
  command_transmission(g_stop_cmd, g_stop_cmd_len, sizeof(g_stop_cmd_len));
}

//*****************************************************************************
//
//  Private Functions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Transmit a single command.
//!
//! This performs a complete transmision of a single command. The command is
//! sent by "Rey Eye" frames.
//!
//! @param[in] *frames[] Array of frames that comprise in a single command.
//! @param[in] *frames_length Length of each frame in the command.
//! @param[in] num_frames Number of frames included in the passed command.
//!
//! @return None.
//
//*****************************************************************************
static void command_transmission(const uint8_t* frames[], const uint8_t* frames_length, uint8_t num_frames);
{
  for (uint8_t i = 0; i < num_frames; i++)
  {
    //
    //  Transmission of the four error bits and the 8 data bits
    //  of the frame (ascii character).
    //
    frame_transmission(frames[i], frames_length[i]);
  }
}

//*****************************************************************************
//
//! @brief Transmit a single frame.
//!
//! This performs a complete transmision of a single frame, including opening
//! and closing half bits of the frame.
//!
//! @param[in] data_frame An specific ascii character.
//! @param[in] length Length of the frame.
//!
//! @return None.
//
//*****************************************************************************
void frame_transmission(const uint8_t* data_frame, uint8_t length)
{
  uint8_t i;

  for (i = 0; i < sizeof(half_start_bits); i++)
  {
    //
    //  Transmission of the opening three half-start-bits included
    //  in the frame.
    //
    ir_led_transmission(*half_start_bits++);
  }

  //
  //  Loop through the data array to passed the pulse duration levels
  //  to the ir led.
  //
  for (i = 0; i < length; i++)
  {
    ir_led_transmission(*data_frame++);
  }

  //
  //  Fixed time between frame transmission.
  //
  _delay_ms(STOP_TIME);
}

//*****************************************************************************
//
//! @brief Transmit a single level.
//!
//! This function evalutes the level passed to define if the IR LED has be set
//! to High (switching on and off), or has to be keep off for a certain time.
//!
//! @param[in] level The IR LED state, either on or off.
//!
//! @return None.
//
//*****************************************************************************
void ir_led_transmission(uint8_t level)
{
  uint8_t cycles;
  switch (level)
  {
    case HIGH_LEVEL:
      //
      //  Transmit 8 cycles at 33kHz
      //
      for (cycles = 0; cycles < CYCLES; cycles++)
      {
        _setBit(PORTD, IR_LED);
        _delay_us(PERIOD / 2);
        _clearBit(PORTD, IR_LED);
        _delay_us(PERIOD / 2);
      }
    break;

    //
    //  The low level produces the same state in the IR LED 0,
    //  but as the level increases (1, 2, 3...) the time increases as well.
    //

    case LOW_LEVEL1:
      _delay_us(LOW_LEVEL1_TIME);
    break;

    case LOW_LEVEL2:
      _delay_us(LOW_LEVEL2_TIME);
    break;

    case LOW_LEVEL3:
      _delay_us(LOW_LEVEL3_TIME);
    break;

    case LOW_LEVEL4:
      _delay_us(LOW_LEVEL4_TIME);
    break;
  }
}
