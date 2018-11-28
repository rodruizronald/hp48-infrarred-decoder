//*****************************************************************************
//
//  API and private functions for the IR Sensor.
//  File:     ir_reciever.c
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

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ir_reciever.h"
#include "bitwiseop.h"

//*****************************************************************************
//
//  The following are defines for the number of bytes recieved from the
//  calculator HP 48GX in one transmittion.
//
//*****************************************************************************

//
//  Each frame of the "Red Eye" protocol is composed of 30 events.
//  1 event -> one rising/falling edge
//  1.5 bits to indicate the start of a new transmission.
//  4 error bits.
//  8 data bits.
//  1.5 bits to indicate the end of the current transmission.
//
#define EVENT_BUFFER_SIZE             30

//
//  The data size may vary depend on the program that the calculator is
//  using to transmit the data.
//
#define DATA_BUFFER_SIZE              12

//*****************************************************************************
//
//  The following are defines for bit position in one data byte
//  sent by the calculator HP 48GX (these bit positions is defined based on
//  on the decodification of one data frame using the IR sensor TSOP 1733).
//
//*****************************************************************************

#define FIRST_BIT_POS                 51
#define SECOND_BIT_POS                47
#define THIRD_BIT_POS                 43
#define FOURTH_BIT_POS                39
#define FIFTH_BIT_POS                 35
#define SIXTH_BIT_POS                 31
#define SEVENTH_BIT_POS               27
#define EIGHTH_BIT_POS                23

//*****************************************************************************
//
//  The following are global varabiles used to store data, flag states, timer
//  ticks and general counters.
//
//*****************************************************************************

static uint64_t g_last_event;
static volatile uint8_t g_byte_cnt;
static volatile uint64_t g_virtual_cnt;
static volatile bool g_is_edge_falling;
static volatile bool g_is_event_buffer_full;
static volatile uint8_t g_event_buffer_index;
static uint8_t  g_data_buffer[DATA_BUFFER_SIZE];
static volatile uint64_t g_event_buffer[EVENT_BUFFER_SIZE];

//*****************************************************************************
//
//  Prototypes for private functions.
//
//*****************************************************************************

static void TIMER1_init(void);
static void update_data_buffer(void);
static void write_bit(uint8_t bit, bool logic_level);
static void find_bit_position(uint8_t bit_position, bool logic_level);

//*****************************************************************************
//
//  Functions for the API.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize the software and hardware resources of the application.
//!
//! @return None.
//
//*****************************************************************************
void
IR_Reciever_init(void)
{
  g_byte_cnt = 0;
  g_last_event = 0;
  g_virtual_cnt = 0;
  g_event_buffer_index = 0;
  g_is_edge_falling = true;
  g_is_event_buffer_full = false;

  TIMER1_init();
}

//*****************************************************************************
//
//! @brief Returns the data buffer to the user.
//!
//! @return g_data_buffer Pointer to the data buffer.
//
//*****************************************************************************
uint8_t*
IR_get_data(void)
{
  return g_data_buffer;
}

//*****************************************************************************
//
//! @brief Indicates to the user that data is ready.
//!
//! This function checks if there is any pending event, if note the user is
//! informed to proceed with the data acquisition.
//!
//! @return status Flag to indicate data availability.
//
//*****************************************************************************
bool
IR_is_data_available(void)
{
  bool status = false;

  //
  //  Check if there is pending data to be updated
  //
  if (g_is_event_buffer_full)
  {
    update_data_buffer();
    g_is_event_buffer_full = false;
  }

  //
  //  If all 12 bytes (30 events each) have been stored,
  //  then return true.
  //
  if (g_byte_cnt == DATA_BUFFER_SIZE)
  {
    status = true;
    g_byte_cnt = 0;
  }

  return status;
}

//*****************************************************************************
//
//  Private Functions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize the TIMER1 in Input Capture Mode.
//!
//! This function resets and configures the TIMER1 registers to operate in
//! Input Capture Mode. The TIMER1 is configured to run at the maximun speed
//! (no prescaler) to capture real time. Two interrupt service routines are
//! set, the Overflow ISR and the Input Capture ISR at a falling edge.
//!
//! @return None.
//
//*****************************************************************************
static void
TIMER1_init(void)
{
  //
  //  Clear the TIMER1 registers.
  //
  TCCR1A = 0x00;
  TCCR1C = 0x00;

  //
  //  Input Capture Mode Setup.
  //  CS10: Prescale 1 (Maximum Timer Speed).
  //  ICNC1: Enable Input Capture Noise Canceler.
  //  ICES1: Capture Falling Edge.
  //
  _set_two_bits(TCCR1B, ICNC1, CS10);
  _clear_bit(TCCR1B, ICES1);

  //
  //  Interrupt Service Routines Setup.
  //  ICIE1: Input Capture.
  //  TOIE1: TIMER1 OverFlow.
  //
  _set_two_bits(TIFR1, ICF1, TOV1);
  _set_two_bits(TIMSK1, ICIE1, TOIE1);
}

