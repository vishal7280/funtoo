#line 1 "C:\\Users\\Sonu\\AppData\\Local\\Temp\\arduino_modified_sketch_236095\\game_comm.h"
#ifndef GAME_COMM_H
#define GAME_COMM_H

#include <WiFi.h>
#include <esp_now.h>
#include <FastLED.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

SoftwareSerial mySerial2(32, 33); // RX, TX for second DFPlayer Mini
DFRobotDFPlayerMini myDFPlayer;

// ========== Configuration ==========
#define NUM_PEERS 1
#define NUM_LEDS_score 105  // Total number of LEDs for five displays (21 LEDs each)

int Score1, Score2, Score3, Score4, Score5;
int val = 0, val2 = 50, val3 = 100, val4 = 150, val5 = 200;
int Gpass = 0;
CRGB leds_score[NUM_LEDS_score];  // Define LEDs strip for all displays
byte digits[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

uint8_t peerAddresses[NUM_PEERS][6] = {
// {0xFC, 0xE8, 0xC0, 0xC8, 0xCC, 0xA0}
{0xFC, 0xE8, 0xC0, 0xC9, 0x00, 0x5C}
//  {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0x03}
};
// ===================================

typedef struct struct_message {
  int id;
  int gameStatus;
  int score;
  int time;
} struct_message;

// Internal function
inline void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  struct_message receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  // Optional debug:
  Gpass = receivedData.gameStatus;
    Serial.printf("Received from %02X:%02X:%02X:%02X:%02X:%02X - ID: %d, Status: %d, Score: %d, Time: %d\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                receivedData.id, receivedData.gameStatus, receivedData.score, receivedData.time);
  
}

inline void setupGameComm() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    ESP.restart();
  }

  esp_now_register_recv_cb(OnDataRecv);

  for (int i = 0; i < NUM_PEERS; i++) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, peerAddresses[i], 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.printf("❌ Failed to add peer %02X:%02X:%02X:%02X:%02X:%02X\n",
                    peerAddresses[i][0], peerAddresses[i][1], peerAddresses[i][2],
                    peerAddresses[i][3], peerAddresses[i][4], peerAddresses[i][5]);
    } else {
      Serial.printf("✅ Added peer %02X:%02X:%02X:%02X:%02X:%02X\n",
                    peerAddresses[i][0], peerAddresses[i][1], peerAddresses[i][2],
                    peerAddresses[i][3], peerAddresses[i][4], peerAddresses[i][5]);
    }
  }
  FastLED.addLeds<WS2812B, 27, GRB>(leds_score, NUM_LEDS_score);
  FastLED.setBrightness(255);
  mySerial2.begin(9600);
  myDFPlayer.begin(mySerial2);
  Serial.println("Second DFPlayer Mini online.");
  myDFPlayer.volume(35); // Set initial volume (0-30)

}

inline void sendGameData(int fixedId, int status, int currentScore, int currentTime) {
  struct_message gameData = {fixedId, status, currentScore, currentTime};

  for (int i = 0; i < NUM_PEERS; i++) {
    esp_err_t result = esp_now_send(peerAddresses[i], (uint8_t *)&gameData, sizeof(gameData));
    if (result == ESP_OK) {
      Serial.printf("✅ Send success to %02X:%02X:%02X:%02X:%02X:%02X\n",
                    peerAddresses[i][0], peerAddresses[i][1], peerAddresses[i][2],
                    peerAddresses[i][3], peerAddresses[i][4], peerAddresses[i][5]);
    } else {
      Serial.printf("❌ Send error to %02X:%02X:%02X:%02X:%02X:%02X, code: %d\n",
                    peerAddresses[i][0], peerAddresses[i][1], peerAddresses[i][2],
                    peerAddresses[i][3], peerAddresses[i][4], peerAddresses[i][5], result);
    }
  }
}

