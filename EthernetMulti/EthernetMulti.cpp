/*
modified 13 Aug 2024
 by Marco Afonso (marco@baconhealthcare.com)
 This version suppress several features that we won't be using, plus uses instances of ws5500 class.
 
 modified 12 Aug 2013
 by Soohwan Kim (suhwan@wiznet.co.kr)

- 10 Apr. 2015
 Added support for Arduino Ethernet Shield 2
 by Arduino.org team

 */
 
#include "EthernetMulti.h"

EthernetClass::EthernetClass()
{
  for(int i = 0; i < MAX_SOCK_NUM; i++)
  {
    _state[i] = 0;
    _server_port[i] = 0;
  }
}


void EthernetClass::begin(uint8_t *mac, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
{
  _w5500.setMACAddress(mac);
  _w5500.setIPAddress(local_ip.raw_address());
  _w5500.setGatewayIp(gateway.raw_address());
  _w5500.setSubnetMask(subnet.raw_address());
  _dnsServerAddress = dns_server;
}

IPAddress EthernetClass::localIP()
{
  IPAddress ret;
  _w5500.getIPAddress(ret.raw_address());
  return ret;
}

IPAddress EthernetClass::subnetMask()
{
  IPAddress ret;
  _w5500.getSubnetMask(ret.raw_address());
  return ret;
}

IPAddress EthernetClass::gatewayIP()
{
  IPAddress ret;
  _w5500.getGatewayIp(ret.raw_address());
  return ret;
}

IPAddress EthernetClass::dnsServerIP()
{
  return _dnsServerAddress;
}

char* EthernetClass::dnsDomainName(){
    return _dnsDomainName;
}

char* EthernetClass::hostName(){
    return _hostName;
}

EthernetClass Ethernet;
