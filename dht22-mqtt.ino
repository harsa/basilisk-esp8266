// Code based on
// https://github.com/adafruit/DHT-sensor-library/blob/master/examples/DHTtester/DHTtester.ino
// https://gist.github.com/igrr/7f7e7973366fc01d6393
// https://github.com/iot-playground/Arduino/blob/master/ESP8266ArduinoIDE/DS18B20_temperature_sensor/DS18B20_temperature_sensor.ino

// esp8266 + dht22 + mqtt

#define AIO_SERVER      "192.168.1.52"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "harsa"
#define AIO_KEY         "f52f8396a2cb44bb93b51059700ff0cc"


#include <DHT.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

const char* ssid = "silakkaverkko";
const char* password = "affa123456";

const char TEMPERATURE_FEED[] PROGMEM = AIO_USERNAME "/feeds/temperature1";
const char feedName[] = "sensor2";

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish  tempFeed = Adafruit_MQTT_Publish(&mqtt, "sensors/sensor2.temp");
Adafruit_MQTT_Publish  humFeed =  Adafruit_MQTT_Publish(&mqtt, "sensors/sensor2.humidity");

char* topic = "sensors/sensor2.temp";
char* server = "192.168.1.52";
char* hellotopic = "hello-topic";

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

#define DHTPIN 14     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define REPORT_INTERVAL 30 // in sec

String clientName;
DHT dht(DHTPIN, DHTTYPE, 15);
//WiFiClient wifiClient;
//PubSubClient client(server, 1883, callback, wifiClient);

float oldH ;
float oldT ;

void setup() {
  Serial.begin(38400);
  Serial.println("DHTxx test!");
  delay(20);

  EEPROM.put(0, "hello");


  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
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

void loop() {
  MQTT_connect();
  
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(200))) {
  }

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


void sendData(float temp, float humidity) {
    if (mqtt.connected()){
        char p[50];
        //payload.toCharArray(p, 50);
        Serial.println("MQTT connected");
        if (tempFeed.publish(temp)){
          Serial.println("temp saved");
         } else {
          Serial.println("temp not saved!");
          }
        if (humFeed.publish(humidity)){
         Serial.println("humidity saved");
         }
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
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
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
    if (i < 5)
      result += ':';
  }
  return result;
}
