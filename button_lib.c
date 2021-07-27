#include "button_lib.h"
#include "string.h"
#include "Arduino.h"
uint32_t BUTTON_HOLD_TICK;
uint32_t BUTTON_IDLE_TICK;
typedef enum
{
  SM_IDLE = 0,
  SM_PREPARE_PRESSING,
  SM_PRESSING,
  SM_PREPARE_RELEASE,
  SM_RELEASE,
  SM_HOLDING,
  SM_PREPARE_IDLE,
} SM_t;
typedef enum
{
  RELEASED  = 0,
  PRESSED   = 1,
} button_state_t;
typedef struct
{
  button_result_t trig_result;
  void (*cb)(void);
  uint8_t is_served;
} button_cb_t;
typedef struct
{
  uint16_t pin;
  uint8_t active_logic;
  button_state_t state;
  button_result_t result;
  button_cb_t cb_data[MAX_CB_PER_BUTTON];
  uint8_t cb_length;
  SM_t current_SM;
  uint32_t SM_count; // general purpose variable used only in state machine
  int (*read_fp)(uint8_t);
  uint8_t has_valid_state;
} button_t;

button_t button[MAX_NUM_OF_BUTTON];
uint8_t button_count= 0;

/*        PRIVATE PROTOTYPE       */
bool    SM_is_processing = 0;
uint8_t button_is_changed_state();
void    button_read();
void    button_SM_process();

/*        PUBLIC FUNCTIONS        */
void button_initialize(uint32_t idle_tick, uint32_t hold_tick)
{
  BUTTON_IDLE_TICK = idle_tick;
  BUTTON_HOLD_TICK = hold_tick;
  memset(button, 0x00, sizeof(button_t) * MAX_NUM_OF_BUTTON);
}

uint8_t button_register(button_register_t register_button, button_result_t ** tracking_var)
{
  if(button_count >= MAX_NUM_OF_BUTTON ||
     register_button.callback_length > MAX_CB_PER_BUTTON)
  {
    return BUTTON_ERR; // fail to register
  }
  else
  {
    memset(&button[button_count], 0x0, sizeof(button_t));
    button[button_count].pin = register_button.pin;
    button[button_count].active_logic = register_button.active_logic;
    for(int i = 0; i < register_button.callback_length; i++)
    {
      button[button_count].cb_data[i].trig_result = register_button.trigger_result[i];
      button[button_count].cb_data[i].cb = register_button.callback[i];
    }
    button[button_count].cb_length = register_button.callback_length;
    if(register_button.read_fp == NULL)
    {
      // Use default arduino digitalread
      button[button_count].read_fp = digitalRead;
    }
    else
    {
      // use user custom function
      button[button_count].read_fp = register_button.read_fp;
    }
    *tracking_var = &button[button_count].result;
    button_count++;
  }
  return BUTTON_OK;
}

void button_update()
{
  button_read();
  if(button_is_changed_state() || SM_is_processing)
  {
    //
    button_SM_process(); 
  }
}

/*      PRIVATE FUNCTIONS       */
void button_read()
{
  //TODO: IMPLEMENT DEBOUNCE ALGORITHM IN THIS FUNCTION
  for(int i=0; i < button_count; i++)
  {
    if(button[i].read_fp(button[i].pin) == button[i].active_logic)
    {
      button[i].state = PRESSED;
    }
    else
    {
      button[i].state = RELEASED;
    }
    button[i].has_valid_state = 1;
  }
  return;
}

uint8_t button_is_changed_state()
{
  uint8_t result = 0;
  static button_state_t old_state[MAX_NUM_OF_BUTTON]={RELEASED};
  for(int i=0; i < button_count; i++)
  {
    if(old_state[i] != button[i].state)
    {
      result = 1;
      old_state[i] = button[i].state;
    }
  }
  return result;
}

void button_SM_process()
{
  SM_is_processing = 1;
  for(int i = 0; i < button_count; i++)
  {
    if(!button[i].has_valid_state) continue;
    switch(button[i].current_SM)
    {
      case SM_IDLE:
        if(button[i].state == PRESSED)
        {
          button[i].SM_count = 0;
          button[i].result.click_count = 0;
          button[i].result.is_hold = 0;
          for(int j = 0; j < button[i].cb_length; j++) button[i].cb_data[j].is_served = 0;
          button[i].current_SM = SM_PREPARE_PRESSING;
        }
        else SM_is_processing = 0;
        break;
      case SM_PREPARE_PRESSING:
        button[i].current_SM = SM_PRESSING;
        button[i].SM_count = 0;
        break;
      case SM_PRESSING:
        if(button[i].state == RELEASED)
        {
          button[i].current_SM = SM_PREPARE_RELEASE;
        }
        else
        {
          if(button[i].SM_count >= BUTTON_HOLD_TICK)
          {
            button[i].current_SM = SM_HOLDING;
            button[i].SM_count   = 0;
          }
          else button[i].SM_count++;
        }
        break;
      case SM_PREPARE_RELEASE:
        button[i].SM_count = 0;
        button[i].result.click_count++;
        button[i].current_SM = SM_RELEASE;
        break;
      case SM_RELEASE:
        if(button[i].state == PRESSED)
        {
          button[i].current_SM = SM_PREPARE_PRESSING;
        }
        else
        {
          if(button[i].SM_count >= BUTTON_IDLE_TICK)
          {
            button[i].current_SM = SM_PREPARE_IDLE;
          }
          else button[i].SM_count++;
        }
        break;
      case SM_HOLDING:
        button[i].result.is_hold = 1;
        if(button[i].state == RELEASED)
        {
          button[i].SM_count = 0;
          button[i].current_SM = SM_RELEASE;
        }
        break;
      case SM_PREPARE_IDLE:
        button[i].current_SM = SM_IDLE;
        button[i].SM_count = 0;
        button[i].result.click_count = 0;
        button[i].result.is_hold = 0;
        break;
    }
    /*execute callback*/
    if(button[i].current_SM != SM_IDLE)
    {
      for(int j = 0; j < button[i].cb_length; j++)
      {
        if(button[i].result.click_count == button[i].cb_data[j].trig_result.click_count &&
          button[i].result.is_hold == button[i].cb_data[j].trig_result.is_hold &&
          button[i].cb_data[j].cb != NULL &&
          button[i].cb_data[j].is_served == 0)
        {
          button[i].cb_data[j].is_served = 1;
          button[i].cb_data[j].cb();
        }
      }
    }
    button[i].has_valid_state = 0;
  }
}