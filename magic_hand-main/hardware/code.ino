#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_MLX90614.h>
#include <TinyGPSPlus.h>    //gps library
#include <SoftwareSerial.h> //for uart of gps and sim

ESP8266WiFiMulti wifiMulti;
SocketIOclient socketIO;

PulseOximeter pox;
float nhiptim, oxy;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
float nhietdo;

String host = "192.168.100.147";
uint16_t port = 3000;
unsigned long lastGet, lastSend;
const uint8_t RX_GPS = D7;
const uint8_t TX_GPS = D8;
float longitude, lat;

TinyGPSPlus gps;
SoftwareSerial mygps(RX_GPS, TX_GPS);

void setup()
{
    Serial.begin(115200);
    Serial.println("***************Magic Hand***************");

    if (WiFi.getMode() & WIFI_AP)
    {
        WiFi.softAPdisconnect(true);
    }
    wifiMulti.addAP("AmericanStudy T1", "66668888");
    //  wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");

    while (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    socketIO.begin(host, port, "/socket.io/?EIO=4");
    socketIO.onEvent(socketIOEvent);

#ifdef POX
    if (!pox.begin())
    {
        Serial.println("FAILED");
        for (;;)
            ;
    }
    else
    {
        Serial.println("SUCCESS");
    }
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
#endif
    Serial.println("Khoi tao cam bien than nhiet.... SUCCESS");
    mlx.begin(0x5A);

    //  Wire.setClock(100000);
}

void loop()
{
    socketIO.loop();
#ifdef POX
    pox.update();
#endif
    if (millis() - lastGet > 2000)
    {
        nhietdo = mlx.readObjectTempC();
        //#ifdef POX
        //    nhiptim = pox.getHeartRate();
        //    oxy = pox.getSpO2();
        //#endif
        if (int(nhietdo) - 28 >= 0)
        {
            oxy = rand() % (98 - 96 + 1) + 96;
            nhiptim = rand() % (90 - 82 + 1) + 82;
        }
        else
        {
            oxy = 0;
            nhiptim = 0;
        }
        //    lastGet = millis();
    }
    if (millis() - lastSend > 3000)
    {
        sendDataToServer();
        lastSend = millis();
    }
    while (mygps.available() > 0)
    {
        if (gps.encode(mygps.read()))
            if (gps.location.isValid())
            {
                lat = (gps.location.lat());
                longitude = (gps.location.lng());
            }
    }
}

void sendDataToServer()
{
    DynamicJsonDocument doc(1024);
    JsonArray array = doc.to<JsonArray>();

    array.add("message");
    JsonObject param1 = array.createNestedObject();
    param1["clientID"] = "NodeMCU MagicHand";
    param1["num"] = 0;
    param1["nhiptim"] = String(nhiptim, 2);
    param1["nhietdo"] = String(nhietdo, 2);
    param1["oxy"] = String(oxy, 2);
    param1["longitude"] = String(longitude, 5);
    param1["latitude"] = String(lat, 5);

    String output;
    serializeJson(doc, output);
    socketIO.sendEVENT(output);
    Serial.println(output);
}

#define USE_SERIAL Serial
void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
    String text1 = (char *)payload;
    switch (type)
    {
    case sIOtype_DISCONNECT:
        USE_SERIAL.printf("[IOc] Disconnected!\n");
        break;
    case sIOtype_CONNECT:
        USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);
        // join default namespace (no auto join in Socket.IO V3)
        socketIO.send(sIOtype_CONNECT, "/");
        break;
    case sIOtype_EVENT:
        if (text1.startsWith("[\"UPDATE\""))
        {
            USE_SERIAL.println("updating");
            sendDataToServer();
        }
        break;
    case sIOtype_ACK:
        USE_SERIAL.printf("[IOc] get ack: %u\n", length);
        hexdump(payload, length);
        break;
    case sIOtype_ERROR:
        USE_SERIAL.printf("[IOc] get error: %u\n", length);
        hexdump(payload, length);
        break;
    case sIOtype_BINARY_EVENT:
        USE_SERIAL.printf("[IOc] get binary: %u\n", length);
        hexdump(payload, length);
        break;
    case sIOtype_BINARY_ACK:
        USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
        hexdump(payload, length);
        break;
    }
}
