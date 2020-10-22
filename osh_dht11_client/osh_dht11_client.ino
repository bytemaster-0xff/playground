#include "./libs/adafruit/adafruit_sensor.h"
#include "./libs/adafruit/DHT.h"
#include "./libs/adafruit/DHT_U.h"
#include "./libs/osh/OSHClient.h"
#include <ESP8266WiFi.h>
#include <time.h>

const char ssid[] = "SLManCave";
const char password[] = "TheWolfBytes";
WiFiClient client;

osh::Sensor s1;
osh::OSHClient* sos;

uint32_t delayMS;


static const char TEMP_URI[] PROGMEM = "http://sensorml.com/ont/swe/property/AirTemperature";
static const char RELH_URI[] PROGMEM = "http://sensorml.com/ont/swe/property/RelativeHumidity";

#define DHTPIN            14

//#define DHTTYPE           DHT11     // DHT 11 
#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
	Serial.begin(115200);

	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	//dht.begin();

	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.begin(ssid, password);

	Serial.println();
	Serial.println("WiFi connected");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
	Serial.println("\nWaiting for time");
	while (!time(nullptr)) {
		Serial.print(".");
		delay(1000);
	}

	s1.setUniqueID("urn:osh:esp8266:dht:attic");
	s1.setName("Attic Temperature Sensor");
	s1.addTimeStampOBC("ms");
	s1.addMeasurement("temp", TEMP_URI, "Cel", "Air Temperature");
	s1.addMeasurement("relh", RELH_URI, "%", "Relative Humidity");

	osh::SOSClient *registrationClient = new osh::SOSClient(client, "10.1.1.244", 8181, "/sensorhub/sos");
	registrationClient->registerDevice(&s1);
	delete registrationClient;

	Serial.println();
	Serial.println("Sensor Registered");
	
  /* add setup code here */

}

void loop()
{
	// Delay between measurements.
	delay(3000);
	
	time_t now = time(nullptr);
	if (now) {
		Serial.println(now);
	}
	tm *tm = gmtime(&now);

	Serial.println(ctime(&now));
	
	osh::SOSClient *sendClient = new osh::SOSClient(client, "10.1.1.244", 8181, "/sensorhub/sos");
	// send temp and pressure measurements

	// Get temperature event and print its value.
	
	float temperature = dht.readTemperature();
	if (isnan(temperature)) {
		Serial.println("Error reading temperature!");
	}
	else {
		Serial.print("Temperature: ");
		Serial.print(temperature);
		Serial.println(" *C");
	}

	//sos->pushFloat(event.temperature); // add random temperature value										// Get humidity event and print its value.
	float relative_humidity = dht.readHumidity();
	if (isnan(relative_humidity)) {
		Serial.println("Error reading humidity!");
	}
	else {
		Serial.print("Humidity: ");
		Serial.print(relative_humidity);
		Serial.println("%");
	}	
	//sos->pushFloat(event.relative_humidity);  // add random pressure value

	sendClient->startMeasurement(&s1);
	sendClient->pushUInt(now);
	sendClient->pushFloat(temperature);  // add random pressure value
	sendClient->pushFloat(relative_humidity);  // add random pressure value
	sendClient->sendMeasurement();

	Serial.println("Completed");
	Serial.println("================================");
	Serial.println("");

	delete sendClient;

	

  /* add main program code here */

}
