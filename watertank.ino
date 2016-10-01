/*
  This a simple example of the aREST Library for the ESP8266 WiFi chip.
  See the README file for more details.

  Written in 2015 by Marco Schwartz under a GPL license.
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
const char* ssid = "HAPPYNET_EXT";
const char* password = "wirelessLife2";

#define LISTEN_PORT           80
#define TRIGGER               5
#define ECHO                  4
#define TANK_HEIGHT           230   // cm
#define TANK_MIN              1     // cm
#define TANK_DIAMETER         360   // cm

int distance = 0;
float percent = 0;
int volume = 0;


static const unsigned long REFRESH_INTERVAL = 2000; // ms
static unsigned long lastRefreshTime = 0;
  
// Create an instance of the server
WiFiServer server(LISTEN_PORT);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  rest.variable("distance", &distance);
  rest.variable("percent", &percent);
  rest.variable("volume", &volume);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("1");
  rest.set_name("watertank");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  // set up the range finder
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
}

void loop() {


  // restart the esp if the wifi drops out
  if (WiFi.status() == WL_DISCONNECTED) {
    ESP.reset();
  }

  // only update the value every 2 seconds
  if(millis() - lastRefreshTime >= REFRESH_INTERVAL) {
    lastRefreshTime += REFRESH_INTERVAL;

    updateDistance();
  }
 
  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);

}

void updateDistance() {

  Serial.println("Taking reading");

  // update the sensor value
  distance = 0;
  
  int total = 0;
  int current = 0;
  int count = 0;

  for (int i=0; i < 5; i++) {
    current = getDistance(); 

    if (current > TANK_HEIGHT || current < TANK_MIN) {
      Serial.println(": out of range - ignoring.");
    } else {
      total+=current;  
      count++;
    }
  }
  distance = total/count;
  percent = 100 - (distance / (float)TANK_HEIGHT) * 100/1;
  volume =  (PI*pow((TANK_DIAMETER/2),2) * (TANK_HEIGHT-distance) / 1000); 

  Serial.println(distance);
}


long getDistance() {

  long duration, distance;
  digitalWrite(TRIGGER, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(TRIGGER, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration/2) / 29.1;
  
  return distance;
}
