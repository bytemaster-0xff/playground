#include "./libs/adafruit/adafruit_sensor.h"
#include "./libs/adafruit/DHT.h"
#include "./libs/adafruit/DHT_U.h"
#include "./libs/osh/OSHClient.h"
#include <ESP8266WiFi.h>
#include <time.h>

const char ssid[] = "SLManCave";
const char password[] = "TheWolfBytes";
WiFiClient client;


uint32_t delayMS;

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


	Serial.println("Completed");
	Serial.println("================================");
	Serial.println("");

	delete sendClient;

	

  /* add main program code here */

}
