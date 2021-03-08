#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DATA 15
#define OLED_RESET     -1

//WIFI Credentials
const char* ssid = "Oceans 2.4GHz";
const char* password = "Amelie2208";

//MQTT Credentials
const char *mqtt_server = "driver.cloudmqtt.com";
const int mqtt_port = 18897;
const char *mqtt_user = "cotvfnff";
const char *mqtt_pass = "L2YawbHDgZ1n"; 

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
OneWire ourWire (DATA);
DallasTemperature sensor (&ourWire);

long lastMsg = 0;
char msg[50];
int value = 0;

void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message received from topic -> ");
  Serial.print(topic);
  Serial.print("/n");

  for(int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
  }

  Serial.println();
}

void reconnect(){
  while(!client.connected()){
    Serial.println("Trying to reconnect to MQTT Server...");
    String clientID = getRandomClientID();
    if(client.connect(clientID.c_str(),mqtt_user,mqtt_pass)){
      Serial.println("Connection to MQTT Server sucessfull!");
      client.publish("TestReconnect","Test message for reconnection.");
      client.subscribe("webClient");
    }else{
      Serial.println("Connection fail!");
      Serial.print(client.state());
      Serial.print("Trying again in 5 seconds..");
      delay(5000);
    }
  }
}

String getRandomClientID(){
  String clientID = "IoT_Temperature_Sensor";
  clientID = clientID + String(random(0xffff),HEX);
  return clientID;
}

void connectToWifi(){
  Serial.println();
  Serial.print("Connecting to...");
  Serial.println(ssid);
  WiFi.begin(ssid,password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println("WiFi ESP32 Status: " + String(WiFi.status()));
  }

  Serial.println("");
  Serial.println("Connected to red WiFi");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
}

void printTemperatureToMonitor(float temp){
  Serial.print(temp);
  Serial.println(" Grados Centrigrados");
}

void printTermperatureToScreen(float temp){
  display.setTextSize(4);
  display.setCursor(0,0);
  display.println(String(temp,1));
  display.display();
}

float getTemperatureFromDS18B20(){
  sensor.requestTemperatures();
  return sensor.getTempCByIndex(0);
}

void setup() {
  #ifdef BUILTIN_LED
  pinMode(BUILTIN_LED,OUTPUT);     // In case it's on, turn output off, sometimes PIN-5 on some boards is used for SPI-SS
  #endif
  Serial.begin(115200);
  connectToWifi();
  client.setServer(mqtt_server,mqtt_port);
  client.setCallback(callback);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("ERROR: Unabled to connect to OLED screen!"));
  }
  display.clearDisplay();
  display.setTextColor(WHITE,BLACK); 
  sensor.begin();
}

void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();

  if(millis() - lastMsg > 2000){
    lastMsg = millis();

    float temp = getTemperatureFromDS18B20();
    printTemperatureToMonitor(temp);
    printTermperatureToScreen(temp);
    
    String message = "Temperature --> " + String(temp);
    message.toCharArray(msg, 50);
    client.publish("temperature",msg);
    Serial.println("Message --> " + message + " --> sent!");
  }

}
