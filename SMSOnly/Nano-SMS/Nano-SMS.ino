
#include<SoftwareSerial.h>
const int AirValue1 = 300; //285   //you need to replace this value with Value_1
const int SWSValue1 = 190;  //you need to replace this value with Value_2

const int AirValue2 = 300; //280  //you need to replace this value with Value_1
const int SWSValue2 = 190;  //you need to replace this value with Value_2
int intRecord=0;
int soilMoistureValue1 = 0,soilMoistureValue2 = 0,soilMoistureValue3 = 0;
int Ave=0;
int soilmoisturepercent=0;
bool isWaterOn=false,isPause =  false, isHour=false;
int intIsWaterOn=0, intIsPause=0, intWater=0;
String strWeather = "";
int intWeather = 0;
bool willRain = false;

SoftwareSerial sw(5,6);  //RX,TX
void setup() {
  Serial.begin(115200); // open serial port, set the baud rate to 9600 bps
  sw.begin(9600);
  pinMode(8,OUTPUT);
  digitalWrite(8,HIGH);
}
void loop() {
  while (sw.available()){
    strWeather = sw.readStringUntil('\n');
    Serial.println();
    Serial.println("\n=====================");
    Serial.println(strWeather);
    intWeather = strWeather.toInt();
    Serial.println("=====================\n");
    if(intWeather>=200 && intWeather <=232){
      willRain = true;
      Serial.print("WILL RAIN\n");
    }
    else if(intWeather>=300 && intWeather<=321){
      willRain = true;
      Serial.print("WILL RAIN\n");
    }
    else if(intWeather>=500 && intWeather<=531){
      willRain = true;
      Serial.print("WILL RAIN\n");
    }
    else{
      willRain = false;
      Serial.print("WILL NOT RAIN\n");
    }
    
  }
  //Serial.println("Reading1: ");
  soilMoistureValue1 = analogRead(A2);  //put Sensor insert into soil
  soilMoistureValue2 = soilMoistureValue1;  //put Sensor insert into soil
  soilMoistureValue3 = analogRead(A3);  //put Sensor insert into soil

  int SM_per1 = map(soilMoistureValue1,AirValue1,SWSValue1,0,100);
  int SM_per2 = map(soilMoistureValue1,AirValue2,SWSValue2,0,100);

  Ave = (SM_per1+SM_per2)/2;
  
  Serial.println("Reading1 :" + String(soilMoistureValue1) + " - " + SM_per1 + "%, Reading2 :" + String(soilMoistureValue2) + " - " + SM_per2 + "%, Average:" + String(Ave) + " "  + String(intIsWaterOn));
  //if(intRecord==300000)
  //if(intRecord==3000){
    //intRecord=0;
    Serial.print("\n===========\nSaved to SD\n===========\n");
    sw.println("Reading1 : " + String(soilMoistureValue1) + " - " + SM_per1 + "%, Reading2 : " + String(soilMoistureValue2) + " - " + SM_per2 + "%, Average: " + String(Ave) + "%, isWaterOn : "  + String(intIsWaterOn));
  //}
//3600000
  Serial.print(intWater);
  if(intWater==3000){
    intWater=0;
    isHour=true;
  }
  
  if(isHour==true){
    Serial.print("One hour");
  
    if(isPause==false)
    {
      if(Ave<80 && willRain == false)
      //if(Ave<50)
      {
        isWaterOn = true;
        digitalWrite(8,LOW);
      }
      else{
        isHour=false;
      }
      if(intIsWaterOn == 3 && isWaterOn == true){
        intIsWaterOn=0;
        isWaterOn = false;
        digitalWrite(8,HIGH);
        isPause=true;
      }
      
      if (isWaterOn == true){
        intIsWaterOn = intIsWaterOn + 1;
      }
    }
    else{
      intIsPause++;
    }
    if(intIsPause==10){
      isPause=false;
      intIsPause=0;
    }
  }
  delay(1000);
  intRecord = intRecord + 1000;
  intWater = intWater+1000; 
}