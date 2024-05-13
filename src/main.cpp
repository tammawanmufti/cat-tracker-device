#include<ESP8266WiFi.h>
#include<ESP8266WebServer.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "html.h"
#include <PubSubClient.h>


ESP8266WebServer server(80);
SoftwareSerial GPS_SoftSerial(D1,D2);/* (Rx, Tx) */
TinyGPSPlus gps; 
WiFiClient wifiClient;
PubSubClient client(wifiClient);

const char* ssid = "Avenger";         /*Enter Your SSID*/
const char* password = "123aka321"; /*Enter Your Password*/
const char *mqtt_server = "broker.mqtt-dashboard.com";
const char *id = "acing";

IPAddress local_IP(192, 168, 1, 99);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

volatile float minutes, seconds;
volatile int degree, secs, mins;

double lat_val, lng_val, alt_m_val;
uint8_t hr_val, min_val, sec_val;
bool loc_valid, alt_valid, time_valid;

void wifi_setup()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(local_IP, gateway, subnet);

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
      client.publish("cat-gps", "acing connected");
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

void MainPage(){
String _html_page = html_page;              /*Read The HTML Page*/
server.send(200, "text/html", _html_page);  /*Send the code to the web server*/
}

void webGPS() {
String sensor_val = "[\""+String(lat_val)+"\",\""+String(lng_val)+"\"]";
server.send(200, "text/plane", sensor_val);
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
}

void DegMinSec( double tot_val)   /* Convert data in decimal degrees into degrees minutes seconds form */
{  
degree = (int)tot_val;
minutes = tot_val - degree;
seconds = 60 * minutes;
minutes = (int)seconds;
mins = (int)minutes;
seconds = seconds - minutes;
seconds = 60 * seconds;
secs = (int)seconds;
}

void server_setup(){
  server.on("/", MainPage);           /*Display the Web/HTML Page*/
  server.on("/readgps", webGPS);       /*Display the updated Latitude and Longitude value*/
  server.begin();                    /*Start Server*/
}

void mqtt_setup(){
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


void setup() {
Serial.begin(9600);/*Set the baudrate to 115200*/
GPS_SoftSerial.begin(9600); /* Define baud rate for software serial communication */
wifi_setup();
server_setup();
mqtt_setup();
}

void loop() {
  server.handleClient();
  if (!client.connected())
  {
    reconnect();
  }
  
  smartDelay(1000);  /* Generate precise delay of 1ms */
  lat_val = gps.location.lat(); /* Get latitude data */
  loc_valid = gps.location.isValid(); /* Check if valid location data is available */
  lng_val = gps.location.lng(); /* Get longtitude data */

  if (!loc_valid)
  {          
    Serial.println("Location Invalid");
  }
  else
  {
    Serial.print("Latitude in Decimal Degrees : ");
    Serial.println(lat_val, 6);
    Serial.print("Longitude in Decimal Degrees : ");
    Serial.println(lng_val, 6);
    String payload = "{\"id\":\"acing\",\"type\":\"gps\",\"data\":{\"lat\":" + String(lat_val) + ",\"lng\":" + String(lng_val) + ",\"status\":true}}";
    client.publish("cat-gps", payload.c_str());
  }
}

