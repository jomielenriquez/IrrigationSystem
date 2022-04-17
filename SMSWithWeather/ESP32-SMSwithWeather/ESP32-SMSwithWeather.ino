// initialize library
#include <WiFi.h>              
#include <TimeLib.h>
#include <WiFiMulti.h>         
#include <ESPmDNS.h>
#include "FS.h"
#include "Network.h"
#include "Sys_Variables.h"
#include "CSS.h"
#include "SD.h"
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"

#include <HTTPClient.h>
#include <Arduino_JSON.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

int count=0;
AsyncWebServer server(80);
String strToPrint="";
String strToPrintWeather="";
String strWeather;

String SD_Status = "";
const char* ssid = "PowerOfPhysics";
const char* password = "Aghststsawlm1625";
String jsonBuffer;
int countLoop=0;

/*
 * Connection of SD Card Reader to ESP 32
 *
 * SD Card | ESP32
 * GND      GND
 * VCC      VCC 5V
 * MOSO     G19
 * MOSI     G23
 * SCK      G18
 * CS       G5
 */

WiFiMulti wifiMulti;

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Connection of RealTime Module to ESP32
//vcc => 3.3v
//SCL => G22
//SDA => G21

#define mqtt_port 1883
#define MQTT_USER "mqtt username"
#define MQTT_PASSWORD "mqtt password"
#define MQTT_SERIAL_PUBLISH_CH "/ic/esp32/serialdata/uno/"

// Serial Communication
#define RXD2 16
#define TXD2 17

// Function to list all files in SD Card
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

// Function to create Directory in SD Card
void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

// Function to remove directory or file in SD Card
void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

// Function to read file in SD Card
void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

// Function to Write File in SD Card
void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

// Functio to Update File in SD Card
void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

// Function to rename file in SD Card
void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

// FUnction to delete File in SD Card
void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

// Function to get weather prediction
String GetWeather(){
  HTTPClient http;
    http.begin("http://api.openweathermap.org/data/2.5/onecall?lat=14.074368&lon=121.147179&exclude=minutely&appid=1ae76ac0b8679d9b65ae01f37ea44b16");
    int httpCode = http.GET(); 
    if(httpCode>0){
      jsonBuffer = http.getString();
    }
    JSONVar myObject = JSON.parse(jsonBuffer);

    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return "No Prediction";
    }


    DateTime Htime = rtc.now();
    int intHour = Htime.hour();
    if(intHour==23){
      intHour=0;  
    }
    else{
      intHour++;
    }
    for(int i=0;i<sizeof(myObject["hourly"]);i++){
      String Temp = JSON.stringify(myObject["hourly"][i]["temp"]);
      int intPreHour = hour(myObject["hourly"][i]["dt"]);
      int intMonth = month(myObject["hourly"][i]["dt"]);
      int intDay = day(myObject["hourly"][i]["dt"]);
      int intYear = year(myObject["hourly"][i]["dt"]);
      String intWeather = JSON.stringify(myObject["hourly"][i]["weather"][0]["id"]);
      strWeather = JSON.stringify(myObject["hourly"][i]["weather"][0]["description"]);
      strWeather.replace("\"","");
      
      if(intHour==intPreHour){
        strToPrintWeather = Temp + ";" + strWeather + ";" + String(intMonth) + "/" + String(intDay) + "/" + String(year(myObject["hourly"][i]["dt"])) + ";" + String(intPreHour) + "\n";
        Serial.print(strToPrintWeather.c_str());
        appendFile(SD, "/WeatherLog.txt", strToPrintWeather.c_str());
        return intWeather;
      }
    }
}

String reading(){
  return strToPrint;  
}

void setup() {
  //initializing serial 2
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  //Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  Serial.begin(115200); //Serial must always be 115200 baud
  Serial.setTimeout(500);// Set time out for 

  if (!WiFi.config(local_IP, gateway, subnet, dns)) { //WiFi.config(ip, gateway, subnet, dns1, dns2);
    Serial.println("WiFi STATION Failed to configure Correctly"); 
  } 
  //wifiMulti.addAP(ssid_1, password_1);  // add Wi-Fi networks you want to connect to, it connects strongest to weakest
  wifiMulti.addAP(ssid_2, password_2);  // Adjust the values in the Network tab
  //wifiMulti.addAP(ssid_3, password_3);
  //wifiMulti.addAP(ssid_4, password_4);  // You don't need 4 entries, this is for example!
  
  Serial.println("Connecting ...");
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250); Serial.print('.');
  }
  Serial.println("\nConnected to "+WiFi.SSID()+" Use IP address: "+WiFi.localIP().toString()); // Report which SSID and IP is in use
  if (!MDNS.begin(servername)) {          // Set your preferred server name, if you use "myserver" the address would be http://myserver.local/
    Serial.println(F("Error setting up MDNS responder!")); 
    ESP.restart(); 
  } 

  // setup sd card module
  if (! rtc.begin()) {
  Serial.println("Couldn't find RTC");
  while (1);
  }

  //Setup SD Card
  if(!SD.begin()){
        Serial.println("Card Mount Failed");
        SD_Status="Card Mount Failed";
        SD_present = false; 
        return;
    }
    else{
      Serial.println(F("Card initialised... file access enabled..."));
      SD_Status="Card initialised... file access enabled...";
      SD_present = true; 
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html);
    });

    server.on("/Reading", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", reading().c_str());
    });

    server.on("/downloadSMS", HTTP_GET, [] (AsyncWebServerRequest *request) {
      File file = SD.open("/SoilMoistureLog.txt");
      if(!file){
          Serial.println("Failed to open file for reading");
          request->send (200, "text/html", "<H1>Failed to open file for reading</h1>");
      }
      else{
          request->send(file, "/SoilMoistureLog.txt", "text/xhr", true);
          Serial.print("Recieved SoilMoistureLog.txt request from client IP ");
          Serial.println(request->client()->remoteIP());
      } 
    });
    
    server.begin();
    Serial.println("HTTP server started");
    if(WiFi.status()== WL_CONNECTED){
      Serial.print(GetWeather());
      Serial2.println(GetWeather());
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}


void loop() {
   DateTime now = rtc.now();
   String Time = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
   String Date = String(now.month()) + "-" + String(now.day()) + "-" + String(now.year());
   String StrDay = daysOfTheWeek[now.dayOfTheWeek()];
   String strSoilMoistureReading = "";
   
   while (Serial2.available()) {
    strSoilMoistureReading = Serial2.readStringUntil('\n');
    strToPrint = strSoilMoistureReading.substring(0, strSoilMoistureReading.length() - 1) + "," + StrDay + "," + Date + " " + Time + "," +SD_Status+ "," + strWeather + "\n";
    Serial.println(strToPrint);
    appendFile(SD, "/SoilMoistureLog.txt", strToPrint.c_str());
   }
   if(countLoop>=300000){
    Serial2.println(GetWeather());
    countLoop=0;
   }
   countLoop=countLoop+10;
   delay(10);
 }
