#include <string>
#define pii pair<int, int>
using namespace std;

#define B_RIGHT 21 // GPIO21 pin connected to button
#define B_UP 19
#define B_LEFT 18
#define B_DOWN 22

// WIRELESS CONNECTIONS - Based on code https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/

#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xC8, 0xF0, 0x9E, 0x9F, 0x9E, 0x30};
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void WirelessSetup() {
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void transmit(uint8_t* integer) {
  Serial.print("TEST OF SENDING");
  Serial.println(*integer);
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, integer, sizeof(integer));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
}

// END WIRELESS

void setup() {
  Serial.begin(9600);
  // initialize the pushbutton pin as an pull-up input
  pinMode(B_RIGHT, INPUT_PULLUP);
  pinMode(B_UP, INPUT_PULLUP);
  pinMode(B_LEFT, INPUT_PULLUP);
  pinMode(B_DOWN, INPUT_PULLUP);

  WirelessSetup();
}

int buttons[4] = {B_RIGHT, B_UP, B_LEFT, B_DOWN};
char fuckyou[4] = {'R', 'U', 'L', 'D'};

int lastState[4] = {HIGH, HIGH, HIGH, HIGH};
int currentState[4];

void loop() {
  // read the state of the switch/button:
  
  for (int i = 0; i < 4; i++) {
    currentState[i] = digitalRead(buttons[i]);

    if(lastState[i] == LOW && currentState[i] == HIGH) {
      Serial.println("The state changed from LOW to HIGH");
      Serial.println(fuckyou[i]);

      uint8_t res = fuckyou[i];
      transmit(&res);
    }

    // save the last state
    lastState[i] = currentState[i];
  }
}