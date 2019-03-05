#include <SPI.h>
#include <RH_RF95.h>
#include <SD.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS A3
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

#define SensorPin A1          //pH meter Analog output to Arduino Analog Input 1
#define Turbidity_dpin A0 
#define pH_dpin A1 
#define DO_dpin A2
#define Temperature_dpin A3

RH_RF95 rf95;

const uint8_t pinTurbidity    = A0;
const uint8_t pinPH           = A1;
const uint8_t pinDO           = A2;
const uint8_t pinTemperature  = A3;

uint16_t datatemperature = 0;
int dataturbidity = 0;


unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int tuf[10],temp;


byte bGlobalErr;
float Temperature_dat[4];// Store Sensor Data
float Turbidity_dat[3];
float pH_dat[4];
char node_id[3] = {1, 1, 1}; //LoRa End Node ID
float frequency = 868.0;
unsigned int count = 1;

void setup()
{
  sensors.begin();
 
 
  Serial.begin(9600);
  if (!rf95.init())
    Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);
  rf95.setSyncWord(0x34);

  Serial.println("LoRa End Node Example --");
  Serial.println("   Temperature and Turbidity Sensor\n");
  Serial.print("LoRa End Node ID: ");

  for (int i = 0; i < 3; i++)
  {
    Serial.print(node_id[i], HEX);
  }
  Serial.println();
}


void loop()

{
  int i;
  for (i = 0; i < 5; i++){
   sensors.requestTemperatures();
  Temperature_dat[i]= sensors.getTempCByIndex(0);}
  int Turbidity_or;
 

 for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    tuf[i]=analogRead(SensorPin);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(tuf[i]>tuf[j])
      {
        temp=tuf[i];
        tuf[i]=tuf[j];
        tuf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=tuf[i];
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  pH_dat[0]=3.5*phValue; 





delay(100);
  Turbidity_or = analogRead(Turbidity_dpin); //Get A0 Status
  float voltage = Turbidity_or * (5.0 / 1024.0);
   Turbidity_dat[1]= ((-1120.4*voltage*voltage)+(5742.3*voltage)-(4352.9)); 
delay(100);
   
  Serial.print("###########    ");
  Serial.print("COUNT=");
  Serial.print(count);
  Serial.println("    ###########");
  count++;
  char data[50] = {0} ;
  int dataLength = 6; // Payload Length
  // Use data[0], data[1],data[2] as Node ID
  data[0] = node_id[0] ;
  data[1] = node_id[1] ;
  data[2] = node_id[2] ;
  data[3] = Temperature_dat[0];
//  data[4] = Temperature_dat[1];//Get Temperature Integer Part
  data[4] = Turbidity_dat[1];//Get Temperature Decimal Part
  data[5] = pH_dat[0];
  //Get Temperature Decimal Part

  rf95.send(data, sizeof(data));


      Serial.print(" Current Temperature = ");
      Serial.print(data[3], DEC);//Show humidity
     // Serial.print(".");
      //Serial.print(data[4], DEC);//Show humidity
      Serial.print(" C  ");
      delay(300);
      Serial.print(" Current Turbidity = ");
      Serial.print(data[4], DEC);//Show humidity
      Serial.print(" NTU");
      delay(300);
      Serial.print(" pH = ");
      Serial.print(data[5], DEC);//Show humidity
    //  Serial.print(data[7], DEC);//Show humidity
}
