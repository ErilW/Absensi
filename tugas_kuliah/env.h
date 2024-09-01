#pragma once
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

#define last_card "last-card-rfid"
#define SSID "SERIGAFE"
#define PASS "08312805"

#define API_KEY "AIzaSyApH2nMSCf8ROYut7cZr61MrqqkoS3amDs"
#define DATABASE_URL "https://absens-39dd0-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "erilsan28@uvers.ac.id"
#define USER_PASSWORD "Passw0rd"
// Firebase Object

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD); 
DefaultNetwork network;
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(network));
RealtimeDatabase Database;
AsyncResult aResult_no_callback;


String readSerialInput() {
  String input = "";
  while (true) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        // End of input
        break;
      }
      input += c;
    }
    delay(10); // Short delay to prevent rapid polling
  }
  input.trim(); // Remove any leading or trailing whitespace
  return input;
}

