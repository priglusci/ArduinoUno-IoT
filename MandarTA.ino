#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

#define DHTPIN D3    // Connect Data pin of DHT to D2
int led = D4;     // Connect LED to D5
#define LDRPIN A0
#define DHTTYPE    DHT22
DHT dht(DHTPIN, DHTTYPE);

//#define ON_Board_LED 2  // On board LED, indicator when connecting to a wifi router

const char* ssid = "Priglusci";   // Your wifi name
const char* password = "parfumhandal"; // Your wifi password

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; // Create a WiFiClientSecure object

// Google spreadsheet script ID
String GAS_ID = "AKfycbx3fsX49Lc5VBEJAl1ZyQyShuIkSW6CFW6eZ5HbQDqWoKV28e5CC_tZ0fHfgJsA9DuUWA";

void setup() {
  Serial.begin(9600);

  dht.begin();
  pinMode(led,OUTPUT);

  WiFi.begin(ssid, password); // Connect to your WiFi router
  Serial.println("");

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  //----------------------------------------
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  float f = analogRead(LDRPIN);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(t) || isnan(h) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  String Temp = "Temperature : " + String(t) + " ";
  String Humi = "Humidity : " + String(h) + " ";
  String Cahaya = "Cahaya : " + String(f) + " ";
  Serial.print(Temp);
  Serial.print(Humi);
  Serial.print(Cahaya);

  sendData(t, h, f); // Call the sendData subroutine
  delay(100);
}

// Subroutine for sending data to Google Sheets
void sendData(float tem, float hum, float cahaya) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Processing data and sending data
  String string_temperature =  String(tem);
  String string_humidity =  String(hum);
  String string_cahaya =  String(cahaya);
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity + "&cahaya="+ string_cahaya ;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
}