# EthernetMultiW5500

A library for Arduino/ESP32 that allows multi WIZnet W5500 to work together, using different CS pins.

I haven't fully tested it, only with an ESP32 connected with two W5500 (the example is here as well). It's a modification of the library provided by several other creators that you can find in the Arduino library.

It's not configured to be a library and other things because I'm lazy. Feel free to use and modify how you want. But the hack way is to put the EthernetMulti folder inside your libraries folder, then use as in the example (just add an include to *EthernetMulti.h*).

Basically, you need to create an instance of the EthernetClass class for each W5500 (determine the CS pin in the constructor), and reference each object to an instance of EthernetServer or EthernetClient.

One thing that might need work is that I couldn't test to disable the multicasting. So ARP calls to announce the IP are ignored, since it's expecting something else in IGPM protocol. Some HMI might have problem finding it with fixed IP, so you could use a DHCP server.
