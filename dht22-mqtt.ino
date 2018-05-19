// Code based on
// https://github.com/adafruit/DHT-sensor-library/blob/master/examples/DHTtester/DHTtester.ino
// https://gist.github.com/igrr/7f7e7973366fc01d6393
// https://github.com/iot-playground/Arduino/blob/master/ESP8266ArduinoIDE/DS18B20_temperature_sensor/DS18B20_temperature_sensor.ino

// esp8266 + dht22 + mqtt

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>

const char* ssid = "silakkaverkko";
const char* password = "affa123456";

WiFiClient client;

PubSubClient mqtt(client);

char* server = "192.168.1.52";

#define DHTPIN 14     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define REPORT_INTERVAL 30 // in sec

String clientName;
DHT dht(DHTPIN, DHTTYPE, 15);
//WiFiClient wifiClient;

float oldH ;
float oldT ;

void setup() {
  Serial.begin(38400);

  delay(20);
  
  mqtt.setServer(server, 1883);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);

  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName);

  dht.begin();
  oldH = -1;
  oldT = -1;
  
  
}
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //mqtt.publish("outTopic", "hello world");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  MQTT_connect();

  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  float hi = dht.computeHeatIndex(f, h);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hi);
  Serial.println(" *F");

  String payload = "";
  payload += t;
 
  if (t != oldT || h != oldH )
  {
  
    sendData(t, h);

    //tempFeed.publish(0);
    
    oldT = t;
    oldH = h;
  }

  int cnt = REPORT_INTERVAL;

  while (cnt--)
    delay(1000);
}

const char rootPath[] = "sensors/";
const char rootPath2[] = "sensors/";
void sendData(float temp, float humidity) {
  
  char tempC[100] = "";
  char humidityC[100] = "";

  const char endHum[] = ".humidity";
  const char endTemp[] = ".temp";
  
  
  dtostrf(temp, 2, 2, tempC);
  dtostrf(humidity, 2, 2, humidityC);

  char cName[18] = "";
  clientName.toCharArray(cName, 18);
  char tempPath[40] = "sensors/";
  //strcat(tempPath, rootPath);
  strcat(tempPath, cName);
  strcat(tempPath, endTemp);

  char humidityPath[40] = "sensors/";

  //strcat(humidityPath, rootPath2);
  strcat(humidityPath, cName);
  strcat(humidityPath, endHum);

  Serial.print("sending data for ");
  Serial.println(tempPath);
    if (mqtt.connected()){
        char p[50];
        //payload.toCharArray(p, 50);
        Serial.println("MQTT connected");
          mqtt.publish(tempPath, tempC);
          mqtt.publish(humidityPath, humidityC);
          Serial.print("values saved: ");
          Serial.println(tempC);
          Serial.println(tempPath);
          Serial.println(humidityPath);

        /*
        if (tempFeed.publish(temp)){
          Serial.println("temp saved");
         } else {
          Serial.println("temp not saved!");
          }
        if (humFeed.publish(humidity)){
         Serial.println("humidity saved");
         }

         */
      } else {
        Serial.println("MQTT not connected");
      }
    
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  
  uint8_t retries = 3;
  char cName[12];
  clientName.toCharArray(cName, 12);

  while (!mqtt.connected()) { // connect will return 0 for connected
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.connect(cName);
       //mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    //if (i < 5)
    //  result += ':';
  }
  return result;
}
