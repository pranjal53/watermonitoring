#include <SoftwareSerial.h>
#include<A7Client.h>
#include <SD.h>
#include <SPI.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS A3
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);
A7Client a7Client (10, 11); // RX, TX on Uno, connected to Tx, Rx of A7 Module

const char server[] = "waterqualitymonitoring.azurewebsites.net";
//char message[] = "GET /log.php?t=2018-12-17,12:31:00&te=30.0&do=7.05&pH=5.66&tu=7&la=28.5441421&ln=77.2720617 HTTP/1.1\r\nHost: waterqualitymonitoring.azurewebsites.net\r\nConnection: close\r\n\r\n";
char msg[200] = ""; 
// char time[] = "2018-12-12,22:35:01";
char temperature[7] = "";
char pH[6] = "";
char DO[7] = ""; 
char turbidity[5] = "";
char lat[9] = "";
char lng[9] = "";

const int chipSelect = 53;
String dataString = "";
const uint8_t pinRST = 5;
const uint8_t pinPWR = 6;
const uint8_t pinTurbidity    = A0;
const uint8_t pinPH           = A1;
const uint8_t pinDO           = A2;
const uint8_t pinTemperature  = A3;

struct Data{
  float temperature;
  float pH;
  float DO;
  int turbidity;
} data;

unsigned long previousMillis = 0; //for looping purpose
long timeInterval = 10000; //update once per 10 seconds

void setup() {
  Serial.begin(9600);
  sensors.begin(); 
  Serial.println(freeRam());


  pinMode(pinRST, OUTPUT);
  pinMode(pinPWR, OUTPUT);
  digitalWrite(pinRST, HIGH);//reset the A7 module
  delay(1000);
  digitalWrite(pinRST, LOW);//finish the reset

  digitalWrite(pinPWR, LOW);//POWER UP the A7 module via a PNP transistor  
  delay(2000); //need to apply power to the pin for >2 seconds
  digitalWrite(pinPWR, HIGH);//finish the power up
  delay(3500);//let the module stable, it will output some gibberish.

  a7Client.changeBaud();//Baud rate for A7.serial is now at 9600 bps
  //Serial.println(freeRam());
  Serial.println(F("Response okay! :) Module is alive!"));
  delay(10000); //wait for the config message for GPRS and everything 

  while(!a7Client.startGPS()){// make sure GPS is on  
  Serial.println(F("Try activating GPS again."));
  }
  // a7Client.startGPS();
  getData(&data);
  printData(&data);
    
  while(!a7Client.readGPS());//force Arduino to complete one GPS at least once =.= 16/11/16      
  if(a7Client.parse(a7Client.lastNMEA())){
    // Serial.print(F("Location: "));
    // Serial.print(a7Client.latitude, 4); Serial.print(a7Client.lat);
    // Serial.print(F(", ")); 
    // Serial.print(a7Client.longitude, 4); Serial.println(a7Client.lon);
    // Serial.print(F("Location (in degrees, works with Google Maps): "));
    // Serial.print(a7Client.latitudeDegrees, 4);
    // Serial.print(F(", ")); 
    // Serial.println(a7Client.longitudeDegrees, 4);    
}

//force the GPS to be DUP if lat=0.000, 0.000
if( a7Client.latitudeDegrees - 0.000 < 0.0000001){
  a7Client.latitudeDegrees = 28.5441421;
    a7Client.longitudeDegrees = 77.2720617;
}

  convertToChar(&data, a7Client);
   // if (lat[0] == '0'){ //force the GPS to be DUP :)
  //   lat[0] = '2';
  //  lat[1] = '8';
  //  lat[2] = '.';
  //  lat[3] = '5';
  //  lat[4] = '4';
  //  lat[5] = '4';
  //  lat[6] = '1';
  //  lat[7] = '4';
 //   lat[8] = '2';
 //   lat[9] = '1';

 //   lng[0] = '7';
 //   lng[1] = '7';
 //   lng[2] = '.';
//    lng[3] = '2';
 //   lng[4] = '7';
 //   lng[5] = '2';
  //  lng[6] = '0';
 //   lng[7] = '6';
 //   lng[8] = '1';
//    lng[9] = '7';
  //  }    
  constructHTTPRequest(msg);  
  a7Client.connect(server, 80);
  sendHTTPRequest(msg);  

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }

   // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("LoggerCD.txt", FILE_WRITE);
}


void loop() {  dataString = sensors.getTempCByIndex(0);
  
  // Serial.println(F("I am down here"));
  
  // if (a7Client.available())
  // Serial.write(a7Client.read());

  // if (Serial.available())
  // a7Client.writeSerial(Serial.read());

//start of millis() code from AdaFruit
  unsigned long currentMillis = millis();
  getData(&data);    
  printData(&data);
  
  if((currentMillis - previousMillis >= timeInterval)){
    previousMillis = currentMillis;
    getData(&data);    
    printData(&data);

    while(!a7Client.readGPS());      
    if(a7Client.parse(a7Client.lastNMEA())){
      Serial.println(F("parsed completed"));    
    }
    convertToChar(&data, a7Client);  
    printCharData();
    constructHTTPRequest(msg);    
    if (a7Client.connect(server, 80)){
      
    }
    sendHTTPRequest(msg);
  }  File dataFile = SD.open("LoggerCD.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print(dataString); //Store date on SD card
    dataFile.print(","); //Move to next column using a ","

    dataFile.print(pH); //Store date on SD card
    dataFile.print(","); //Move to next column using a ","

    dataFile.print(DO); //Store date on SD card
    dataFile.print(","); //Move to next column using a ","

    dataFile.print(turbidity); //Store date on SD card
    dataFile.print(","); //Move to next column using a ","

    dataFile.print(lat); //Store date on SD card
    dataFile.print(","); //Move to next column using a ","

    dataFile.print(lng); //Store date on SD card
    dataFile.print(","); //Move to next column using a ","

    dataFile.println(); //End of Row move to next row
    dataFile.close(); //Close the file
  }
  else
  Serial.println("OOPS!! SD card writing failed");
}
