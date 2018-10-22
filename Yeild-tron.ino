#include <Adafruit_SleepyDog.h>
#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_FONA.h"
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 5      
#define DHTTYPE DHT11
#define sensorPin A0
#define phsensor A1
#define uploadTime 2000
#define FONA_RX  9
#define FONA_TX  8
#define FONA_RST  4
#define callerIDbuffer "08104673556"
unsigned long previous = 0;
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

/*********************** APN field ********************************/
#define FONA_APN       ""
#define FONA_USERNAME  ""
#define FONA_PASSWORD  ""

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "klempy"
#define AIO_KEY         "caf3ff7bebd842918fff1f38f1426a8b"
/************ Global State (you don't need to change this!) ******************/

// Setup the FONA MQTT class by passing in the FONA class and MQTT server and login details.
Adafruit_MQTT_FONA mqtt(&fona, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

// FONAconnect is a helper function that sets up the FONA and connects to
// the GPRS network. See the fonahelper.cpp tab above for the source!
boolean FONAconnect(const __FlashStringHelper *apn, const __FlashStringHelper *username, const __FlashStringHelper *password);

/****************************** Feeds ***************************************/

Adafruit_MQTT_Publish soilph = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/farm-data");
Adafruit_MQTT_Publish moisture = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moist");
Adafruit_MQTT_Publish Temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish humid = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humid");

/*************************** Sketch Code ************************************/

 float sensorValue ;
 DHT dht (DHTPIN, DHTTYPE);
 void setup () {
   dht.begin();    
   while (!Serial);

  // Watchdog is optional!
  Watchdog.enable(8000);
  
  Serial.begin(115200);

  Serial.println(F("Adafruit FONA MQTT demo"));

 // mqtt.subscribe(&onoffbutton);

  Watchdog.reset();
  delay(5000);  // wait a few seconds to stabilize connection
  Watchdog.reset();
  
  // Initialise the FONA module
  while (! FONAconnect(F(FONA_APN), F(FONA_USERNAME), F(FONA_PASSWORD))) {
    Serial.println("Retrying FONA");
  }

  Serial.println(F("Connected to Cellular!"));

  Watchdog.reset();
  delay(5000);  // wait a few seconds to stabilize connection
  Watchdog.reset();
 }
 boolean check = false, check1 = false;
 void loop() {
     sensorValue = analogRead(A0);
     float phval = analogRead(A1);
     sensorValue=map(sensorValue, 0, 1023, 100, 0);
     phval = map(phval, 0, 1023, 0, 14);
     Serial.print("ph : ");
     Serial.print(phval);
  
     float humidity = dht.readHumidity ();  
     float temp = dht.readTemperature (); 
       MQTT_connect();
     if (phval < 5.0 && !check){
        if (!fona.sendSMS(callerIDbuffer, "PH is below 5.0: add ")) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Sent!"));
      }
      check = true;
     }
     else if (phval >= 5.0 && check)
       check = false;
     if (phval > 7.0 && !check1){
        if (!fona.sendSMS(callerIDbuffer, "PH is above 7.0: add ")) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Sent!"));
      }
      check1 = true;
     }
     else if (phval <= 5.0 && check1)
       check1 = false;     
       
      Watchdog.reset();
      // Now we can publish stuff!
      if (millis() - previous > uploadTime){
      Serial.print(F("\nSending Sensors val "));
      if (!soilph.publish(phval))
          Serial.println(F("sent!"));
      if (!humid.publish(humidity))
          Serial.println(F("sent!")); 
      if (!Temp.publish(temp))
          Serial.println(F("sent!")); 
      if (!moisture.publish(sensorValue))
          Serial.println(F("sent!")); 
      }
 }
                                                            
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(2000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}
