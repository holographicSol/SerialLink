SerialLinkExample0 |
                   |-> This example is stable and works one way. (In this example ESP32 to ATMEGA2560)
SerialLinkExample1 |



SerialLink2WayExample0 |
                       |-> This example is stable and works both ways. (In this example between ESP32 and ATMEGA2560)
SerialLink2WayExample1 |


Problem using many cheap displays directly on the ESP32:
	Not enough pixels to display required data.
	If multiplexing multiple displays then we use a lot on IO.
	Further IO may still also be required for interface/control.


Experiment:
	Reduce IO used for display(s) and HIDs on ESP32 while also GAINING screen real estate and HIDs on ESP32.
	Make an ATMEGA2560 + ILI9486 sandwich to be controlled by the ESP32.
	Method use between 1-2 PINs on the ESP32:
	1 Wire example: Control a panel with one wire.
	2 Wire example: Control a panel with one wire, send touchscreen data (x,y,z) back to ESP32 with the second wire.
	Touchscreen X,Y,Z -> ESP32
	ATMEGA2560        <- Commands/Information From ESP32


The Work:
	The setup has to be stable and so the two 'methods' so far for sending and receiving over serial have been carefully,
	tediously and painfully constructed to ensure that data is received from start to end, in order, intact and without corruption.
	Care must still be taken to avoid corruption:
		Dumping other junk on the same line(s).
		Interference.
		Dodgy wiring.


The Output (inspired by NMEA sentences, simple and effective):
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


Summary So Far:
	More input.
	More screen space for information/graphics (I am interested in information).
	Only 2 PINs are used on ESP32.
	And its fast! (calculate graphics on the ATMEGA itself, the serial to ATMEGA should command and populate data to be displayed).
	A Touchscreen TFT hat on an ATMEGA2560 with aa ESP32 under the hood for compute with WiFi, Bluetooth, all available IO and a kick ass clock (for 10 bucks).