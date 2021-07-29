# Arduino_button_lib
This library helps user quickly handle buttons with different event. All this library needs is calling button_update() periodically, the rest will be handled by library
# Quick Start
Begin doing anything, you must first initialize this library by calling  <code>button_initialize</code>, you also need to pass the IDLE_TICK and HOLDING_TICK, button is considered IDLE if it's not been pressed for IDLE_TICK time and HOLD if it's pressed for HOLDING_TICK time  
## To register a button  
Create a <code>button_register_t</code> variable. In this struct, there are some varaible you need to set  
1. pin: The pin you connect your button to  
2. active_logic: The logic(1 or 0) when your button is active(Pressed)  
To add a callback(The function which will be called when there is event on button)  
3. Set <code>trigger_result.click_count</code> to the number of click you want ( 1 for single click, 2 for double click, 3 for triple click, etc)  
4. Set <code>trigger_result.is_hold</code> to 1 if you want to press then hold button( hold DOES NOT count as click, so if you want to PRESS->HOLD->RELEASE), set click count to 0 and is_hold to 1
5. Add your fuction you want to be called for that event to the callback 
6. Repeat from step 3 to add another callback for another event, right now maximum callback for each button is 3, but you can modify library for more callback, but it will uses more RAM
7. Register button by using <code>button_register</code>

Your program must call <code>button_update</code> periodically, the time between each call is also the time for 1 tick mentioned above
