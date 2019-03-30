#include <SimpleTimer.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Utility\Socket.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
static byte mac[] = {
  0x90, 0xA2, 0xDA, 0x0F, 0xD8, 0xBE
};
static IPAddress g_MyIPAddress(192,168,0,138);
static byte g_TargetMacAddress[] = {0x10,0x78,0xD2,0xDB,0x77,0x3A};

unsigned int localPort = 8888;       // local port to listen for UDP packets

char timeServer[] = "time.ewha.or.kr"; // time.ewha.or.kr NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

unsigned long timeTick;

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

SimpleTimer timer;

unsigned int year;
unsigned int month;
unsigned int date;
unsigned int hour;
unsigned int minute;
unsigned int second;
//char cl_str[100];

void cl_time() {
    second++;
    if(second > 59)
    {
      second = 0;
      minute++;
      if(minute > 59)
      {
        minute = 0;
        hour++;
        if(hour > 23)
        {
          hour = 0;
        }
      }
    }
    if(hour == 6 && minute == 0 && second < 10)
    {
      SendWOLMagicPacket(g_TargetMacAddress);
    }
    //sprintf(cl_str, "Current Time: %02u:%02u:%02u", hour, minute, second);
    //Serial.println(cl_str);
}

void getNTPtime()
{
  //char str[100];
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  timeTick = millis();

  while(true)
  {
    if(millis() - timeTick >= 1000)
      break;
  }
  
  // wait to see if a reply is available
  if (Udp.parsePacket()) {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    const unsigned long KST_time = 32400UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears + KST_time;
    // print Unix time:
    //Serial.println(epoch);

    // print the hour, minute and second:
     year   = (epoch / 31556926UL) + 1970;
     month  = (epoch % 31556926UL) / 2629743UL + 1;
     date   = (epoch % 2629743UL) / 86400UL + 1;
     hour   = (epoch % 86400UL) / 3600;
     minute = (epoch % 3600) / 60;
     second =  epoch % 60;
   
     //sprintf(str, "%04u/%02u/%02u %02u:%02u:%02u\r\n", year, month, date, hour, minute, second);
     //Serial.print(str);
    
  }
  // wait ten seconds before asking for the time again
  Ethernet.maintain();
}

void setup() {
  /*
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  */


  // start Ethernet and UDP
  Ethernet.begin(mac, g_MyIPAddress);

  Udp.begin(localPort);

  getNTPtime();
  
  timer.setInterval(1000, cl_time);  // 1초마다 repeatSec 함수 호출
  timer.setInterval(1800000, getNTPtime);  // 30분마다 getNTPtime 함수 호출
}

void loop() {
  timer.run();
}

// send an NTP request to the time server at the given address
void sendNTPpacket(char* address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void SendWOLMagicPacket(byte * pMacAddress)
{
  // The magic packet data sent to wake the remote machine. Target machine's
  // MAC address will be composited in here.
  const int nMagicPacketLength = 102;
  byte abyMagicPacket[nMagicPacketLength] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  byte abyTargetIPAddress[] = { 255, 255, 255, 255 }; // don't seem to need a real ip address.
  const int nWOLPort = 7;
  const int nLocalPort = 8888; // to "listen" on (only needed to initialize udp)

  
  // Compose magic packet to wake remote machine. 
  for (int ix=6; ix<102; ix++)
    abyMagicPacket[ix]=pMacAddress[ix%6];

  /*
  if (UDP_RawSendto(abyMagicPacket, nMagicPacketLength, nLocalPort, 
  abyTargetIPAddress, nWOLPort) != nMagicPacketLength)
    Serial.println("Error sending WOL packet");
  */
}

int UDP_RawSendto(byte* pDataPacket, int nPacketLength, int nLocalPort, byte* pRemoteIP, int nRemotePort)
{
  int nResult;
  int nSocketId; // Socket ID for Wiz5100

  // Find a free socket id.
  nSocketId = MAX_SOCK_NUM;
  for (int i = 0; i < MAX_SOCK_NUM; i++) 
  {
    uint8_t s = W5100.readSnSR(i);
    if (s == SnSR::CLOSED || s == SnSR::FIN_WAIT) 
    {
      nSocketId = i;
      break;
    }
  }

  if (nSocketId == MAX_SOCK_NUM)
    return 0; // couldn't find one. 

  if (socket(nSocketId, SnMR::UDP, nLocalPort, 0))
  {
    nResult = sendto(nSocketId,(unsigned char*)pDataPacket,nPacketLength,(unsigned char*)pRemoteIP,nRemotePort);
    close(nSocketId);
  } else
    nResult = 0;

  return nResult;
}
