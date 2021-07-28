#include "button_lib.h"
#define BUTTON_PIN 9
#define LED_PIN    13
void single_click();
void double_click();
button_result_t * tracking_var;
void setup()  
{
  button_initialize(10,50); // button has to be idle 10 tick to be considered idle, hold for 50 tick to be considered holding
  // if 1 tick = 20ms, this mean 200ms and 1000ms
  static button_register_t button;
  button.pin = BUTTON_PIN;
  button.active_logic = 0;
  /* setup a trigger result for single click */
  button.trigger_result[0].click_count = 1;
  button.callback[0] = single_click;
  /* setup a trigger result for double click */
  button.trigger_result[1].click_count = 2;
  button.callback[1] = double_click;
  /* we have 2 callback */
  button.callback_length = 2;
  /* register this button */
  button_register(button, &tracking_var);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
}
void loop()
{
  // call button_update every 20ms
  static uint32_t last_millis;
  if((millis() - last_millis) >= 20) {
    button_update();
    last_millis = millis();
  }
}
void single_click()
{
  digitalWrite(LED_PIN, 1);
}
void double_click()
{
  digitalWrite(LED_PIN, 0);
}
