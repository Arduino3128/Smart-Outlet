#include <Servo.h>
#include <ArduinoJson.h>
#include "DHT.h"

#define servo_pin 10
#define pir_pin 11
#define ldr_pin 9
#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
Servo light_servo;
String message = "";
int pir_val, ldr_val;
char rc;
bool trig = false, messageReady = false, ldr_trig = false, auto_trig = false, man_trig = false;
DynamicJsonDocument SEND(2048);

void setup()
{
    dht.begin();
    delay(500);
    pinMode(pir_pin, INPUT);
    pinMode(ldr_pin, INPUT);
    light_servo.attach(servo_pin);
    Serial3.begin(115200);
    Serial.begin(9600);
    Serial.println("Starting....");
    pinMode(13, OUTPUT);
    light_servo.write(110);
}

void DHTData()
{
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t))
    {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }
    float hic = dht.computeHeatIndex(t, h, false);
    SEND["Temperature"] = t;
    SEND["Real Feel"] = hic;
    SEND["Humidity"] = h;
    Serial3.print('<');
    serializeJson(SEND, Serial3);
    Serial3.print('>');
}

void turnon()
{
    light_servo.attach(servo_pin);
    light_servo.write(140);
    delay(500);
    light_servo.write(110);
    Serial.println("Motion Detected: Turning on Light!");
}
void turnoff()
{
    light_servo.attach(servo_pin);
    light_servo.write(80);
    delay(500);
    light_servo.write(110);
    Serial.println("Motion Ended: Turning off Light!");
}

void auto_lights()
{
    pir_val = digitalRead(pir_pin);
    if (pir_val == HIGH && trig == false)
    {
        turnon();
        trig = true;
    }
    else if (pir_val == LOW && trig == true)
    {
        turnoff();
        trig = false;
    }
}

void ldr_lights()
{
    ldr_val = digitalRead(ldr_pin);
    if (ldr_val == HIGH && trig == false)
    {
        turnon();
        trig = true;
    }
    else if (ldr_val == LOW && trig == true)
    {
        turnoff();
        trig = false;
    }
}

void LightData()
{
    messageReady = false;
    Serial3.flush();
    while (Serial3.available() && messageReady == false)
    {
        rc = Serial3.read();
        if (rc == '<')
        {
            message = Serial3.readStringUntil('>');
            Serial.println(message);
            messageReady = true;
        }
        else
        {
            messageReady = false;
        }
    }
    if (messageReady)
    {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, message);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            messageReady = false;
            return;
        }
        if (doc["Light"]["Manual"] == "on" && man_trig == false)
        {
            man_trig = true;
            turnon();
        }
        else if (doc["Light"]["Manual"] == "off" && man_trig == true)
        {
            man_trig = false;
            turnoff();
        }
        if (doc["Light"]["Auto"] == "off" && auto_trig == true)
        {
            auto_trig = false;
        }
        else if (doc["Light"]["Auto"] == "on" && auto_trig == false)
        {
            auto_trig = true;
        }
        if (doc["Light"]["LDR"] == "off" && ldr_trig == true)
        {
            ldr_trig = false;
        }
        else if (doc["Light"]["LDR"] == "on" && ldr_trig == false)
        {
            ldr_trig = true;
        }
    }
}

void loop()
{
    LightData();
    DHTData();
    light_servo.detach();
    if (auto_trig == true && man_trig == false)
        auto_lights();
    if (auto_trig == false && ldr_trig == true && man_trig == false)
        ldr_lights();
}