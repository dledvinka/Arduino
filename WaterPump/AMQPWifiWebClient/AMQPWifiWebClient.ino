//https://github.com/mravinale/iot-arduino-wifi/blob/master/examples/mqtt_temperature/mqtt_temperature.ino

// Free mqtt client => http://mqtt.eclipse.org/ this one works
// Python client example => http://www.steves-internet-guide.com/python-mqtt-publish-subscribe/

// ----------------------------------------------------------------------------------------

/* mqtt_temperature
MIT license
written by Mariano Ravinale
*/

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
//#include <DHT.h>
#include <PubSubClient.h>

#define aref_voltage 3.3

#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER);

#define WLAN_SSID       "Thor"
#define WLAN_PASS       "kokos123"
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT11

char humidityBuffer[50];
char tempBuffer[50];

Adafruit_CC3000_Client client = Adafruit_CC3000_Client();
IPAddress mqttip(192, 168, 88, 254);
//PubSubClient mqttclient("mqtt.eclipse.org", 1883, client);
PubSubClient mqttclient(mqttip, 1883, client);

//DHT dht(DHTPIN, DHTTYPE);

void setup(void)
{
 Serial.begin(115200);

  Serial.println(F("Initialising the CC3000 ..."));
  if (!cc3000.begin()) {  Serial.println(F("Failed!")); while(1); }

  Serial.println(F("Deleting old connection profiles"));
  if (!cc3000.deleteProfiles()) {  Serial.println(F("Failed!"));  while(1);  }

  char *ssid = WLAN_SSID;
  Serial.println(F("Attempting to connect to ")); Serial.println(ssid);

  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) { Serial.println(F("Failed!")); while(1); }
  Serial.println(F("Connected!"));

  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()) { delay(100);}
  displayConnectionDetails();

  //dht.begin();
}

void loop(void) {

  float h = 57.0;
  float t = 22.9;

  char tempBuffer[50];
  String temp = dtostrf(t, 5, 2, tempBuffer);

  char humidityBuffer[50];
  String humidity = dtostrf(h, 5, 2, humidityBuffer);

  String humidityJson = "{\"value\": \""+humidity+"\"}";
  humidityJson.toCharArray(humidityBuffer, 50);

  String temperatureJson = "{\"value\": \""+temp+"\"}";
  temperatureJson.toCharArray(tempBuffer, 50);

  // are we still connected?
  if (!mqttclient.connected()) {  reconnect(); }

  Serial.println("Temperature: "+temp);
  mqttclient.publish("key/1qaz2wsx/device/arduino/tag/temperature",tempBuffer, true);
  delay(30000);
  Serial.println("Humidity: "+humidity);
  mqttclient.publish("key/1qaz2wsx/device/arduino/tag/humidity",humidityBuffer, true);

  mqttclient.loop();
  delay(30000);
}

/**************************************************************************/
/*!
    @brief  Tries to reconnect the CC3000's to the mqtt broker
*/
/**************************************************************************/
void reconnect() {
  // Loop until we're reconnected
  while (!mqttclient.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (mqttclient.connect("arduinoClient", "rbmqqtdev", "s12IRNF3jUDbpq")) {
      Serial.print("Connected!");
    } else {
      Serial.print("Failed!");
      Serial.print(mqttclient.state());
      Serial.println("trying again in 10 seconds");s
      delay(10000);
    }
  }
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
