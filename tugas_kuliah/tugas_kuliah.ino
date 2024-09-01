#include <Preferences.h>
#include "env.h"
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>

#define SS_PIN 4
#define RST_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
Preferences pref;

bool is_admin = false;

// predefine function
void connect_wifi();
void firebase_init();
String read_badge();
bool is_card_in_database(String badge);
String get_data(String badge, String type_key);
bool writing_data(String badge);



void setup() {
  SPI.begin();
  rfid.PCD_Init();
  Serial.begin(115200);
  Serial.setTimeout(100);
  Serial.setRxTimeout(100);
  pref.begin("absensi", false);

  // built in function
  connect_wifi();
  firebase_init();

  String is_any = pref.getString(last_card, "");
  if (is_any != "") {
    Serial.println("Last Card Detected : " + String(is_any));
    // TODO  : Update File ke RTDB
    pref.putString(last_card, "");
  }
}



void loop() {
  authHandler();
  Database.loop();

  // tambah data baru menggunakan serial
  if (is_admin) {
    String badge = read_badge();

    if (badge == "") {
      return;
    }

    if (is_card_in_database(badge)) {
      show_lcd("Card Already Registed");
      is_admin = false;
      return;
    }

    Serial.print("Masukkan nama: ");
    String name  = readSerialInput();
    Serial.println(name);

    Serial.flush();
    delay(1000);
    Serial.print("Masukkan nim: ");
    String nim = readSerialInput();

    Serial.println(nim);
    delay(1000);
    Serial.flush();
    if (nim != "" && name != "") {
      if (add_new_card(badge, name, nim, "Mahasiswa")) {
        show_lcd("Succesfuly Add new data!");
      }
    } else {
      show_lcd("Failed Add new data");
    }

    is_admin = false;
  }

  String badge = read_badge();
  // check ada deteksi badge atau tidak
  if (badge == "") {
    return;
  }

  if (badge == pref.getString(last_card, "")) {
    return;
  } else {
    pref.putString(last_card, badge);
  }

  bool exist = is_card_in_database(badge);
  if (!exist) {
    show_lcd("Unregister Card !!!");
  } else {
    show_lcd(get_data(badge, "name"));
    show_lcd(get_data(badge, "nim"));
    String role = get_data(badge, "role");
    Serial.println(role);

    if (role.equalsIgnoreCase("admin")) {
      is_admin = true;
    }else{
      writing_data(badge);
    }
  }

  delay(100);
}




String read_badge() {
  String badge = "";
  String inStringHex = "";
  if (!rfid.PICC_IsNewCardPresent())
    return badge;

  if (!rfid.PICC_ReadCardSerial())
    return badge;

  Serial.print("UID tag :");
  unsigned char revUid[rfid.uid.size];
  for (byte i = 0; i < rfid.uid.size; i++) {
    inStringHex += String(rfid.uid.uidByte[rfid.uid.size - (i + 1)], HEX);
    delay(10);
  }
  inStringHex.toUpperCase();
  badge = strtoul(inStringHex.c_str(), NULL, 16);

  if (badge.length() < 10) {
    int diff = 10 - badge.length();
    String a = "";
    for (int j = 0; j < diff; j++) {
      a = a + "0";
    }
    badge = a + badge;
  }
  return badge;
}


void connect_wifi() {
  WiFi.begin(SSID, PASS);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
}

void firebase_init() {
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  Serial.println("Initializing app...");
  ssl_client.setInsecure();
  initializeApp(aClient, app, getAuth(user_auth), aResult_no_callback);
  authHandler();
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
  aClient.setAsyncResult(aResult_no_callback);
}

bool is_card_in_database(String badge) {
  bool a = Database.existed(aClient, ("/name/" + badge));
  if (aClient.lastError().code() == 0) {
    return a;
  } else {
    printError(aClient.lastError().code(), aClient.lastError().message());
    return false;
  }
}

String get_data(String badge, String type_key) {
  String a = ("/" + type_key + "/" + badge);
  String v3 = Database.get<String>(aClient, a);
  if (aClient.lastError().code() == 0) {
    return v3;
  } else {
    printError(aClient.lastError().code(), aClient.lastError().message());
    return "";
  }
}


bool writing_data(String badge) {
  JsonWriter writer;
  object_t ts_json;
  writer.create(ts_json, ".sv", "timestamp");  // -> {".sv": "timestamp"}
  bool status = Database.push(aClient, ("/waktu/" + badge), ts_json);
  if (status) {
    return true;
  } else {
    printError(aClient.lastError().code(), aClient.lastError().message());
    return false;
  }
}


void show_lcd(String text) {
  Serial.println(text);
}


// builtin firebase handler
void authHandler() {
  // Blocking authentication handler with timeout
  unsigned long ms = millis();
  while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000) {
    JWT.loop(app.getAuth());
    printResult(aResult_no_callback);
  }
}

void printResult(AsyncResult &aResult) {
  if (aResult.isEvent()) {
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
  }

  if (aResult.isDebug()) {
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  }

  if (aResult.isError()) {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  }
}

void printError(int code, const String &msg) {
  Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}


bool add_new_card(String badge, String nama, String nim, String role) {
  JsonWriter writer;
  object_t ts_time;

  writer.create(ts_time, ".sv", "timestamp");  // -> {".sv": "timestamp"}

  bool status_timestamp = Database.push(aClient, ("/waktu/" + badge), ts_time);
  bool status_nama = Database.set(aClient, ("/name/" + badge), nama);
  bool status_nim = Database.set(aClient, ("/nim/" + badge), nim);
  bool status_role = Database.set(aClient, ("/role/" + badge), role);
  bool status_activate = Database.set(aClient, ("/status/" + badge), true);

  if (status_nama && status_nim) {
    return true;
  } else {
    printError(aClient.lastError().code(), aClient.lastError().message());
    return false;
  }
}


