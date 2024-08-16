#include <EthernetMulti.h>

byte mac1[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
byte mac2[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x03 };
IPAddress ip1(10, 19, 45, 1);
IPAddress ip2(10, 19, 46, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetClass eth1;
EthernetClass eth2;

EthernetServer server1(&eth1, 8088);
EthernetServer server2(&eth2, 8088);

boolean gotAMessage1 = false; // whether or not you got a message from the client yet
boolean gotAMessage2 = false; // whether or not you got a message from the client yet

void setup() {
  Serial.begin(9600);
  delay(500);
  
  eth1.init(33);
  eth2.init(27);

  eth1.begin(mac1, ip1, ip1, ip1, subnet);
  eth2.begin(mac2, ip2, ip2, ip2, subnet);

  Serial.print("eth1 ip "); Serial.println(eth1.localIP());
  Serial.print("eth1 subnet "); Serial.println (eth1.subnetMask ()); 
  Serial.print("eth1 gateway "); Serial.println (eth1.gatewayIP ());
  Serial.print("eth1 dnsServerIP "); Serial.println (eth1.dnsServerIP ());
  Serial.print("eth2 ip "); Serial.println(eth2.localIP());
  Serial.print("eth2 subnet "); Serial.println (eth2.subnetMask ()); 
  Serial.print("eth2 gateway "); Serial.println (eth2.gatewayIP ());
  Serial.print("eth2 dnsServerIP "); Serial.println (eth2.dnsServerIP ());

    // start listening for clients
  server1.begin();
  server2.begin();
}

void loop() {
return;
  EthernetClient client1 = server1.available();

  // when the client sends the first byte, say hello:
  if (client1) {
    if (!gotAMessage1) {
      Serial.println("We have a new client 1");
      client1.println("Hello, client 1!");
      gotAMessage1 = true;
    }

    // read the bytes incoming from the client:
    while(true)
    {
      char thisChar = client1.read();
      if(thisChar < 0) break;
      // echo the bytes back to the client:
      client1.write(thisChar);
      // echo the bytes to the server as well:
      //Serial.print(thisChar);
    }
  }


  EthernetClient client2 = server2.available();

  // when the client sends the first byte, say hello:
  if (client2) {
    if (!gotAMessage2) {
      Serial.println("We have a new client 2");
      client2.println("Hello, client 2!");
      gotAMessage2 = true;
    }

    // read the bytes incoming from the client:
    char thisChar = client2.read();
    // echo the bytes back to the client:
    client2.write(thisChar);
    // echo the bytes to the server as well:
    //Serial.print(thisChar);
  }
}
