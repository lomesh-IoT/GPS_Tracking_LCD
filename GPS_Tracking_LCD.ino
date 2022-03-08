#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <SPI.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET LED_BUILTIN // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//write wifi credentials
const char* ssid = "";
const char* password = "";

//write ip address of server where momqtt broker has been installed
const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient client(mqtt_server, 1883, espClient);

static const int RXPin = 4, TXPin = 5;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

DynamicJsonDocument doc(1024);


void setup()
{
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;); 
  }
  setup_wifi();
  

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      //Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial.available() > 0)
    if (gps.encode(Serial.read()))
      if (gps.location.isValid() && gps.time.isValid()) {
        doc["lat"] = String(gps.location.lat(), 6);
        doc["lon"] = String(gps.location.lng(), 6);
        char buffer1[512];
        serializeJson(doc, buffer1);

        client.publish("test_channel", buffer1);
        doc.clear();
        memset(buffer1, 0, sizeof buffer1);
      }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    //Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  WiFi.begin(ssid, password);
  int count = 0;
  display.clearDisplay();
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {


    display.setCursor(count, 8);
    display.println(".");
    display.display();
    count++;
    delay(500);
  }
  randomSeed(micros());
  display.clearDisplay();
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("IP: ");
  display.setCursor(16, 0);
  display.println(WiFi.localIP().toString());
  display.display();
  delay(1000);

}
