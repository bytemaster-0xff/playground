#include <SoftwareSerial.h> // Include software serial library, ESP8266 library dependency
#include <ESP8266WiFi.h>
#include <Ethernet.h>


WiFiServer  server(80);

void setup() {
	Serial.begin(115200);
	Serial.println("Hello World");

	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println("CasaDeWolf");
	
	/* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
	would try to act as both a client and an access-point and could cause
	network-issues with your other WiFi-devices on your WiFi-network. */
	/*	WiFi.mode(WIFI_STA);
	WiFi.begin("CasaDeWolf", "TheWolfBytes");

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.print("IP address: ");


	server.begin();
	Serial.print("Server Started Listening on: ");
	Serial.print(WiFi.localIP());
	Serial.print(":");
	Serial.print("9001");
	*/
}

void loop() {
	// Check if a client has connected
	/*WiFiClient client = server.available();
	if (!client) {
		return;
	}

	// Wait until the client sends some data
	Serial.println("new client");
	while (!client.available()) {
		delay(1);
	}

	// Read the first line of the request
	String req = client.readStringUntil('\r');
	Serial.println(req);
	client.flush();

	// Match the request
	int val;
	if (req.indexOf("/gpio/0") != -1)
		val = 0;
	else if (req.indexOf("/gpio/1") != -1)
		val = 1;
	else {
		String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Invalid request";
		s += "</html>\n";
		client.print(s);
		Serial.println("Simple Message");
		client.stop();
		return;
	}

	// Set GPIO2 according to the request
	digitalWrite(2, val);

	client.flush();

	// Prepare the response
	String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
	s += (val) ? "high" : "low";
	s += "</html>\n";

	// Send the response to the client
	client.print(s);
	delay(1);
	Serial.println("Client disonnected");
	*/
}