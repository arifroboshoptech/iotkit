#include <Wire.h>
#include <WiFi.h>
#include <MQTT.h>
#include <WiFiManager.h>


#define WIFI_SSID             "Makesense_roboshop" //thange to your personal Access Point 
#define WIFI_PASSWORD         "makesense"
#define MQTT_HOST             "broker.hivemq.com"
#define MQTT_PREFIX_TOPIC     "iotkit2010335954/mqtt" //change to your own mqtt topic prefix
#define MQTT_PUBLISH_TOPIC    "/data"
#define MQTT_SUBSCRIBE_TOPIC  "/control"

#define TRIGGER_PIN 16
#define SOIL_PIN 36




int timeout = 120;






WiFiClient net;
MQTTClient mqtt(1024);

String wifi_status;

unsigned long lastMillis = 0;

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
 

  WiFi.mode(WIFI_STA);
  //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  WiFiManager wm; 
  wm.setConfigPortalTimeout(timeout); 

  if (!wm.startConfigPortal(WIFI_SSID)) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected");
    wifi_status = "ON ";
}

void messageReceived(String &topic, String &payload) {
  Serial.println("Incoming Status: " + payload);
  Serial.println();
}

void connectToMqttBroker(){
  Serial.print("Connecting to '" + String(MQTT_HOST) + "' ...");
  
  mqtt.begin(MQTT_HOST, net);
  mqtt.onMessage(messageReceived);
  

  String uniqueString = String(WIFI_SSID) + "-" + String(random(1, 98)) + String(random(99, 999));
  char uniqueClientID[uniqueString.length() + 1];
  
  uniqueString.toCharArray(uniqueClientID, uniqueString.length() + 1);
  
  while (!mqtt.connect(uniqueClientID)) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(" connected!");

  Serial.println("Subscribe to: " + String(MQTT_PREFIX_TOPIC) + String(MQTT_SUBSCRIBE_TOPIC));
  
  mqtt.subscribe(String(MQTT_PREFIX_TOPIC) + String(MQTT_SUBSCRIBE_TOPIC));

}


void setup() {
  
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  delay(2000);

  Serial.println("\nESP32 MQTT");
  //connectToWiFi();
  //connectToMqttBroker();

  Serial.println();

}

void loop() { 
 
  mqtt.loop();
  delay(10);  // <- fixes some issues with WiFi stability


  //Online Mode Triggered
  if (digitalRead(TRIGGER_PIN) == HIGH){
    Serial.println("Button Triggered");
 

    connectToWiFi();
    connectToMqttBroker();

    if (!mqtt.connected()) {
      connectToMqttBroker();
    }
     
  }

  if (WiFi.status() != WL_CONNECTED){
      wifi_status = "OFF";
    }  

    //Temperature and Humidity
    int soil_value= analogRead(SOIL_PIN);
    soil_value = map(soil_value,550,0,0,100);
    Serial.print("Moisture : ");
    Serial.print(soil_value);
    Serial.println("%");


    

    

    //send via mqtt

    if (millis() - lastMillis > 10000) {
    lastMillis = millis();

    String dataInJson = "{";
    dataInJson += "\"soilmoisture\":" + String(soil_value);
    dataInJson += "}";

    Serial.println("Data to Publish: " + dataInJson);
    Serial.println("Length of Data: " + String(dataInJson.length()));
    Serial.println("Publish to: " + String(MQTT_PREFIX_TOPIC) + String(MQTT_PUBLISH_TOPIC));
    
    mqtt.publish(String(MQTT_PREFIX_TOPIC) + String(MQTT_PUBLISH_TOPIC), dataInJson);
  }
  
  Serial.println();

  delay(2000);   

  

}
