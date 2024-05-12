#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <string>
#include <stdexcept>

using namespace std::string_literals;
// Update these with values suitable for your network.

const char *ssid = "Avenger";
const char *password = "123aka321";
const char *mqtt_server = "broker.mqtt-dashboard.com";
const char *id = "acing";
int *value = 0;

WiFiClient espClient;
PubSubClient client(espClient);
SoftwareSerial GPS_SoftSerial(D2, D1);
TinyGPSPlus gps;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }
  if ((char)payload[0] == '0')
  {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("cat-gps", "hello world");
      // ... and resubscribe
      client.subscribe("cat-gps");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void gps_loop()
{
  if (gps.location.isValid())
  {
    String payload = "{\"id\":\"acing\",\"type\":\"gps\",\"data\":{\"lat\":" + String(gps.location.lat()) + ",\"lng\":" + String(gps.location.lng()) + ",\"status\":true}}";
    Serial.println(payload);
    client.publish("cat-gps", payload.c_str());
  }
  else
  {
    String payload = "{\"id\":\"acing\",\"type\":\"gps\",\"data\":{\"lat\":0,\"lng\":0,\"status\":false}}";
    client.publish("cat-gps", payload.c_str());
    Serial.println(payload);
  }
}

static void smartDelay(unsigned long ms)
{
unsigned long start = millis();
do 
{
while (GPS_SoftSerial.available())  /* Encode data read from GPS while data is available on serial port */
  gps.encode(GPS_SoftSerial.read());
/* Encode basically is used to parse the string received by the GPS and to store it in a buffer so that information can be extracted from it */
} while (millis() - start < ms);
gps_loop();
}


void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  smartDelay(1000);
  
}

