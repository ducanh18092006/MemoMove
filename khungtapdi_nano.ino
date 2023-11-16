
#include<Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <TinyGPSPlus.h>
#include <Adafruit_MLX90614.h>
#include<SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);//rx ,tx truyền uart đến esp qua phần mềm
#define RXPin 7     //chan rx softuart gps
#define TXPin 8  
SoftwareSerial mygps(RXPin, TXPin);//rx , tx của module GPS
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
TinyGPSPlus gps;
#define REPORTING_PERIOD_MS     1000
PulseOximeter pox;
uint32_t tsLastReport = 0;
float nhietdo,nhiptim,oxy;
float chuvibanhxe= 22.3 ;
int dem=0;
int cambien=9;
float khoangcach=0;
uint32_t last_check = millis();
float latitude;   //Storing the Latitude
float longitude;  
bool check = true; 
void setup()
{
    mySerial.begin(4800);//khởi tạo cổng myserial để truyền uart
    Serial.begin(9600);
    pinMode(cambien,INPUT);
    Serial.print("khoi tao cảm biến nhịp tim và oxy..");
    if (!pox.begin()) {
     Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
     pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    mlx.begin(0x5A);

    Wire.setClock(100000);
}

void loop()
{
  pox.update();
  dokhoangcach();

    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      Serial.print("nhiet do:");
     nhietdo=mlx.readObjectTempC();
     Serial.println(nhietdo);
      Serial.println("°C");
     
        Serial.print("nhip tim la:");
       nhiptim=pox.getHeartRate();
       Serial.println(nhiptim);

        Serial.print("oxy SpO2:");
        oxy=pox.getSpO2();
        Serial.println(oxy);
        send_dulieu();

        tsLastReport = millis();
    }
    while (mygps.available() > 0) {
    // sketch displays information every time a new sentence is correctly encoded.
    if (gps.encode(mygps.read()))
      Location();
}
}
void dokhoangcach()
{
    if(digitalRead(cambien)==0)
    {
      if(check == true)
      {
          dem++;
      delay(1000);
      khoangcach=dem*chuvibanhxe;
      Serial.print("khoang cach la:");
      Serial.print(khoangcach/100);
      check=false;
      }
    }
    else
    {
      check = true;
    }
}
void send_dulieu() {
  String data[4] = { String(nhiptim, 2), String(oxy, 2), String(nhietdo, 2) ,String(khoangcach/100,2) };
////////////////////      0                     1                2                    3 
  String combinedData = "";
  
  // Kết hợp các dữ liệu vào chuỗi
  for (int i = 0; i < 4; i++) {
    combinedData += data[i];
    if (i <4 ) {
      combinedData += ",";  //
    }
  }
  combinedData += "\n";
  // Gửi chuỗi dữ liệu qua UART
  mySerial.print(combinedData);//truyền dữ liệu đến esp
}

void Location() {
  if (gps.location.isValid()) {
    latitude = (gps.location.lat());  //Storing the Lat. and Lon.
    longitude = (gps.location.lng());
#ifdef DEBUG
    Serial.print("LATITUDE:  ");
    Serial.println(latitude, 6);  // float to x decimal places
    Serial.print("LONGITUDE: ");
    Serial.println(longitude, 6);
#endif
  }
}
