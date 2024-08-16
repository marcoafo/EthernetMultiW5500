#ifndef ethernetserver_h
#define ethernetserver_h

#include "Server.h"

class EthernetClient;
class EthernetClass;

class EthernetServer : 
public Server {
private:
  uint16_t _port;
  EthernetClass* _ethernet;
  void accept();
public:
  EthernetServer(EthernetClass* ethernet, uint16_t port);
  EthernetClient available();
  virtual void begin();
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  using Print::write;
};

#endif
