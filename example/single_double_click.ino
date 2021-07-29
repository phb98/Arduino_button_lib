#include "button_lib.h"
#define BUTTON_PIN 9
#define LED_PIN    13
void single_click();
void double_click();
void hold();
button_result_t * tracking_var;
void setup()  
{
  button_initialize(10,50); // button has to be idle for 10 tick to be considered idle, hold for 50 tick to be considered holding
  // if 1 tick = 20ms, this mean 200ms and 1000ms
  // in this example timer 2 is setup 1 tick equal 16.5ms
  /* create a struct to register our button */
  static button_register_t button;
  button.pin = BUTTON_PIN;
  button.active_logic = 0;

  /* setup a trigger result for single click */
  button.trigger_result[0].click_count = 1;
  button.callback[0] = single_click;

  /* setup a trigger result for double click */
  button.trigger_result[1].click_count = 2; // if you want triple click, change to 3, 4 click change to 4
  button.trigger_result[1].is_hold     = 0; // if you want hold, change to 1 (HOLD DOES NOT COUNT AS CLICK)
  button.callback[1] = double_click;

  /* we have 2 callback */
  button.callback_length = 2;

  /* register this button */
  button_register(button, &tracking_var);

  /* Setup gpio for our button and led */
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  /* Setup timer 2 for polling button */
  TCCR2A = 0;
  TCCR2B = 0b111; // 1024 PRESCALER
  TIMSK2 = (1<<TOIE2);
  Serial.begin(115200);
}
void loop()
{
  // do nothing since all has been handled internally
  Serial.print("Button click count:"); Serial.println(tracking_var->click_count);
  Serial.print("Button is holding:");  Serial.println(tracking_var->is_hold);
  delay(100);
}
ISR(TIMER2_OVF_vect)
{
  button_update();
}
void single_click()
{
  digitalWrite(LED_PIN, 1);
}
void double_click()
{
  digitalWrite(LED_PIN, 0);
}

