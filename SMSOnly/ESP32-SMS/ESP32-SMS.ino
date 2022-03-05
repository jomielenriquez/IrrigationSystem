#include <WiFi.h> 
#include <TimeLib.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include "FS.h"
#include "Network.h"
#include "Sys_Variables.h"
#include "CSS.h"
//SD Card Library
#include "SD.h"
#include <SPI.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>

ThreeWire myWire(4,15,2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

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
 * Connect the SD card to the following pins:
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
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//vcc => 3.3v
//SCL => G22
//SDA => G21

#define mqtt_port 1883
#define MQTT_USER "mqtt username"
#define MQTT_PASSWORD "mqtt password"
#define MQTT_SERIAL_PUBLISH_CH "/ic/esp32/serialdata/uno/"

// Serial Monitor pin 
#define RXD2 17
#define TXD2 16
#define countof(a) (sizeof(a) / sizeof(a[0]))

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

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

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

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

String reading(){
  return strToPrint;  
}
void setup() {
  //initializing serial 2
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
  // The logical name http://fileserver.local will also access the device if you have 'Bonjour' running or your system supports multicast dns
  if (!MDNS.begin(servername)) {          // Set your preferred server name, if you use "myserver" the address would be http://myserver.local/
    Serial.println(F("Error setting up MDNS responder!")); 
    ESP.restart(); 
  } 

  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    //printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
  //rtc.adjust(DateTime(__DATE__, __TIME__));

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

    //Host web server
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
      //Serial.print(GetWeather());
      //Serial2.println(GetWeather());
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}


void loop() {
   //server.handleClient(); // Listen for client connections
   
   RtcDateTime now = Rtc.GetDateTime();
   
   //DateTime now = rtc.now();
   char datestring[20];
   char timestring[20];
   
   snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u"),
            now.Month(),
            now.Day(),
            now.Year());
   snprintf_P(timestring, 
            countof(datestring),
            PSTR("%02u:%02u:%02u"),
            now.Hour(),
            now.Minute(),
            now.Second() );
   
   String Time = timestring;
   String Date = datestring;
   //String StrDay = daysOfTheWeek[now.dayOfTheWeek()];
   String StrDay = "";
   String strSoilMoistureReading = "";
   
   while (Serial2.available()) {
    strSoilMoistureReading = Serial2.readStringUntil('\n');
    strToPrint = strSoilMoistureReading.substring(0, strSoilMoistureReading.length() - 1) + "," + StrDay + "," + Date + " " + Time + "," +SD_Status+ "," + strWeather + "\n";
    Serial.println(strToPrint);
    appendFile(SD, "/SoilMoistureLog.txt", strToPrint.c_str());
   }
   //3600000
   if(countLoop>=300000){
    //Serial2.println(GetWeather());
    countLoop=0;
   }
   countLoop=countLoop+10;
   delay(10);
 }
