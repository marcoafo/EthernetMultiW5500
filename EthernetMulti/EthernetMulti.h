/*
 modified 12 Aug 2013
 by Soohwan Kim (suhwan@wiznet.co.kr)

 - 10 Apr. 2015
 Added support for Arduino Ethernet Shield 2
 by Arduino.org team
 
 */
#ifndef ethernet_h
#define ethernet_h

#include <inttypes.h>
#include "w5500.h"
#include "IPAddress.h"
#include "EthernetClient.h"
#include "EthernetServer.h"



class EthernetClass {
private:
  IPAddress _dnsServerAddress;
  char* _dnsDomainName;
  char* _hostName;

  uint8_t _state[MAX_SOCK_NUM];
  uint16_t _server_port[MAX_SOCK_NUM];

  W5500Class _w5500;
public:
  EthernetClass();

  void init(uint8_t cspin = 33) { _w5500.init(cspin); }
  
  void begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);

  IPAddress localIP();
  IPAddress subnetMask();
  IPAddress gatewayIP();
  IPAddress dnsServerIP();
  char* dnsDomainName();
  char* hostName();

  friend class EthernetClient;
  friend class EthernetServer;
  friend class EthernetUDP;
};

// extern EthernetClass Ethernet;  // needs instance

#endif
