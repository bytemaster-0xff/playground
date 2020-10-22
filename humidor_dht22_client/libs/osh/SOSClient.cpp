#include "OSHClient.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

namespace osh
{

#define HTTP_CHUNK_SIZE 64

class HttpChunkedRequest: public Print
{
private:
    Client* client;
    uint8_t buffer[HTTP_CHUNK_SIZE];
    size_t curLen;

public:
    HttpChunkedRequest(Client* client)
    {
        this->client = client;
        curLen = 0;
    }

    virtual ~HttpChunkedRequest() {}

    int connect(const char* server, int port)
    {
        curLen = 0;
		if (client->connected()) {
			Serial.println(F("Currently connected, disconnecting."));
			client->stop();
		}

        int retries = 10;
        while (!client->connect(server, port))
        {
            Serial.println(F("Connection failed"));
            if (--retries == 0)
            {
                Serial.println(F("Aborting"));
                return -1;
            }
            Serial.println(F("Retrying..."));
            delay(1000);
        }

		Serial.println("Connected");

#ifdef ESP8266
        ((WiFiClient*)client)->setNoDelay(true);
#endif

        return 0;
    }

    void startPost(const char* server, const char* path, const char* mimeType)
    {
        Serial.print(F("Sending POST request to "));
        Serial.println(path);
		
        client->print(F("POST "));
        
		client->print(path);
        client->println(F(" HTTP/1.1"));
        
		client->print(F("Host: "));
        client->println(server);
		
		client->print("Authorization: ");
		client->println("Basic YWRtaW46YWRtaW4=");

        client->print(F("Content-Type: "));		
        client->println(mimeType);
        
		client->println(F("Transfer-Encoding: chunked"));
        client->println();

		Serial.print(F("Request Sent"));

    }

    size_t write(uint8_t c)
    {
        if (curLen+1 > HTTP_CHUNK_SIZE)
            flush();
        buffer[curLen++] = c;
        return 1;
    }

    void flush()
    {
        client->println((int)curLen, 16);
        client->write(buffer, curLen);
        client->println();
        curLen = 0;
    }

    void end()
    {
        flush();
        client->println(0);
        client->println();	
    }

	void close()
	{
		client->stop();
	}
};


SOSClient::SOSClient(Client& client, const char* hostNameOrIp, int httpPort, const char* sosPath)
{
    this->client = &client;
    this->device = NULL;
    this->sendingSensor = NULL;
    this->server = hostNameOrIp;
    this->port = httpPort;
    this->path = sosPath;
}


void SOSClient::registerDevice(Device* device)
{
    this->device = device;

    // send InsertSensor request and get offering ID
    String offeringID = sendInsertSensor(device);

    // send InsertResultTemplate requests
    if (device->isSystem())
    {
        System* sys = (System*)device;
        Sensor** sensors = sys->getSensors();
        for (int i = 0; i < sys->getNumSensors(); i++)
        {
            Sensor* s = sensors[i];
            sendResultTemplate(offeringID.c_str(), s);
        }
    }
    else
    {
        sendResultTemplate(offeringID.c_str(), (Sensor*)device);
    }
}


String SOSClient::sendInsertSensor(Device* device)
{
    HttpChunkedRequest postReq(client);
    postReq.connect(server, port);
    postReq.startPost(server, path, "text/xml; charset=utf-8");

    // send insert sensor request
    XMLWriter w(&postReq);//&Serial);
    w.tagStart("InsertSensor");
    w.tagField("service", "SOS");
    w.tagField("version", "2.0.0");
    w.tagEnd(true, false);
    w.writeNode("procedureDescriptionFormat", "http://www.opengis.net/sensorml/2.0");
    w.tagOpen("procedureDescription");
    device->writeXML(w, false);
    w.tagClose();
    w.tagOpen("metadata");
    w.tagOpen("SosInsertionMetadata");
    w.writeNode("observationType", "");
    w.writeNode("featureOfInterestType", "");
    w.tagClose();
    w.tagClose();
    w.tagClose();
    postReq.end();

    // read offering ID
    String offeringID = readResponseTag("assignedOffering");
    Serial.print("Offering is ");
    Serial.println(offeringID);

    return offeringID;
}


void SOSClient::sendResultTemplate(const char* offeringID, Sensor* s)
{
    HttpChunkedRequest postReq(this->client);
    postReq.connect(server, port);
    postReq.startPost(server, path, "text/xml; charset=utf-8");

    XMLWriter w(&postReq);//&Serial);
    w.tagStart("InsertResultTemplate");
    w.tagField("service", "SOS");
    w.tagField("version", "2.0.0");
    w.tagEnd(true, false);
    w.tagOpen("proposedTemplate");

    w.tagStart("ResultTemplate");
#ifndef OSH_NOXMLNS
    char nsUri[40];
    buildUrl(OGC_NS_PREFIX, SWE_NS, nsUri);
    w.tagField(w.buildTagName(XMLNS_PREFIX, SWE_PREFIX), nsUri);
#endif
    w.tagEnd(true, false);

    w.writeNode("offering", offeringID);

    w.tagOpen("resultStructure");
    s->writeOutput(w);
    w.tagClose();

    w.tagOpen("resultEncoding");
    w.tagStart("TextEncoding");
    w.tagField("tokenSeparator", ",");
    w.tagField("blockSeparator", "&#x0D;");
    w.tagEnd(true, true);
    w.tagClose();

    w.tagClose();
    w.tagClose();
    w.tagClose();

    postReq.end();

    String templateID = readResponseTag("acceptedTemplate");
    s->setStreamID(templateID);
    Serial.print("Template ID is ");
    Serial.println(templateID);
}


String SOSClient::readResponseTag(const char* tagName)
{
    String tagValue;

    // wait until response is available or 5s timeout
    int i=0;
    while ((!client->available()) && (i < 500))
    {
        delay(10);
        i++;
    }

    // read the whole response and extract desired tag value
    while (client->peek() != -1)
    {
        String line = client->readString();
        int idx = line.indexOf(tagName);
        if (idx > 0)
        {
            idx += strlen(tagName) + 1;
            int idx2 = line.indexOf('<', idx);
            tagValue = line.substring(idx, idx2);
        }

        Serial.println(line);
    }

    return tagValue;
}


void SOSClient::pushInt(int val)
{
    if (measBuf.length() > 0)
        measBuf += ',';
    measBuf += val;
}


void SOSClient::pushUInt(unsigned int val)
{
    if (measBuf.length() > 0)
        measBuf += ',';
    measBuf += val;
}


void SOSClient::pushFloat(float val)
{
    if (measBuf.length() > 0)
            measBuf += ',';
    measBuf += val;
}


void SOSClient::pushDouble(double val)
{
    if (measBuf.length() > 0)
        measBuf += ',';
    measBuf += val;
}


void SOSClient::pushString(char* val)
{
    if (measBuf.length() > 0)
        measBuf += ',';
    measBuf += val;
}


void SOSClient::startMeasurement(Sensor* s)
{
    this->sendingSensor = s;
    this->measBuf = "";
}


void SOSClient::sendMeasurement()
{
    HttpChunkedRequest postReq(client);
    String url = String(path) + "?service=SOS&version=2.0&request=InsertResult&template=" + sendingSensor->getStreamID();
    url.replace("#", "%23"); // URL encode
    postReq.connect(server, port);
    postReq.startPost(server, url.c_str(), "text/plain");
    postReq.println(measBuf);
    postReq.end();
    Serial.println(measBuf);
	postReq.close();
}

} // end namespace osh
