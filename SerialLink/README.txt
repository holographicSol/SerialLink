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
	Touchscreen X,Y,Z -> ESP32 (Example $TS,x,y,z... if z=1: x,y clicked -> function)
	ATMEGA2560        <- Commands/Information From ESP32 (Example $GFX,runaGFXsequence,..)


The Work:
	The setup has to be stable and so the two 'methods' so far for sending and receiving over serial have been carefully,
	tediously and painfully constructed to ensure that data is received from start to end, in order, intact and without corruption.
	Care must still be taken to avoid corruption:
		Dumping other junk on the same line(s).
		Interference.
		Dodgy wiring.
	So far this setup is extremely fast and can yeild extremely responsive touchscreen to function(s) over serial, with all ESP32 clock
	and IO still available, and while intending to be 99.999% reliable (it has to be reliable or the funs over, that is why this project
	is initially called SerialLink, which I can base implementations of a fast reliable serial link upon, as required).


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
	And its fast! (calculate graphics on the ATMEGA itself, the serial to ATMEGA should command and populate other data to be displayed).
	A Touchscreen TFT hat on an ATMEGA2560 with aa ESP32 under the hood for compute with WiFi, Bluetooth, all available IO and a kick ass
	clock (for 10 bucks).
	The microcontrollers can be the same or differnt using the provided methods (possibly other methods may be required but possibly with
	the same principles, tagged, comma delimited senences terminated with an ETX char).
	In these examples the ATMEGA2560 should be thought of as a peripheral and the ESP32 should be thought of as central. Serial multiplexing
	and things like PWM where suitable could expand on this.


Performance:
	Some of the examples use MCUFriend_kbv for driving the ILI9341 TFT Display, ensure the following:
		mcufriend_sheild.h: uncomment #include "mcufriend_shield.h"
		mcufriend_special: uncomment #define USE_BLD_BST_MEGA2560 (in my case, you may use something else.)
	This should about double the frame rate.
	On ESP32 SerialLink can TXD a print to ILI9486 hat on an MTMEGA2560 in 7 microseconds, however in the case of a ILI9486 hat on 
	an ATMEGA2560, an ATMEGA2560 may take a while to write to the ILI9486. This is the peripherals problem and SerialLink
	on an ESP32 with a TXD time to a peripheral of 7 microseconds is capable of providing over 140,000 instructions to any given peripheral
	a second, meaning that in the case of a slower peripheral, the slower peripheral can't make use of every instruction capable of beong
	sent, but that another peripheral could, which is fantastic news. I intend to complete a SerialLink for the ILI9341 hat on the
	ATMEGA2560, and do next intend to build a peripheral (perhaps another touch screen display on a microcontroller) that can make use
	more instructions per second from a central device.

Architectural Theory:
	Hardware architecture shape is inspired by general computer hardware layout and network architecture (in this case ring and center).
	Instructions between center and peripheral devices are inspired by NMEA sentences (being reasonably efficient and human friendly).
	The difference being that unlike general purpose computers, we have limited memory and other resouces so instructions between peripherals
	and center may be bespoke, which is as a benefit potentially more efficient than loading a million drivers just in case something is 
	plugged in that never will be.
	Accomplishing peripheral communication in 1-2 wires, freeing up more room for expansion/compute on a given 'central' controller.
