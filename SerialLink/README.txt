SerialLinkExample0 |
                   |-> This example is stable and works one way. (In this example ESP32 to ATMEGA2560)
SerialLinkExample1 |



SerialLink2WayExample0 |
                       |-> This example is stable and works both ways. (In this example between ESP32 and ATMEGA2560)
SerialLink2WayExample1 |



Experiment: Reduce IO used for display(s) and HIDs on ESP32 while also gaining screen real estate and HIDs on ESP32:
I am experimenting in performance and methods with having an ATMEGA2560 as a panel controller for a
3.5" IPS TFT LCD Touchscreen Display (ILI9486), to be controlled by another microcontroller, in this case ESP32.
This serial method used between 1-2 PINs on the ESP32:
1 Wire example: Control a panel with one wire.
2 Wire example: Control a panel with one wire, send touchscreen data (x,y,z) back to ESP32 with the second wire.



The setup has to be stable and so the two 'methods' so far for sending and receiving over serial have been carefully,
tediously and painfully constructed to ensure that data is received from start to end, in order and without corruption.
Care must still be taken to avoid corruption:
Dumping other junk on the same line(s).
Interference.
Dodgy wiring.
If care is taken in and around the setup then received data should be perfectly as expected every time like:


ATMEGA2560 Receives:
-------------------------------------------
[RXD]       $FOO,1,20,300,4000,OneWirePanelController,12345
[RXD TOKEN] $FOO
[RXD TOKEN] 1
[RXD TOKEN] 20
[RXD TOKEN] 300
[RXD TOKEN] 4000
[RXD TOKEN] OneWirePanelController
[RXD TOKEN] 12345


ESP32 Receives:
-------------------------------------------
[RXD]       $BAR,1,20,300,4000,OneWirePanelController,12345
[RXD TOKEN] $BAR
[RXD TOKEN] 1
[RXD TOKEN] 20
[RXD TOKEN] 300
[RXD TOKEN] 4000
[RXD TOKEN] OneWirePanelController
[RXD TOKEN] 12345
-------------------------------------------


Summary so far:
More input.
More screen space.
All but 2 PINs remain free on ESP32.
And its fast!