A reference for a central MCU running SerialLink and a reference for a  peripheral MCU running SerialLink,
ready be built upon for custom projects.

The next improvement is intended to be the addition of soft serial and a single int to specify which pin/serial will
be used in readRXD()/writeTXD() regardless of weather hardware serial of software serial is used, like readRXD(int)
