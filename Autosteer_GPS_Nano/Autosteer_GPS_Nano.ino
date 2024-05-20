  /*  UDP Autosteer GPS code For Arduino
   *  20/05/24
   *  Desmartins Daniel
   */
#define VERSION 0.31
//#define PANDA //Use PANDA ? expérimental code... Not working yet!!

#include <Wire.h>
#include "EtherCard_AOG.h"
#include <IPAddress.h>

/************************* User Settings *************************/
const int32_t baudGPS = 115200;

#define ImuWire Wire
#define RAD_TO_DEG_X_10 572.95779513082320876798154814105

//#define baseLineLimit 5       //Max CM differance in baseline

#define REPORT_INTERVAL 20    //BNO report time, we want to keep reading it quick & offen. Its not timmed to anything just give constant data.
uint32_t READ_BNO_TIME = 0;   //Used stop BNO data pile up (This version is without resetting BNO everytime)

/*****************************************************************/

// Ethernet Options
struct ConfigIP {
    uint8_t ipOne = 192;
    uint8_t ipTwo = 168;
    uint8_t ipThree = 5;
};  ConfigIP networkAddress;   //3 bytes

//ethercard 10,11,12,13  
// Arduino Nano = 10 depending how CS of Ethernet Controller ENC28J60 is Connected
#define CS_Pin 10

// IP & MAC address of this module of this module
static uint8_t myip[] = { 0,0,0,126 };
static uint8_t gwip[] = { 0,0,0,1 };
static uint8_t myDNS[] = { 8,8,8,8 };
static uint8_t mask[] = { 255,255,255,0 };
static uint8_t ipDestination[] = {0,0,0, 255};
static uint8_t mymac[] = {0x00, 0x00, 0x56, 0x00, 0x00, 0x78};

const uint16_t portMy = 5126;             // port of this module
const uint16_t AOGNtripPort = 2233;       // port NTRIP data from AOG comes in
const uint16_t AOGAutoSteerPort = 8888;   // port Autosteer data from AOG comes in
const uint16_t portDestination = 9999;    // Port of AOG that listens

uint8_t Ethernet::buffer[256]; // udp send and receive buffer

#include "zNMEAParser.h"
#include "BNO08x_AOG.h"

// booleans to see if we are BNO08x
bool useBNO08x = false;

// BNO08x address variables to check where it is
const uint8_t bno08xAddresses[] = { 0x4A, 0x4B };
const int16_t nrBNO08xAdresses = sizeof(bno08xAddresses) / sizeof(bno08xAddresses[0]);
uint8_t bno08xAddress;
BNO080 bno08x;

/* A parser is declared with 3 handlers at most */
#ifdef PANDA
NMEAParser<2> parser;
#else
String nmea = "";

int16_t bno08xHeading10x = 0;
int16_t bno08xRoll10x = 0;
#endif

float roll = 0;
float pitch = 0;
float yaw = 0;

// Setup procedure ------------------------
void setup()
{
  delay(500);                         //Small delay so serial can monitor start up

  #ifdef PANDA
  // the dash means wildcard
  parser.setErrorHandler(errorHandler);
  parser.addHandler("G-GGA", GGA_Handler);
  parser.addHandler("G-VTG", VTG_Handler);
  #endif

  delay(10);
  Serial.begin(baudGPS);
  Serial.println(F(""));
  Serial.println(F("Firmware : Autosteer GPS Nano"));
  Serial.print(F("Version : "));
  Serial.println(VERSION);

  autosteerSetup();
  
  uint8_t error;

  ImuWire.begin();
  
  for (int16_t i = 0; i < nrBNO08xAdresses; i++)
  {
      bno08xAddress = bno08xAddresses[i];
      ImuWire.beginTransmission(bno08xAddress);
      error = ImuWire.endTransmission();

      if (error == 0)
      {
          // Initialize BNO080 lib
          if (bno08x.begin(bno08xAddress, ImuWire)) //??? Passing NULL to non pointer argument, remove maybe ???
          {
              //Increase I2C data rate to 400kHz
              ImuWire.setClock(400000); 

              delay(300);

              // Use gameRotationVector and set REPORT_INTERVAL
              bno08x.enableGameRotationVector(REPORT_INTERVAL);
              useBNO08x = true;
          }
      }
      if (useBNO08x) break;
  }

  delay(100);
  
  ether.begin(sizeof Ethernet::buffer, mymac, CS_Pin);

  //grab the ip from EEPROM
  myip[0] = networkAddress.ipOne;
  myip[1] = networkAddress.ipTwo;
  myip[2] = networkAddress.ipThree;

  gwip[0] = networkAddress.ipOne;
  gwip[1] = networkAddress.ipTwo;
  gwip[2] = networkAddress.ipThree;

  ipDestination[0] = networkAddress.ipOne;
  ipDestination[1] = networkAddress.ipTwo;
  ipDestination[2] = networkAddress.ipThree;

  //set up connection
  ether.staticSetup(myip, gwip, myDNS, mask);

  ether.udpServerListenOnPort(&udpNtrip, AOGNtripPort);
  ether.udpServerListenOnPort(&udpSteerRecv, AOGAutoSteerPort);
}

void loop()
{    
    ether.packetLoop(ether.packetReceive());

    // Pass NTRIP etc to GPS
    // Read incoming nmea from GPS
    #ifdef PANDA
    if (Serial.available())
    {
        parser << Serial.read();
    }

    //Read BNO
    if((millis() - READ_BNO_TIME) > REPORT_INTERVAL && useBNO08x)
    {
      READ_BNO_TIME = millis();
      readBNO();
    }
    #else //PANDA
    while (Serial.available() > 41) {
      nmea += Serial.readStringUntil('\n'); 
        
      if (nmea.length() > 0) {
        ether.sendUdp(nmea.c_str(), nmea.length(), portMy, ipDestination, portDestination);
        nmea = "";
      }
    }
    #endif //PANDA
    
    autosteerLoop();
}//End Loop
//**************************************************************************
