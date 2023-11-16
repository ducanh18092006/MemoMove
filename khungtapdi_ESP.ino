#include <SoftwareSerial.h>
SoftwareSerial SIM900(D7, D8);  //rx tx
String number1 = "0964255231";
#include <MPU6050_tockn.h>
#include <Wire.h>

#include "DFRobotDFPlayerMini.h"
DFRobotDFPlayerMini myDFPlayer;
SoftwareSerial mySoftwareSerial(D3, D4);//rx tx dfplayer mini

MPU6050 mpu6050(Wire);
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL60xsX9kNl"
#define BLYNK_TEMPLATE_NAME "khung tap di"
#define BLYNK_AUTH_TOKEN "h8LXqtVtu0dEU_WSpha0MkNRFYrRx1b0"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(D6, D5);  //rx,tx
// HX711 circuit wiring
char auth[] = "h8LXqtVtu0dEU_WSpha0MkNRFYrRx1b0";
char ssid[] = "AmericanStudy T1";
char pass[] = "66668888";
String nhiptim, oxy, nhietdo, khoangcach, data[4];
uint32_t lastSendData = 0;
int cambien = D8;
bool check = true;
void setup() {
  mySoftwareSerial.begin(9600);//dfplayer
  Serial.begin(9600);
  mySerial.begin(4800);
  pinMode(cambien, INPUT);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  SIM900.begin(9600);           //19200
  delay(2000);                  // give time to log on to network.
  SIM900.print("AT+CLIP=1\r");  // test
  delay(1000);
  SIM900.println("AT");
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
 if (myDFPlayer.begin(mySoftwareSerial)) {
     myDFPlayer.volume(30);  
}
}
void loop() {
  mpu6050.update();
   Serial.print("angleX : ");
  Serial.print(mpu6050.getAngleX());
  Serial.print("\tangleY : ");
  Serial.print(mpu6050.getAngleY());
  Serial.print("\tangleZ : ");
  Serial.println(mpu6050.getAngleZ());
  if (abs(mpu6050.getAngleY()) > 70) {
        if (check== true){
            nhantin();
            check= false ;
        }
    Serial.println("canh bao benh nhan bi nga !");
    Serial.println("can cap cuu");
  }
  else{
    check = true;
  }
  //   if(Serial.available() >0)

  //   switch (Serial.read())
  //   {
  //   case '1':
  //   Serial.println("dang goi");
  //   goidien();
  //     break;
  //   case '2':
  //   Serial.println("dang nhan tin");
  //  nhantin();
  //     break;
  
  Blynk.run();
  // if (millis()-  lastSendData >1000){
  nhandulieu();
  Blynk.virtualWrite(V0, nhiptim);
  Blynk.virtualWrite(V1, oxy);
  Blynk.virtualWrite(V2, nhietdo);
  Blynk.virtualWrite(V3, khoangcach);
}
  BLYNK_WRITE(V4){
  int p = param.asInt();
  if(p==1)
  {
    nhacnho(1,1000);
  }
  else
  {
     myDFPlayer.pause();
  }
}

void nhandulieu() {
  if (mySerial.available()) {                              // Kiểm tra xem có dữ liệu đang được gửi đến không
    String receivedData = mySerial.readStringUntil('\n');  // Đọc chuỗi dữ liệu cho đến khi gặp ký tự '\n'

    // Tách dữ liệu bằng dấu phẩy
    int startIndex = 0;
    int commaIndex;
    for (int i = 0; i < 4; i++) {
      commaIndex = receivedData.indexOf(',', startIndex);
      if (commaIndex != -1) {
        data[i] = receivedData.substring(startIndex, commaIndex);
        startIndex = commaIndex + 1;
      } else {
        data[i] = receivedData.substring(startIndex);
        break;
      }
    }
    nhiptim = data[0];
    oxy = data[1];
    nhietdo = data[2];
    khoangcach = data[3];
    Serial.println(nhiptim);
    Serial.println(oxy);
    Serial.println(nhietdo);
    Serial.println(khoangcach);
  }
}
void nhantin() {
  SIM900.println("AT+CMGF=1");
  delay(1000);
  //Serial.println ("Set SMS Number");
  SIM900.println("AT+CMGS=\"" + number1 + "\"\r");
  delay(1000);
  String SMS = "canh bao benh nhan bi nga";
  SIM900.println(SMS);
  delay(100);
  SIM900.println((char)26); 
  delay(1000);
}
void nhacnho(int number ,int time) {
     myDFPlayer.play(number);
     delay(time);
}