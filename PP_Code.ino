 #include <Ticker.h>

//#include <TimerOne.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"


#include <ESP32_Servo.h>
#include <Arduino.h>
#include <IRremote.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "dhanasekaran01"            // Replace it with your username
#define AIO_KEY         "aio_igcx47nQN7VGvnpcoMBauJSvrima"   // Replace with your Project Auth Key



#define WLAN_SSID       "DS"             // Your SSID
#define WLAN_PASS       "Dsekaran"        // Your password


#define DECODE_NEC

Ticker Remo;

Servo servo1;
Servo servo2;
Servo servo3;

int pos1 = 0;    
int servoPin1 = 18;


int pos2 = 0;    
int servoPin2 = 22;


int pos3 = 0;    
int servoPin3 = 19;

SoftwareSerial mySoftwareSerial(13,12); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);


Adafruit_MQTT_Subscribe leg = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/left"); // FeedName
Adafruit_MQTT_Subscribe hand = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/right"); // FeedName
Adafruit_MQTT_Subscribe Stop = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/stop"); // FeedName

void MQTT_connect();


void setup() 
{
  Remo.attach(0.5, Remote);
  
  
  mySoftwareSerial.begin(9600);
  Serial.begin(9600);
  
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo3.attach(servoPin3);
  servo3.write(180);
  servo1.write(0);
  servo2.write(0);

  IrReceiver.begin(21, 2, 2);

   Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
 

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&leg);
  mqtt.subscribe(&hand);
  mqtt.subscribe(&Stop);

  myDFPlayer.pause();
  
}

int leg_State, hand_State, Stop_State; 

void Remote()
{
  if (IrReceiver.decode()) {

        // Print a short summary of received data
        IrReceiver.printIRResultShort(&Serial);
        if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
            // We have an unknown protocol here, print more info
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        }
        Serial.println();

     
        IrReceiver.resume(); // Enable receiving of the next value

        if (IrReceiver.decodedIRData.command == 0x12) 
        {
            Hand(5); // number used to tell cycles
            Serial.println("Hand(5)");
        } 
        
        else if (IrReceiver.decodedIRData.command == 0x1A) // IrReceiver.decodedIRData.command - used to get data
        {
            Leg(5);
            Serial.println("Leg(5)");
        }
        else if (IrReceiver.decodedIRData.command == 0x1E) 
        {
            Body(5);
            Serial.println("Body(5)");
        }
        else if (IrReceiver.decodedIRData.command == 0x1 ) 
        {
            myDFPlayer.pause();
            Serial.println("Pause/Stop");
        }
        else if (IrReceiver.decodedIRData.command == 0x2 ) 
        {
            myDFPlayer.previous();
            Serial.println("Previous");
        }
        else if (IrReceiver.decodedIRData.command == 0x3 ) 
        {
            myDFPlayer.next();
            Serial.println("Next");
        }
        else if (IrReceiver.decodedIRData.command == 0x4 ) 
        {
            myDFPlayer.start();
            Serial.println("Start");
        }
        else if (IrReceiver.decodedIRData.command == 0x5 ) 
        {
            myDFPlayer.volumeDown();
            Serial.println("-");
        }
        else if (IrReceiver.decodedIRData.command == 0x6 ) 
        {
            myDFPlayer.volumeUp(); 
            Serial.println("+");
        }
        
    }
}

void loop() 
{

  noInterrupts();
  interrupts();

  MQTT_connect();
  
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &leg) {
      Serial.print(F("Got: "));
      Serial.println((char *)leg.lastread);
      leg_State = atoi((char *)leg.lastread);
    }
    if (subscription == &hand) {
      Serial.print(F("Got: "));
      Serial.println((char *)hand.lastread);
      hand_State = atoi((char *)hand.lastread);
    }
    if (subscription == &Stop) {
      Serial.print(F("Got: "));
      Serial.println((char *)Stop.lastread);
      Stop_State = atoi((char *)Stop.lastread);
    }
  }
  Serial.print("Leg - ");
  Serial.println(leg_State);
  Serial.print("Hand - ");
  Serial.println(hand_State);
  Serial.print("Stop - ");
  Serial.println(Stop_State);

  if (leg_State == 1)
  {
    leg_State = 0 ;
    Hand(5);
    Serial.println("Leg Exercise");
  }

  if (hand_State == 1)
  {
    hand_State = 0 ;
    Leg(5);
    Serial.println("Hand Exercise");
  }

  

  
}


void Hand(int cycle)
{
  for(int i = 0 ; i < cycle ; i++)
  {
    for (pos1 = 0; pos1 <= 180; pos1 += 3)  // (pos1 = 0; pos1 <= 180; pos1 += *)  * can be changed to adjust speed
    { 
      servo1.write(pos1);              
      delay(25); // to adjust speed                      
    }
    for (pos1 = 180; pos1 >= 0; pos1 -= 3) 
    { 
      servo1.write(pos1);              
      delay(25);                       
    }
        Serial.println(i);

    if (Stop_State == 1)
  {
    Stop_State = 0 ;
    
    Serial.println("Stop Exercise");
    break;
  }
    servo1.write(0);
  }
}


void Leg(int cycle)
{
  for(int i = 0 ; i < cycle ; i++)
  {

    for (pos2 = 0; pos2 <= 180; pos2 += 3) 
    { 
      servo2.write(pos2);              
      delay(25);                       
    }
    for (pos2 = 180; pos2 >= 0; pos2 -= 3) 
    { 
      servo2.write(pos2);              
      delay(25);                       
    }
        Serial.println(i);

    if (Stop_State == 1)
  {
    Stop_State = 0 ;
    
    Serial.println("Stop Exercise");
    break;
  }
    servo2.write(0);
  }
}


void Body(int cycle)
{
  for(int i = 0 ; i < cycle ; i++)
  {

    for (pos3 = 180; pos3 >= 0; pos3 -= 3) 
    { 
      servo3.write(pos3);              
      delay(25);                       
    }

    for (pos3 = 0; pos3 <= 180; pos3 += 3) 
    { 
      servo3.write(pos3);              
      delay(25);                       
    }
    
        Serial.println(i);

    if (Stop_State == 1)
  {
    Stop_State = 0 ;
    
    Serial.println("Stop Exercise");
    break;
  }
    servo3.write(180);
    }

}
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
  
}
