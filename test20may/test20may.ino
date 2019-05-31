#include <lmic.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <hal/hal.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
TinyGPS gps;
SoftwareSerial ss(10, 11); // Arduino RX, TX to conenct to GPS module.

#define ONE_WIRE_BUS 4
#define Turbidity_dpin A0

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float temperature;
int turbidity;
float turbidity1;
uint16_t temp;
int turb;

static void smartdelay(unsigned long ms);
unsigned int count = 1;        //For times count
 
float flat, flon,falt;

static uint8_t mydata[19] = {0x05,0x67,0x00,0x00,0x02,0x67,0x00,0x00,0x03,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};     

/* LoRaWAN NwkSKey, network session key
   This is the default Semtech key, which is used by the prototype TTN
   network initially.
   ttn*/
static const PROGMEM u1_t NWKSKEY[16] = {  };

/* LoRaWAN AppSKey, application session key
   This is the default Semtech key, which is used by the prototype TTN
   network initially.
   ttn*/
static const u1_t PROGMEM APPSKEY[16] = {  };

/*
 LoRaWAN end-device address (DevAddr)
 See http://thethingsnetwork.org/wiki/AddressSpace
 ttn*/
static const u4_t DEVADDR = ;

/* These callbacks are only used in over-the-air activation, so they are
  left empty here (we cannot leave them out completely unless
   DISABLE_JOIN is set in config.h, otherwise the linker will complain).*/
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }


static osjob_t initjob,sendjob,blinkjob;

/* Schedule TX every this many seconds (might become longer due to duty
 cycle limitations).*/
const unsigned TX_INTERVAL = 10;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println("OP_TXRXPEND, not sending");
    } else {
        GPSRead();
        Tem();
        printdata();
        // Prepare upstream data transmission at the next possible time.
        //  LMIC_setTxData2(1,datasend,sizeof(datasend)-1,0);
        LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
        Serial.println("Packet queued");
        Serial.print("LMIC.freq:");
        Serial.println(LMIC.freq);
        Serial.println("Receive data:");
        
    } 
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    Serial.println(ev);
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            Serial.println(F(""));
            if(LMIC.dataLen) {
                // data received in rx slot after tx
                Serial.print(F("Data Received: "));
                Serial.write(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
                Serial.println();
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void setup() {
     // initialize digital pin  as an output.
   
    Serial.begin(9600);
     ss.begin(9600);  
    sensors.begin();
    while(!Serial);
    Serial.println(F("LoRa GPS Example---- "));
    Serial.println(F("Connect to TTN"));
    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();
    /*LMIC_setClockError(MAX_CLOCK_ERROR * 1/100);
     Set static session parameters. Instead of dynamically establishing a session
     by joining the network, precomputed session parameters are be provided.*/
    #ifdef PROGMEM
    /* On AVR, these values are stored in flash and only copied to RAM
       once. Copy them to a temporary buffer here, LMIC_setSession will
       copy them into a buffer of its own again.*/
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly 
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif
    
    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

   
    
    // Set data rate and transmit power (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    // Start job
    do_send(&sendjob);
}
void GPSRead()
{
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  falt=gps.f_altitude();  //get altitude       
  flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;//save six decimal places 
  flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
  falt == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : falt, 2;//save two decimal places
  int32_t lat = flat * 10000;
  int32_t lon = flon * 10000;
  int32_t alt = falt * 100;

  mydata[10] = lat >> 16;
  mydata[11] = lat >> 8;
  mydata[12] = lat;
  mydata[13] = lon >> 16;
  mydata[14] = lon >> 8;
  mydata[15] = lon;
  mydata[16] = alt >> 16;
  mydata[17] = alt >> 8;
  mydata[18] = alt;
  
}

void Tem()
{
       sensors.requestTemperatures();
       temperature = sensors.getTempCByIndex(0);    //Temperature detection
       turbidity1 = analogRead(Turbidity_dpin); //Get A0 Status
       float voltage = turbidity1 * (5.0 / 1024.0);
       turbidity = ((-1120.4*voltage*voltage)+(5742.3*voltage)-(4352.9)); 
       
      
       temp = temperature * 10;
       turb = turbidity * 10;
       mydata[2] = temp >> 8;
       mydata[3]= temp & 0xFF;
       mydata[6] = turb >> 8;
       mydata[7] = turb & 0xFF;
}

void printdata(){
       Serial.print(F("###########    "));
       Serial.print(F("NO."));
       Serial.print(count);
       Serial.println(F("    ###########"));
       count++;
       Serial.println(F("The temperautre and turbidity :"));
       Serial.print(F("["));
       Serial.print(temperature);
       Serial.print(F("℃"));
       Serial.print(F(","));
       Serial.print(turbidity);
       Serial.print(F("NTU"));
       Serial.print(F("]"));
       Serial.println(F(""));
       if(flon!=1000.000000)
  {  
       Serial.println(F("The longtitude and latitude and altitude are:"));
       Serial.print(F("["));
       Serial.print(flon);
       Serial.print(F(","));
       Serial.print(flat);
       Serial.print(F(","));
      Serial.print(falt);
       Serial.print(F("]"));
     Serial.println(F(""));
     

      
}
smartdelay(1000);
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
    {
      gps.encode(ss.read());
    }
  } while (millis() - start < ms);
}


void loop() {
    os_runloop_once(); 
}
