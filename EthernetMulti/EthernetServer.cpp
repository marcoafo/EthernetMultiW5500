#include "w5500.h"
#include "socket.h"
extern "C" {
#include "string.h"
}

#include "EthernetMulti.h"
#include "EthernetClient.h"
#include "EthernetServer.h"

EthernetServer::EthernetServer(EthernetClass* ethernet, uint16_t port)
{
  _ethernet = ethernet;
  _port = port;
}

// perhaps this could return true or false in case it didn't find a usable socket?
void EthernetServer::begin()
{
  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    EthernetClient client(_ethernet, sock);
    if (client.status() == SnSR::CLOSED) {
      socket(&_ethernet->_w5500, sock, SnMR::TCP, _port, 0);
      listen(&_ethernet->_w5500, sock);
      _ethernet->_server_port[sock] = _port;
      //char buf[256]; sprintf(buf, "attached at socket %d port %d for CS %d", sock, _port, _ethernet->_w5500._SPI_CS); Serial.println(String(buf));
      break;
    }
  }
}

void EthernetServer::accept()
{
  int listening = 0;

  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    EthernetClient client(_ethernet, sock);

    if (_ethernet->_server_port[sock] == _port) {
      if (client.status() == SnSR::LISTEN) {
        listening = 1;
      } 
      else if (client.status() == SnSR::CLOSE_WAIT && !client.available()) {
        client.stop();
      }
    } 
  }

  if (!listening) {
    begin();
  }
}

EthernetClient EthernetServer::available()
{
  accept();

  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    EthernetClient client(_ethernet, sock);
    if (_ethernet->_server_port[sock] == _port &&
        (client.status() == SnSR::ESTABLISHED ||
         client.status() == SnSR::CLOSE_WAIT)) {
      if (client.available()) {
        // XXX: don't always pick the lowest numbered socket.
        return client;
      }
    }
  }

  return EthernetClient(_ethernet, MAX_SOCK_NUM);
}

size_t EthernetServer::write(uint8_t b) 
{
  return write(&b, 1);
}

size_t EthernetServer::write(const uint8_t *buffer, size_t size) 
{
  size_t n = 0;
  
  accept();

  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    EthernetClient client(_ethernet, sock);

    if (_ethernet->_server_port[sock] == _port &&
      client.status() == SnSR::ESTABLISHED) {
      n += client.write(buffer, size);
    }
  }
  
  return n;
}