// Display 1: LEDs 0-20
void Display1stScore() {
  for (int i = 0; i < 21; i++) leds_score[i] = CRGB::Black;

  int cursor;
  int Fd = (Score1 < 10) ? 1 : (Score1 < 100) ? 2 : 3;

  for (int i = 1; i <= Fd; i++) {
    int digit = 0;
    if (i == 1) {
      cursor = 14;
      digit = Score1 % 10;
    }
    else if (i == 2) {
      cursor = 7;
      digit = (Score1 / 10 % 10);
    }
    else {
      cursor = 0;
      digit = (Score1 / 100 % 10);
    }

    for (int k = 0; k <= 6; k++) {
      leds_score[cursor] = digits[digit][k] ? CRGB::Fuchsia : CRGB::Black;
      cursor++;
    }
  }
}

// Display 2: LEDs 21-41
void Display2ndScore() {
  for (int i = 21; i <= 41; i++) leds_score[i] = CRGB::Black;

  int cursor;
  int Fd = (Score2 < 10) ? 1 : (Score2 < 100) ? 2 : 3;

  for (int i = 1; i <= Fd; i++) {
    int digit = 0;
    if (i == 1) {
      cursor = 35;
      digit = Score2 % 10;
    }
    else if (i == 2) {
      cursor = 28;
      digit = (Score2 / 10 % 10);
    }
    else {
      cursor = 21;
      digit = (Score2 / 100 % 10);
    }

    for (int k = 0; k <= 6; k++) {
      leds_score[cursor] = digits[digit][k] ? CRGB::Red : CRGB::Black;
      cursor++;
    }
  }
}

// Display 3: LEDs 42-62
void Display3rdScore() {
  for (int i = 42; i <= 62; i++) leds_score[i] = CRGB::Black;

  int cursor;
  int Fd = (Score3 < 10) ? 1 : (Score3 < 100) ? 2 : 3;

  for (int i = 1; i <= Fd; i++) {
    int digit = 0;
    if (i == 1) {
      cursor = 56;
      digit = Score3 % 10;
    }
    else if (i == 2) {
      cursor = 49;
      digit = (Score3 / 10 % 10);
    }
    else {
      cursor = 42;
      digit = (Score3 / 100 % 10);
    }

    for (int k = 0; k <= 6; k++) {
      leds_score[cursor] = digits[digit][k] ? CRGB::Red : CRGB::Black;
      cursor++;
    }
  }
}

// Display 4: LEDs 63-83
void Display4thScore() {
  for (int i = 63; i <= 83; i++) leds_score[i] = CRGB::Black;

  int cursor;
  int Fd = (Score4 < 10) ? 1 : (Score4 < 100) ? 2 : 3;

  for (int i = 1; i <= Fd; i++) {
    int digit = 0;
    if (i == 1) {
      cursor = 77;
      digit = Score4 % 10;
    }
    else if (i == 2) {
      cursor = 70;
      digit = (Score4 / 10 % 10);
    }
    else {
      cursor = 63;
      digit = (Score4 / 100 % 10);
    }

    for (int k = 0; k <= 6; k++) {
      leds_score[cursor] = digits[digit][k] ? CRGB::Green : CRGB::Black;
      cursor++;
    }
  }
}

// Display 5: LEDs 84-104
void Display5thScore() {
  for (int i = 84; i <= 104; i++) leds_score[i] = CRGB::Black;

  int cursor;
  int Fd = (Score5 < 10) ? 1 : (Score5 < 100) ? 2 : 3;

  for (int i = 1; i <= Fd; i++) {
    int digit = 0;
    if (i == 1) {
      cursor = 98;
      digit = Score5 % 10;
    }
    else if (i == 2) {
      cursor = 91;
      digit = (Score5 / 10 % 10);
    }
    else {
      cursor = 84;
      digit = (Score5 / 100 % 10);
    }

    for (int k = 0; k <= 6; k++) {
      leds_score[cursor] = digits[digit][k] ? CRGB::Yellow : CRGB::Black;
      cursor++;
    }
  }
}


#endif  // GAME_COMM_H