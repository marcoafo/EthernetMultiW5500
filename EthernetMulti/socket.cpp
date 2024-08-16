 /*
 * - 13 Aug. 2024
 * Uses instances of w5500 classes for multiple adapters.
 * by Marco Afonso (marco@baconhealthcare.com)
 * 
 * - 10 Apr. 2015
 * Added support for Arduino Ethernet Shield 2
 * by Arduino.org team
 */
 
#include "w5500.h"
#include "socket.h"

static uint16_t local_port;

/**
 * @brief	This Socket function initialize the channel in perticular mode, and set the port and wait for w5500 done it.
 * @return 	1 for success else 0.
 */
uint8_t socket(W5500Class* ws, SOCKET s, uint8_t protocol, uint16_t port, uint8_t flag)
{
  if ((protocol == SnMR::TCP) || (protocol == SnMR::UDP) || (protocol == SnMR::IPRAW) || (protocol == SnMR::MACRAW) || (protocol == SnMR::PPPOE))
  {
    close(ws, s);
    ws->writeSnMR(s, protocol | flag);
    if (port != 0) {
      ws->writeSnPORT(s, port);
    } 
    else {
      local_port++; // if don't set the source port, set local_port number.
      ws->writeSnPORT(s, local_port);
    }

    ws->execCmdSn(s, Sock_OPEN);
    
    return 1;
  }

  return 0;
}


/**
 * @brief	This function close the socket and parameter is "s" which represent the socket number
 */
void close(W5500Class* ws, SOCKET s)
{
  ws->execCmdSn(s, Sock_CLOSE);
  ws->writeSnIR(s, 0xFF);
}


/**
 * @brief	This function established  the connection for the channel in passive (server) mode. This function waits for the request from the peer.
 * @return	1 for success else 0.
 */
uint8_t listen(W5500Class* ws, SOCKET s)
{
  if (ws->readSnSR(s) != SnSR::INIT)
    return 0;
  ws->execCmdSn(s, Sock_LISTEN);
  return 1;
}


/**
 * @brief	This function established  the connection for the channel in Active (client) mode. 
 * 		This function waits for the untill the connection is established.
 * 		
 * @return	1 for success else 0.
 */
