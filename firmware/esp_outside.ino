#include <time.h>
#include <Wire.h>
#include "secrets.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <Adafruit_BMP280.h>
#include <ArduinoMqttClient.h>


// WIFI and MQTT client
const char* ssid = STASSID;
const char* passwd = STAPSK;

IPAddress local_IP(192,168,1,36);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress dns1(8,8,8,8);
IPAddress dns2(8,8,2,2);

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
String mac;

const char broker[] = "192.168.1.31";
int        port     = 1883;
const char topic1[]  = "node_data/temp";
const char topic2[]  = "node_data/pres";

const long interval = 1000;
unsigned long previousMillis = 0;


// SENSOR BMP280
Adafruit_BMP280 bmp;
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();
float temp,pres;


// JSON
JsonDocument doc;
String jout = "";


// GPS
static const int RXPin = 14, TXPin = 12;
static const uint32_t GPSBaud = 9600;
time_t unix_time;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void warning_led() {
  digitalWrite(D7,LOW);
  delay(1000);
  digitalWrite(D7,HIGH);
  delay(1000);
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



time_t convertToUnixTime(uint16_t year, uint8_t month, uint8_t day, 
                         uint8_t hour, uint8_t minute, uint8_t second) {
    struct tm timeinfo = {0};
    timeinfo.tm_year = year - 1900;    // Years since 1900
    timeinfo.tm_mon = month - 1;       // Months since January (0-11)
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    timeinfo.tm_isdst = 0;             // No daylight saving (GPS uses UTC)
    
    return mktime(&timeinfo);
}






void setup() {
  Serial.begin(9600);
  ss.begin(GPSBaud);
  pinMode(D7,OUTPUT);
  
  pinMode(D4,OUTPUT);
  digitalWrite(D4,LOW);
  delay(500);
  digitalWrite(D4,HIGH);
  delay(500);

  //SENSOR INIT
  Wire.begin(4,5); // esp needs initialization
  unsigned status;
  //status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  status = bmp.begin(0x76);
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    for (int i = 0; i < 5; i++) {
      warning_led();
      delay(500);
    }
    ESP.deepSleep(30e6);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X16,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_1000); /* Standby time. */

  bmp_temp->printSensorDetails();

  //WIFI && MQTT INIT
  mac = WiFi.macAddress();                // original mac of device
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.config(local_IP, gateway, subnet, dns1, dns2);
  WiFi.begin(ssid, passwd, channel, bssid);
  int limit = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    limit++;
    if(limit > 5){break;}
  }

  if(!mqttConnectionCheck()) {ESP.deepSleep(30e6);}
}





/*int sendTimes = 0; */ //uncomment for implementing deep sleep after sending

void loop() {
  
  
  mqttClient.poll();                  //keeping the connection alive  
  while (ss.available())
      gps.encode(ss.read());

  if(!mqttConnectionCheck()) {ESP.deepSleep(30e6);}
  
      
  unsigned long currentMillis = millis();
  //sendData();
  if (currentMillis - previousMillis >= interval) {
  // save the last time a message was sent
    previousMillis = currentMillis;
    if (!gps.date.isValid() || !gps.time.isValid() || gps.date.year() < 2025 || !gps.time.isUpdated()) {}
    else {
      unix_time = convertToUnixTime(gps.date.year(),gps.date.month(),gps.date.day(),gps.time.hour(),gps.time.minute(),gps.time.second());
      sendData();
      sendTimes++;
    }

    Serial.println(unix_time);
    unix_time = 0;
    /*if(sendTimes>1000)
    {
      ESP.deepSleep(time_us)
    }*/ //uncomment to implement deep sleep mode and modify number of times to send 
  }
}

void sendData() {
  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);

  temp = temp_event.temperature;
  pres = pressure_event.pressure;
  jout = "";
  doc.clear();
  doc["mac"] = mac;
  doc["data"] = temp;
  doc["device_timestamp"] = unix_time;
  serializeJson(doc, jout);

  mqttClient.beginMessage(topic1);
  mqttClient.print(jout);
  mqttClient.endMessage();
   
  jout = "";
  doc.clear();
  doc["mac"] = mac;
  doc["data"] = pres;
  doc["device_timestamp"] = unix_time;
  serializeJson(doc, jout);

  mqttClient.beginMessage(topic2);
  mqttClient.print(jout);
  mqttClient.endMessage();
  jout = "";
    
}

