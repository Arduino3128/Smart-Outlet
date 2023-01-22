#define BLYNK_TEMPLATE_ID "TEMPLATE_ID"
#define BLYNK_DEVICE_NAME "Desk Lights"
#define BLYNK_FIRMWARE_VERSION "1.7"
#define BLYNK_AUTH_TOKEN "AUTH_TOKEN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266_SSL.h>
#include <ArduinoJson.h>
#include "OTAflasher.h"

char auth[] = BLYNK_AUTH_TOKEN;
DynamicJsonDocument doc(2048);
String ret, msg, overTheAirURL;
bool messageReady = false;
char ssid[] = "rockyou.txt";
char pass[] = "No Bruteforcing";
char rc;

BlynkTimer timer;

BLYNK_CONNECTED()
{
    Blynk.syncAll();
    Blynk.virtualWrite(V10, "\n\n⚝Connected to Blynk Server!\n  ⤷Firmware Version: " + String(BLYNK_FIRMWARE_VERSION) + "\n");
}

BLYNK_WRITE(InternalPinDBG)
{
    if (String(param.asStr()) == "reboot")
    {
        Blynk.virtualWrite(V10, "⟲ Starting reboot sequence...");
        restartMCU();
    }
}

BLYNK_WRITE(V0)
{
    int value = param.asInt();
    if (value == 1)
        doc["Light"]["Manual"] = "on";
    else
        doc["Light"]["Manual"] = "off";
    Serial.print('<');
    serializeJson(doc, Serial);
    Serial.print('>');
    return;
}

BLYNK_WRITE(V1)
{
    int value = param.asInt();
    if (value == 1)
        doc["Light"]["Auto"] = "on";
    else
        doc["Light"]["Auto"] = "off";
    Serial.print('<');
    serializeJson(doc, Serial);
    Serial.print('>');
    return;
}

BLYNK_WRITE(V2)
{
    int value = param.asInt();
    if (value == 1)
        doc["Light"]["LDR"] = "on";
    else
        doc["Light"]["LDR"] = "off";
    Serial.print('<');
    serializeJson(doc, Serial);
    Serial.print('>');
    return;
}

BLYNK_WRITE(InternalPinOTA)
{
    overTheAirURL = param.asString();
    Blynk.logEvent("sys_ota", "OTA Firmware Upgrade Started!");
    Blynk.virtualWrite(V10, "⦿ Received Request for Update ⥯");
    Blynk.virtualWrite(V10, "⦿ Starting Upgrade Process....");
    Blynk.virtualWrite(V10, " ⤷ Firmware URL: " + overTheAirURL + "\n");
    ret = enterOTA(overTheAirURL);
    Blynk.virtualWrite(V10, ret);
}

void sendSensor()
{
    msg = "";
    messageReady = false;
    Serial.flush();
    while (Serial.available() && messageReady == false)
    {
        rc = Serial.read();
        if (rc == '<')
        {
            msg = Serial.readStringUntil('>');
            messageReady = true;
        }
        else
        {
            messageReady = false;
        }
    }
    if (messageReady)
    {
        DynamicJsonDocument RECV(2048);
        DeserializationError error = deserializeJson(RECV, msg);
        if (error)
        {
            // Blynk.virtualWrite(V10, "PARSING ERROR");
            messageReady = false;
            return;
        }
        float temp = RECV["Temperature"];
        float realfeel = RECV["Real Feel"];
        int humidity = RECV["Humidity"];
        Blynk.virtualWrite(V7, temp);
        Blynk.virtualWrite(V8, humidity);
        Blynk.virtualWrite(V9, realfeel);
    }
}
void setup()
{
    Serial.begin(115200);
    Blynk.begin(auth, ssid, pass);
    doc["Light"]["Auto"] = "off";
    doc["Light"]["Manual"] = "off";
    doc["Light"]["LDR"] = "off";
    timer.setInterval(2000L, sendSensor);
}

void loop()
{
    Blynk.run();
    timer.run();
}