//*****************************************************************************
//
//! @brief Updates the data buffer (one data frame from "red eye" protocol).
//!
//! This function processes each of the 30 events in the event buffer to
//! determine the logic level, either 1 or 0, and the bit position of all
//! eight data bits within the frame. Each time this functions is executed
//! a new byte is stored in the data buffer.
//!
//! @return None.
//
//*****************************************************************************
static void
update_data_buffer(void)
{
  //
  //  General counter varaibles for loops and events.
  //
  uint8_t i, j;
  uint8_t event_cnt = 0;
  uint8_t quater_of_bit = 0;
  bool state = true;

  for (i = 0; i < EVENT_BUFFER_SIZE; i++)
  {
    //
    //  The first value is ignored.
    //
    if(i > 0)
    {
      //
      //  Calculate the pulse width.
      //
      uint16_t pulse_width = (uint16_t)g_event_buffer[i] - g_last_event;

      //
      //  Evalute of many quater of bit the pulse has, time is givin in us.
      //
      if (pulse_width > 20 && pulse_width < 100)
      {
        quater_of_bit = 1;
      }
      else if (pulse_width > 120 && pulse_width < 200)
      {
        quater_of_bit = 3;
      }
      else if (pulse_width > 220 && pulse_width < 300)
      {
        quater_of_bit = 5;
      }

      //
      //  Loop through the quaters of a bit until reaching the data frame
      //
      for (j = 0; j < quater_of_bit; j++)
      {
        //
        //  The data frame start at the 23th position, MSB is sent first.
        //
        if (event_cnt == EIGHTH_BIT_POS) find_bit_position(event_cnt, state);
        event_cnt++;
      }

      //
      //  Change logical value for the upcoming bit.
      //
      state = !state;
    }

    //
    //  Store the last event for pulse width calculation.
    //
    g_last_event = g_event_buffer[i];
  }
}

//*****************************************************************************
//
//! @brief Finds an specific bit position.
//!
//! This function evalutes the bit position passed to determine where should
//! be written the logic level.
//!
//! @param[in] bit_position Postion of one bit in the data frame.
//! @param[in] logic_level Logic level of the bit, true or false (1 or 0).
//!
//! @return None.
//
//*****************************************************************************
static void
find_bit_position(uint8_t bit_position, bool logic_level)
{
  switch (bit_position)
  {
    case FIRST_BIT_POS:
      write_bit(0, logic_level);
    break;

    case SECOND_BIT_POS:
      write_bit(1, logic_level);
    break;

    case THIRD_BIT_POS:
      write_bit(2, logic_level);
    break;

    case FOURTH_BIT_POS:
      write_bit(3, logic_level);
    break;

    case FIFTH_BIT_POS:
      write_bit(4, logic_level);
    break;

    case SIXTH_BIT_POS:
      write_bit(5, logic_level);
    break;

    case SEVENTH_BIT_POS:
      write_bit(6, logic_level);
    break;

    case EIGHTH_BIT_POS:
      write_bit(7, logic_level);
    break;
  }
}

//*****************************************************************************
//
//! @brief Writes either 1 or 0 in the passed bit.
//!
//! @param[in] bit Specific bit postion form 0-7.
//! @param[in] logic_level Logic level of the bit, true or false (1 or 0).
//!
//! @return None.
//
//*****************************************************************************
static void
write_bit(uint8_t bit, bool logic_level)
{
  //
  //  Based on the logic level value the bit position will be set (1) or clear (0)
  //
  if (logic_level)
  {
    _set_bit(g_data_buffer[g_byte_cnt], bit);
  }
  else
  {
    _clear_bit(g_data_buffer[g_byte_cnt], bit);
  }
}

//*****************************************************************************
//
//  Interrupt Service Routines
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief ISR vector for the TIMER1 OverFlow.
//!
//! This ISR updates a global virtual counter to keep track of the overall
//! time elapsed since the first Input Capture ISR. In the last two bytes
//! of this virtual counter the TIMER1 High and Low byte will be added.
//
//*****************************************************************************
ISR (TIMER1_OVF_vect)
{
  //
  //  += (uint64_t) 0x10000.
  //  00000000 -> byte 6.
  //  00000000 -> byte 5.
  //  00000000 -> byte 4.
  //  00000001 -> byte 3.
  //  00000000 -> byte 2 (TIMER1 High Byte - ICR1H).
  //  00000000 -> byte 1 (TIMER1 Low Byte - ICR1L).
  //
  g_virtual_cnt += (uint64_t) 0x10000;
}

//*****************************************************************************
//
//! @brief ISR vector for the TIMER1 Capture Input.
//!
//! This ISR access the TIMER1 High and Low byte to get the pulse width in us
//! (time elapsed since the last event). In each event the edge-triggered
//! configuration must be switched from falling to rising and viceversa to
//! guarantee a next call.
//
//*****************************************************************************
ISR (TIMER1_CAPT_vect)
{
  //
  //  Capture the current time in TIMER1.
  //  ICR1H: High Byte.
  //  ICR1L: Low Byte.
  //
  uint16_t timer_value = ((ICR1H << 8) | ICR1L);

  //
  //  Switch the edge-triggered configuration.
  //
  if (g_is_edge_falling)
  {
    //
    //  Capture rising edge next time.
    //
    _set_bit(TCCR1B, ICES1);
  }
  else
  {
    //
    //  Capture falling edge next time.
    //
    _clear_bit(TCCR1B, ICES1);
  }
  g_is_edge_falling = !g_is_edge_falling;

  //
  //  Store the input capture time of rising/falling edge-triggered.
  //
  g_event_buffer[g_event_buffer_index] = g_virtual_cnt + timer_value;

  //
  //  Increase event buffer index.
  //
  g_event_buffer_index++;

  //
  //  Check if event buffer is full.
  //
  if (g_event_buffer_index == EVENT_BUFFER_SIZE)
  {
      g_byte_cnt++;
      g_event_buffer_index = 0;
      g_is_event_buffer_full = true;
  }
}
