SerialLinkExample0 |
                   |-> This example is stable and works one way. (In this example ESP32 to ATMEGA2560)
SerialLinkExample1 |



SerialLink2WayExample0 |
                       |-> This example is stable and works both ways. (In this example between ESP32 and ATMEGA2560)
SerialLink2WayExample1 |



Experiment: Reduce IO used for display(s) and HIDs on ESP32 while also gaining screen real estate and HIDs on ESP32:
I am experimenting in performance and methods with having an ATMEGA2560 as a panel controller for a
3.5" IPS TFT LCD Touchscreen Display (ILI9486), to be controlled by another microcontroller, in this case ESP32.
