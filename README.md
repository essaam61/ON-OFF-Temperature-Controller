Developed and configured a simple ON-OFF temperature controller application based on FreeRTOS Multitasking Kernel approach. It was applied on Arm Cortex M4 Tiva C EVB using FreeRTOS along with its techniques by Keil IDE.
An ON-OFF temperature controller, will turn OFF the heater/oven whenever the temperature is above a pre-defined setpoint  (i.e., the required value).. Similarly, if the temperature is below the setpoint then the heater is turned ON.

The setpoint temperature is entered using a keyboard connected to a PC. It can be changed at any desired time while the oven is under control. i.e. there is no need to stop the controller in order to change the setpoint value (hence multitasking). An LCD connected to the development board displays the setpoint temperature as well as the actual measured oven temperature. The temperature of the oven is measured using an analog temperature sensor chip. A buzzer is used which becomes active if the temperature of the oven goes above a dangerous preset value to indicate an alarm condition. A LED displays the state of the oven at any time such that when the oven is ON then the LED is ON, and vice-versa.


![Circuit](https://user-images.githubusercontent.com/109050863/197562182-a9f21b74-26ed-447c-84a6-5f70f76a6404.jpg)
