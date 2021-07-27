#pragma once
#ifdef __cplusplus
extern "C"{
#endif
#include <stdint.h>
#include <stdbool.h>
#define MAX_CB_PER_BUTTON 10
#define BUTTON_ERR 1
#define BUTTON_OK 0
typedef struct 
{
  uint8_t click_count;
  uint8_t is_hold;
} button_result_t;
typedef struct 
{
  uint16_t pin;
  uint8_t active_logic;
  button_result_t trigger_result[MAX_CB_PER_BUTTON]; // if tracking var match trigger result, callback will be called
  void (*callback[MAX_CB_PER_BUTTON])(void);
  uint8_t callback_length;
  int  (*read_fp)(uint8_t); // if user want to use faster read than digitalRead function of Arduino
} button_register_t;

void button_initialize(uint32_t idle_tick, uint32_t hold_tick);
uint8_t button_register(button_register_t register_button, button_result_t **tracking_var);
void button_update(); //call periodically

#ifdef __cplusplus
}
#endif