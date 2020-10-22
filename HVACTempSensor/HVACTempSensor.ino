#include "./libs/adafruit/adafruit_sensor.h"
#include "./libs/adafruit/DHT.h"
#include "./libs/OneWire.h"
#include "./libs/DallasTemperature.h"

#include "./libs/adafruit/DHT_U.h"
#include <ESP8266WiFi.h>

const char ssid[] = "SLManCave";
const char password[] = "TheWolfBytes";

#define DHT_PIN_IN D5
#define DHT_PIN_COOLED D6

WiFiClient client;

OneWire inOneWire(DHT_PIN_IN);
DallasTemperature temp_in_sensor(&inOneWire);

OneWire cooledOneWire(DHT_PIN_COOLED);
DallasTemperature temp_cooled_sensor(&cooledOneWire);

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
	/* add setup code here */
}

void loop(){
	float temp_in = temp_in_sensor.getTempFByIndex(0);
	if (temp_in < 32 || temp_in > 120) {
		temp_in = -1;
	}
	temp_in_sensor.requestTemperatures();


	float temp_cooled = temp_cooled_sensor.getTempFByIndex(0);
	if (temp_cooled < 32 || temp_cooled > 120) {
		temp_cooled = -1;
	}
	temp_cooled_sensor.requestTemperatures();


	if (isnan(temp_cooled)) {
		Serial.println("Error reading temperature!");
	}
	else {
		Serial.print("Temperature In: ");
		Serial.print(temp_cooled);
		Serial.println(" *C");
	}

	
	if (isnan(temp_in)) {
		Serial.println("Error reading temperature!");
	}
	else {
		Serial.print("Temperature Cooled: ");
		Serial.print(temp_in);
		Serial.println(" *C");
	}


	//hvac = methnod
	//deviceid = device

	if (client.connect("lagovista.softwarelogistics.iothost.net", 9000)) {
		client.print("GET /hvactemperatures/hvac?in=");
		client.print(temp_in);
		client.print("&cooled=");
		client.print(temp_cooled);
		client.println(" HTTP/1.1");
		client.println("Host: lagovista.softwarelogistics.iothost.net");
		client.println("Connetion: close");
		client.println();
		Serial.println("Request Sent");
	}
	else {
		Serial.println("Could not connected");
	}

	delay(60000);

	
	/* add main program code here */

}
