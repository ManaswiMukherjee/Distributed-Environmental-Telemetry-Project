#include <WiFi.h>
#include "secrets.h"                   // make sure to create a secrets.h tab in arduino ide
#include <WiFiUdp.h>                   
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoMqttClient.h>




//WIFI & MQTT SETUP
const char* ssid = STASSID;
const char* passwd = STAPSK;

WiFiMulti multi;                        // for connecting to access point
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);      // wifi client
arduino::String mac;

const char broker[] = "192.168.1.31";    // local ip
int        port     = 1883;              //default mqtt port
const char topic1[]  = "node_data/temp"; // temperature topic
const char topic2[]  = "node_data/pres"; // pressure topic

const long interval = 1000;
unsigned long previousMillis = 0;



//SENSOR BMP280
Adafruit_BMP280 bmp; // using I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();
float temp,pres;



//DEBUG LED's RED & GREEN
const uint8_t red_led = 15;
const uint8_t green_led = 18;



//DISPLAY SSD1306
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C     // I2C address of oled screen

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);



//JSON
JsonDocument doc;
String jout;



//NTP server
WiFiUDP ntpUDP;                 // udp setup for ntp client
NTPClient timeClient(ntpUDP, "pool.ntp.org");
int epochtime;






//DEBUG LED
void warning_led() {
  digitalWrite(red_led,HIGH);
  delay(1000);
  digitalWrite(red_led,LOW);
  delay(1000);
}


// everything right led
void ok_led() {
  digitalWrite(green_led,HIGH);
  delay(1000);
  digitalWrite(green_led,LOW);
  delay(1000);
}



void Serialdisplaydata(char d,float data) {
  if(d == 't') {
    Serial.print(F("Temperature is : "));
    Serial.print(data);
    Serial.println(F("*C"));
  }
  else if(d == 'p') {
    Serial.print(F("Pressure is : "));
    Serial.print(data);
    Serial.println(F("hPa"));
  }
}



bool mqttConnectionCheck() {
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    int limit = 0;
    while(!mqttClient.connected() && limit < 30) {
      warning_led();
      delay(5000);
      limit++;
      if(mqttClient.connect(broker,port)){return true;}
    }
    return false;

  }
  else {return true;}
}












void setup() {
  Serial.begin(9600);
  

  //LED INIT
  pinMode(red_led,OUTPUT);
  pinMode(green_led,OUTPUT);

  //SENSOR INIT
  unsigned sensor_status;
  sensor_status = bmp.begin(0x76);     // 0x76 address of bmp280 sensor

  if (!sensor_status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print(("SensorID was: 0x"));
    Serial.println(bmp.sensorID(),16);
    Serial.print(("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n"));
    Serial.print(("   ID of 0x56-0x58 represents a BMP 280,\n"));
    Serial.print(("        ID of 0x60 represents a BME 280.\n"));
    Serial.print(("        ID of 0x61 represents a BME 680.\n"));
    while (1)
    { 
      warning_led();
      delay(500);
    }
  }

    /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  bmp_temp->printSensorDetails();

  //SCREEN INIT
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while(1) {
      warning_led();
      delay(500);
    }
  }

  // clear screen
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.display();

  //WIFI INIT
  multi.addAP(ssid, passwd);

  mac = WiFi.macAddress();                // original mac of device
  if (multi.run() != WL_CONNECTED) {
    Serial.println("Unable to connect to network, rebooting in 10 seconds...");
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setTextSize(2);
    display.setCursor(0,0);
    display.println(F("NO NET"));
    display.print(F("rebooting"));
    display.display();
    delay(10000);
    rp2040.reboot();
  }

  delay(2000);
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println(F("IP addr"));
  display.setTextSize(1);
  display.println();
  display.print(WiFi.localIP());
  display.display();
  delay(5000);

  //MQTT INIT
  mqttClient.setUsernamePassword("username", "password");
  if(!mqttConnectionCheck()) {rp2040.reboot();}

  Serial.println(F("You're connected to the MQTT broker!"));
  ok_led();
  //NTP TIME INIT
  timeClient.begin();
}







void loop() {
  mqttClient.poll();                  //keeping the connection alive

  if(!mqttConnectionCheck()) {rp2040.reboot();}

  //get data from sensor
  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);

  temp = temp_event.temperature;
  pres = pressure_event.pressure;

  //get time from server
  timeClient.update();
  epochtime = timeClient.getEpochTime();


    
  Serialdisplaydata('t',temp);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);
  display.println(F("Temp:"));
  display.setTextSize(2);
  display.print(temp);
  display.println(F(" *C"));


  Serialdisplaydata('p',pres);

  display.println();
  display.setTextSize(1);
  display.println(F("Pres:"));
  display.setTextSize(2);
  display.print(pres);
  display.println(F("hPa"));
  display.display();

  Serial.println();

  
  
  

  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
  // save the last time a message was sent
    previousMillis = currentMillis;

    doc["mac"] = mac;
    doc["data"] = temp;
    doc["device_timestamp"] = epochtime;
    serializeJson(doc, jout);

    mqttClient.beginMessage(topic1);
    mqttClient.print(jout);
    mqttClient.endMessage();

    doc["mac"] = mac;
    doc["data"] = pres;
    doc["device_timestamp"] = epochtime;
    serializeJson(doc, jout);

    mqttClient.beginMessage(topic2);
    mqttClient.print(jout);
    mqttClient.endMessage();

    Serial.println();

  }

  ok_led();
  delay(2000);
  display.clearDisplay();
  display.display();
}
