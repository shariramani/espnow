/*
  Some part of code taken from
  https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/
  Board Tested: wemos Lolin32
*/

#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"



// Replace with your WiFi network credentials (STATION)
const char* ssid = "YOUR_WiFi_SSID";
const char* password = "YOUR_WiFi_PASSWORD";

//// Replace with your network credentials (AP)
const char* ssidAP     = "ESPnowAP";
const char* passwordAP = "123456789";

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  char a[50]; //mqtt topic
  char b[20]; // value of mqtt topic
  unsigned int messageId;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  
    // Copies the sender mac address to a string
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("id: ");
  Serial.println(myData.id);
  Serial.print("mqtt topic: ");
  Serial.println(myData.a);
  Serial.print("value: ");
  Serial.println(myData.b);
  Serial.print("messageId: ");
  Serial.println(myData.messageId);
  Serial.println();
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

    //Print MAC address
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  
  // Set device as a Wi-Fi Station
  //WiFi.mode(WIFI_STA);



  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  
  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

//// Set device as a Wi-Fi AP (espnow nodes will get channel from this AP)
  Serial.print("Configuring access point...");
  WiFi.softAP(ssidAP, passwordAP);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);






  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

}
