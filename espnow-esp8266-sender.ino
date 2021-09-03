/*
 Welcome to espnow!! 
 This sketch finds gateway channel and mac at its own.
 Trick is to span an AP at gateway, sender scans the network and finds channel and mac.
*/
 
#include <ESP8266WiFi.h>
#include <espnow.h>

// Enable FEATURE_ADC_VCC to measure supply voltage using the analog pin
// Please note that the TOUT (A0) pin has to be disconnected in this mode
// Use the "System Info" device to read the VCC value
#define FEATURE_ADC_VCC true
#if FEATURE_ADC_VCC
ADC_MODE(ADC_VCC);
#endif

// Turn on debug statements to the serial output
#define  DEBUG  1

#if  DEBUG
#define PRINT(s, x) { Serial.print(F(s)); Serial.print(x); Serial.println(); }
#define PRINTS(x) Serial.print(F(x))
#define PRINTLN(x) Serial.println(F(x))
#define PRINTD(x) Serial.println(x, DEC)

#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTLN(x)
#define PRINTD(x)

#endif

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 2
//topic will start with basename/mac/
char topic_basename[] = "myhome";


uint8_t* macAddress_receiver;   //receivers mac address will be stored in it.
char my_mac_charArray[13];      //Array to store own mac address

 
// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int id; //BOARD_ID to identify who sent message. note that I am using mac address in mqtt topic so BOARD_ID is redundant info.
  char a[50]; //mqtt topic, size should be more than longest topic
  char b[20]; // value of mqtt topic; size should be more than longest value
  unsigned int messageId;
} struct_message;
 
// Create a struct_message called myData
struct_message myData;

unsigned int messageId = 0;

// Insert your SSID
constexpr char WIFI_SSID[] = "ESPnowAP";


//find WiFi channel between espnow receiver and  gateway router,Also get mac of espnow receiver
int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
    for (uint8_t i=0; i<n; i++) {
      if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
        macAddress_receiver = WiFi.BSSID(i);
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}

 
// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
  
void setup() {

   #if  DEBUG
  Serial.begin(115200);     // Init Serial Monitor
  #endif

  WiFi.persistent( false ); // for time saving

 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  int32_t channel = getWiFiChannel(WIFI_SSID);
  #if  DEBUG
  WiFi.printDiag(Serial); // define DEBUG 0 to verify channel number before
  #endif
  wifi_promiscuous_enable(1);
  wifi_set_channel(channel);
  wifi_promiscuous_enable(0);
  #if  DEBUG
  WiFi.printDiag(Serial); // define DEBUG 0 to verify channel change after
  #endif  
     
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    PRINTLN("Error initializing ESP-NOW");
    return;
  }
 
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
   
  // Register peer
    //esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    esp_now_add_peer(macAddress_receiver, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  //find own mac address. mac will be included in mqtt topic. this will uniqly identify the sender device.
  String my_mac="";
  //  my_mac = WiFi.macAddress();
  //  Serial.println(my_mac);
  my_mac = getMacAddress();
  PRINT("my_mac:", my_mac);
  //Convert String to CharArray for publishing 
  my_mac.toCharArray(my_mac_charArray, 13); //mac has 12 char + 1 for string termination 
    
}
  
void loop() {

  //read the VCC value like this
  //WeMos has, just as the NodeMCU board, 2 resistors on the ADC/Tout pin, 100k to ground and 220k to the external ADC pin.
  //This means the ADC is always loaded with 100k which is not allowed when measuring VCC-internal. 
  //It results in a measured value being about 10% too low. Correction it is easy, in Optional Settings the field Formula needs this formula: %value%*1.1
  #if FEATURE_ADC_VCC
  float vcc = ESP.getVcc()*1.1 / 1000.0;
  //we need CharArray
  String vccS = String(vcc,3);
  PRINT("vccS:",vccS);
  //Convert String to CharArray for publishing 
  int vccS_len = vccS.length() + 1;
  char vccS_charArray[vccS_len];
  vccS.toCharArray(vccS_charArray, vccS_len);
  #endif

    
// Set variables to send voltage
//topic: myhome/a020a90e9dcb/vcc
//value: 3.38

  myData.id = BOARD_ID;
  strcpy(myData.a, topic_basename);
  strcat(myData.a,"/");
  strcat(myData.a,my_mac_charArray);
  strcat(myData.a,"/vcc");
  strcpy(myData.b, vccS_charArray);
  myData.messageId = messageId++;

   PRINT("topic:",myData.a);
   //Serial.println(myData.a);
   PRINT("value:",myData.b);
   //Serial.println(myData.b);
   //Serial.println();
  // Send message via ESP-NOW
   esp_now_send(macAddress_receiver, (uint8_t *) &myData, sizeof(myData));
  
  delay(2000);
  //ESP.deepSleep(300 * 1000000); //To put in deep sleep, comment delay and uncomment this line.
 
}

//Function to get mac address
String getMacAddress() {
byte mac[6];

WiFi.macAddress(mac);
String cMac = "";
for (int i = 0; i < 6; ++i) {
if (mac[i]<0x10) {cMac += "0";}
cMac += String(mac[i],HEX);
if(i<5)
cMac += ""; // put : or - if you want byte delimiters
}
//cMac.toUpperCase();
return cMac;
}
