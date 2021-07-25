#define POLLING_MODE 1
#define BUTTON_HOLDING_INTERVAL
#define BUTTON_IDLE_INTERVAL
#include <stdint.h>
typedef enum 
{
  IDLE,
  SINGLE_PRESSED,
  DOUBLE_PRESSED,
  HOLDING,
} button_result_t;
typedef enum
{
  RELEASE = 0,
  PRESSED = 1,
} button_state_t;
typedef enum
{
  SM_IDLE,
  SM_BEGIN_PRESSING,
  SM_PREPARE_RELEASE,
  SM_RELEASE,
  SM_HOLDING,
  SM_PREPARE_IDLE,
} SM_t;
typedef struct
{
  SM_t current_SM = SM_IDLE;
  uint16_t count;
  uint8_t click_count;
  uint8_t has_been_hold;
} button_SM_t;

uint16_t button_arr[10];
button_state_t button_state_arr[10];
button_result_t* button_result_arr[10];
bool need_reading_button;
uint8_t button_count;
void init(uint16_t pin, button_result_t * result )
{
  /* User must set pinMode themselves */
  if(button_count < 10)
  {
    button_arr[button_count] = pin;
    button_result_arr[button_count] = result;
    button_count++;
  }
  else
  {
    return;
  }
}
void update()
{
  read_button();
  if(need_reading_button || button_changed_state(button_state_arr))
  {
    process();
  }
  return;
}
void read_button()
{
  for(int i=0; i < button_count; i++)
  {
    if(digitalRead(button_arr[i])) button_state_arr[i] = PRESSED;
    else button_state_arr[i] = RELEASE;
  }
  return;
}
bool button_changed_state(button_state_t * button_new_state)
{
  bool result = 0;
  static button_state_t button_old_state[10];
  for(int i=0; i < button_count; i++)
  {
    if(button_old_state[i] != button_new_state[i])
    {
      button_old_state[i] = button_new_state[i];
      result = 1;
    } 
  }
  return result;
}
void process()
{
  need_reading_button = 1;
  static button_SM_t button_SM[10];
  for(int i = 0; i < button_count; i++)
  {
    switch(button_SM[i].current_SM)
    {
      case SM_IDLE:
        if(button_state_arr[i] == PRESSED)
        {
          button_SM[i].current_SM = SM_BEGIN_PRESSING;
          button_SM[i].click_count = 0;
          button_SM[i].has_been_hold = 0;
          button_SM[i].count = 0; 
        }
        else need_reading_button = 0;
        break;
      case SM_BEGIN_PRESSING:
        if(button_state_arr[i] == RELEASE)
        {
          button_SM[i].count = 0;
          button_SM[i].current_SM = SM_PREPARE_RELEASE;
        }
        if(button_SM[i].count > BUTTON_HOLDING_INTERVAL)
        {
          button_SM[i].count = 0;
          button_SM[i].current_SM = SM_HOLDING;
        }
        button_SM[i].count++;
        break;
      case SM_PREPARE_RELEASE:
        button_SM[i].click_count++;
        button_SM[i].count = 0;
        button_SM[i].current_SM = SM_RELEASE;
        break;
      case SM_RELEASE:
        if(button_state_arr[i] == PRESSED)
        {
          button_SM[i].count = 0;
          button_SM[i].current_SM = SM_BEGIN_PRESSING;
        }
        else if(button_SM[i].count > BUTTON_IDLE_INTERVAL)
        {
          button_SM[i].count = 0;
          button_SM[i].current_SM = SM_PREPARE_IDLE;
        }
        button_SM[i].count++;
        break;
      case SM_HOLDING:
        button_SM[i].has_been_hold = 1;
        break;

  }
}