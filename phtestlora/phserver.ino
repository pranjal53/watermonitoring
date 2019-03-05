/*
  Upload Data to IoT Server ThingSpeak (https://thingspeak.com/):
  Support Devices: LG01 
  
  Example sketch showing how to get data from remote LoRa node, 
  Then send the value to IoT Server

  It is designed to work with the other sketch dht11_client. 

  modified 24 11 2016
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <Console.h>
#include "ThingSpeak.h"
#include "YunClient.h"
YunClient client;
RH_RF95 rf95;

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

unsigned long myChannelNumber = 662568;
const char * myWriteAPIKey = "8FOTQQJVONAN2U7V";
const char * myReadAPIKey = "YPPSNUCGJWU1JHDY";
uint16_t crcdata = 0;
uint16_t recCRCData = 0;
float frequency = 868.0;

void setup()
{
    Bridge.begin(BAUDRATE);
    //Console.begin();// Don't use Console here, since it is conflict with the ThinkSpeak library. 

    ThingSpeak.begin(client);
    
    if (!rf95.init())
        //Console.println("init failed");
    ;
    // Setup ISM frequency
    rf95.setFrequency(frequency);
    // Setup Power,dBm
    rf95.setTxPower(13);
    
    //Console.println("Start Listening ");
}
void loop()
{

    if (rf95.waitAvailableTimeout(2000))// Listen Data from LoRa Node
    {
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//receive data buffer
        uint8_t len = sizeof(buf);//data buffer length
        if (rf95.recv(buf, &len))//Check if there is incoming data
        {
                //Console.println("Get Data from LoRa Node");
                if(buf[0] == 1||buf[1] == 1||buf[2] ==1) //Check if the ID match the LoRa Node ID
                {
                    uint8_t data[] = "   Server ACK";//Reply 
                    data[0] = buf[0];
                    data[1] = buf[1];
                    data[2] = buf[2];
                    rf95.send(data, sizeof(data));// Send Reply to LoRa Node
                    rf95.waitPacketSent();
                    int newData[4] = {0, 0, 0, 0}; //Store Sensor Data here
                    for (int i = 0; i < 3; i++)
                    {
                        newData[i] = buf[i + 3];
                    }
                    int h = newData[0];
                    int t = newData[1];
                    int b = newData[2];
                    
                    ThingSpeak.setField(1,h); // 
                    ThingSpeak.setField(2,t);
                    ThingSpeak.setField(3,b);
                    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);   // Send Data to IoT Server.
                }
            }       
         }
         else
         {
              //Console.println("recv failed");
              ;
          }
     }