uint8_t connect(W5500Class* ws, SOCKET s, uint8_t * addr, uint16_t port)
{
  if 
    (
  ((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
    ((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
    (port == 0x00) 
    ) 
    return 0;

  // set destination IP
  ws->writeSnDIPR(s, addr);
  ws->writeSnDPORT(s, port);
  ws->execCmdSn(s, Sock_CONNECT);

  return 1;
}



/**
 * @brief	This function used for disconnect the socket and parameter is "s" which represent the socket number
 * @return	1 for success else 0.
 */
void disconnect(W5500Class* ws, SOCKET s)
{
  ws->execCmdSn(s, Sock_DISCON);
}


/**
 * @brief	This function used to send the data in TCP mode
 * @return	1 for success else 0.
 */
uint16_t send(W5500Class* ws, SOCKET s, const uint8_t * buf, uint16_t len)
{
  uint8_t status=0;
  uint16_t ret=0;
  uint16_t freesize=0;

  if (len > ws->SSIZE) 
    ret = ws->SSIZE; // check size not to exceed MAX size.
  else 
    ret = len;

  // if freebuf is available, start.
  do 
  {
    freesize = ws->getTXFreeSize(s);
    status = ws->readSnSR(s);
    if ((status != SnSR::ESTABLISHED) && (status != SnSR::CLOSE_WAIT))
    {
      ret = 0; 
      break;
    }
  } 
  while (freesize < ret);

  // copy data
  ws->send_data_processing(s, (uint8_t *)buf, ret);
  ws->execCmdSn(s, Sock_SEND);

  /* +2008.01 bj */
  while ( (ws->readSnIR(s) & SnIR::SEND_OK) != SnIR::SEND_OK ) 
  {
    /* m2008.01 [bj] : reduce code */
    if ( ws->readSnSR(s) == SnSR::CLOSED )
    {
      close(s);
      return 0;
    }
  }
  /* +2008.01 bj */
  ws->writeSnIR(s, SnIR::SEND_OK);
  return ret;
}


/**
 * @brief	This function is an application I/F function which is used to receive the data in TCP mode.
 * 		It continues to wait for data as much as the application wants to receive.
 * 		
 * @return	received data size for success else -1.
 */
int16_t recv(W5500Class* ws, SOCKET s, uint8_t *buf, int16_t len)
{
  // Check how much data is available
  int16_t ret = ws->getRXReceivedSize(s);
  if ( ret == 0 )
  {
    // No data available.
    uint8_t status = ws->readSnSR(s);
    if ( status == SnSR::LISTEN || status == SnSR::CLOSED || status == SnSR::CLOSE_WAIT )
    {
      // The remote end has closed its side of the connection, so this is the eof state
      ret = 0;
    }
    else
    {
      // The connection is still up, but there's no data waiting to be read
      ret = -1;
    }
  }
  else if (ret > len)
  {
    ret = len;
  }

  if ( ret > 0 )
  {
    ws->recv_data_processing(s, buf, ret);
    ws->execCmdSn(s, Sock_RECV);
  }
  return ret;
}


/**
 * @brief	Returns the first byte in the receive queue (no checking)
 * 		
 * @return
 */
uint16_t peek(W5500Class* ws, SOCKET s, uint8_t *buf)
{
  ws->recv_data_processing(s, buf, 1, 1);

  return 1;
}


/**
 * @brief	This function is an application I/F function which is used to send the data for other then TCP mode. 
 * 		Unlike TCP transmission, The peer's destination address and the port is needed.
 * 		
 * @return	This function return send data size for success else -1.
 */
uint16_t sendto(W5500Class* ws, SOCKET s, const uint8_t *buf, uint16_t len, uint8_t *addr, uint16_t port)
{
  uint16_t ret=0;

  if (len > ws->SSIZE) ret = ws->SSIZE; // check size not to exceed MAX size.
  else ret = len;

  if
    (
  ((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
    ((port == 0x00)) ||(ret == 0)
    ) 
  {
    /* +2008.01 [bj] : added return value */
    ret = 0;
  }
  else
  {
    ws->writeSnDIPR(s, addr);
    ws->writeSnDPORT(s, port);

    // copy data
    ws->send_data_processing(s, (uint8_t *)buf, ret);
    ws->execCmdSn(s, Sock_SEND);

    /* +2008.01 bj */
    while ( (ws->readSnIR(s) & SnIR::SEND_OK) != SnIR::SEND_OK ) 
    {
      if (ws->readSnIR(s) & SnIR::TIMEOUT)
      {
        /* +2008.01 [bj]: clear interrupt */
        ws->writeSnIR(s, (SnIR::SEND_OK | SnIR::TIMEOUT)); /* clear SEND_OK & TIMEOUT */
        return 0;
      }
    }

    /* +2008.01 bj */
    ws->writeSnIR(s, SnIR::SEND_OK);
  }
  return ret;
}


/**
 * @brief	This function is an application I/F function which is used to receive the data in other then
 * 	TCP mode. This function is used to receive UDP, IP_RAW and MAC_RAW mode, and handle the header as well. 
 * 	
 * @return	This function return received data size for success else -1.
 */
uint16_t recvfrom(W5500Class* ws, SOCKET s, uint8_t *buf, uint16_t len, uint8_t *addr, uint16_t *port)
{
  uint8_t head[8];
  uint16_t data_len=0;
  uint16_t ptr=0;

  if ( len > 0 )
  {
    ptr = ws->readSnRX_RD(s);
    switch (ws->readSnMR(s) & 0x07)
    {
    case SnMR::UDP :
      ws->read_data(s, ptr, head, 0x08);
      ptr += 8;
      // read peer's IP address, port number.
      addr[0] = head[0];
      addr[1] = head[1];
      addr[2] = head[2];
      addr[3] = head[3];
      *port = head[4];
      *port = (*port << 8) + head[5];
      data_len = head[6];
      data_len = (data_len << 8) + head[7];

      ws->read_data(s, ptr, buf, data_len); // data copy.
      ptr += data_len;

      ws->writeSnRX_RD(s, ptr);
      break;

    case SnMR::IPRAW :
      ws->read_data(s, ptr, head, 0x06);
      ptr += 6;

      addr[0] = head[0];
      addr[1] = head[1];
      addr[2] = head[2];
      addr[3] = head[3];
      data_len = head[4];
      data_len = (data_len << 8) + head[5];

      ws->read_data(s, ptr, buf, data_len); // data copy.
      ptr += data_len;

      ws->writeSnRX_RD(s, ptr);
      break;

    case SnMR::MACRAW:
      ws->read_data(s, ptr, head, 2);
      ptr+=2;
      data_len = head[0];
      data_len = (data_len<<8) + head[1] - 2;

      ws->read_data(s, ptr, buf, data_len);
      ptr += data_len;
      ws->writeSnRX_RD(s, ptr);
      break;

    default :
      break;
    }
    ws->execCmdSn(s, Sock_RECV);
  }
  return data_len;
}

/**
 * @brief	Wait for buffered transmission to complete.
 */
void flush(W5500Class* ws, SOCKET s) {
  // TODO
}

uint16_t igmpsend(W5500Class* ws, SOCKET s, const uint8_t * buf, uint16_t len)
{
  uint8_t status=0;
  uint16_t ret=0;

  if (len > ws->SSIZE) 
    ret = ws->SSIZE; // check size not to exceed MAX size.
  else 
    ret = len;

  if (ret == 0)
    return 0;

  ws->send_data_processing(s, (uint8_t *)buf, ret);
  ws->execCmdSn(s, Sock_SEND);

  while ( (ws->readSnIR(s) & SnIR::SEND_OK) != SnIR::SEND_OK ) 
  {
    status = ws->readSnSR(s);
    if (ws->readSnIR(s) & SnIR::TIMEOUT)
    {
      /* in case of igmp, if send fails, then socket closed */
      /* if you want change, remove this code. */
      close(s);
      return 0;
    }
  }

  ws->writeSnIR(s, SnIR::SEND_OK);
  return ret;
}

uint16_t bufferData(W5500Class* ws, SOCKET s, uint16_t offset, const uint8_t* buf, uint16_t len)
{
  uint16_t ret =0;
  if (len > ws->getTXFreeSize(s))
  {
    ret = ws->getTXFreeSize(s); // check size not to exceed MAX size.
  }
  else
  {
    ret = len;
  }
  ws->send_data_processing_offset(s, offset, buf, ret);
  return ret;
}

int startUDP(W5500Class* ws, SOCKET s, uint8_t* addr, uint16_t port)
{
  if
    (
     ((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
     ((port == 0x00))
    ) 
  {
    return 0;
  }
  else
  {
    ws->writeSnDIPR(s, addr);
    ws->writeSnDPORT(s, port);
    return 1;
  }
}

int sendUDP(W5500Class* ws, SOCKET s)
{
  ws->execCmdSn(s, Sock_SEND);
		
  /* +2008.01 bj */
  while ( (ws->readSnIR(s) & SnIR::SEND_OK) != SnIR::SEND_OK ) 
  {
    if (ws->readSnIR(s) & SnIR::TIMEOUT)
    {
      /* +2008.01 [bj]: clear interrupt */
      ws->writeSnIR(s, (SnIR::SEND_OK|SnIR::TIMEOUT));
      return 0;
    }
  }

  /* +2008.01 bj */	
  ws->writeSnIR(s, SnIR::SEND_OK);

  /* Sent ok */
  return 1;
}
