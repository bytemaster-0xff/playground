#ifndef OSH_H
#define OSH_H

#ifndef OSH_NOXML
#include "XMLWriter.h"
#endif

#include <Client.h>

// get/set macro
#define OSH_GETSETVAR(type, name) \
protected: \
    type name; \
public: \
    type& get##name() { return name; } \
    void set##name(type newval) { name = newval; }


namespace osh
{

#ifndef OSH_NOMETADATA

/**
 * A bunch of strings stored in flash so we don't use up all the RAM!
 */
extern const char XMLNS_PREFIX[] PROGMEM;
extern const char SML_PREFIX[] PROGMEM;
extern const char GML_PREFIX[] PROGMEM;
extern const char SWE_PREFIX[] PROGMEM;
extern const char XLINK_PREFIX[] PROGMEM;

#ifndef OSH_NOXMLNS
extern const char OGC_NS_PREFIX[] PROGMEM;
extern const char SML_NS[] PROGMEM;
extern const char SWE_NS[] PROGMEM;
extern const char GML_NS[] PROGMEM;
extern const char XLINK_NS[] PROGMEM;
#endif

extern const char TIME[] PROGMEM;
extern const char QUANTITY[] PROGMEM;
extern const char COUNT[] PROGMEM;
extern const char CATEGORY[] PROGMEM;
extern const char BOOLEAN[] PROGMEM;
extern const char TEXT[] PROGMEM;

extern const char ATT_ID[] PROGMEM;
extern const char ATT_NAME[] PROGMEM;
extern const char ATT_DEFINITION[] PROGMEM;
extern const char ATT_REFRAME[] PROGMEM;
extern const char ATT_AXISID[] PROGMEM;

extern const char ELT_COMPONENT[] PROGMEM;
extern const char ELT_SYSTEM[] PROGMEM;
extern const char ELT_IDENTIFIER[] PROGMEM;
extern const char ELT_DESCRIPTION[] PROGMEM;
extern const char ELT_OUTPUTS[] PROGMEM;
extern const char ELT_OUTPUTLIST[] PROGMEM;
extern const char ELT_OUTPUT[] PROGMEM;
extern const char ELT_DATARECORD[] PROGMEM;
extern const char ELT_VECTOR[] PROGMEM;
extern const char ELT_FIELD[] PROGMEM;
extern const char ELT_COORDINATE[] PROGMEM;
extern const char ELT_LABEL[] PROGMEM;
extern const char ELT_UOM[] PROGMEM;
extern const char ATT_HREF[] PROGMEM;
extern const char ATT_CODE[] PROGMEM;

extern const char HTTP_PREFIX[] PROGMEM;
extern const char OGC_DEF_PREFIX[] PROGMEM;
extern const char DEF_SAMPLING_TIME[] PROGMEM;
extern const char DEF_ISO8601[] PROGMEM;
extern const char DEF_UTC[] PROGMEM;
extern const char DEF_EPSG4326[] PROGMEM;
extern const char DEF_EPSG4979[] PROGMEM;


static void buildUrl(const char* prefix, const char* path, char* buf)
{
    strcpy_P(buf, prefix);
    strcat_P(buf, path);
}

static void buildDefUrl(const char* def, char* buf)
{
    /*if (strncmp_P(def, HTTP_PREFIX, 7) == 0)
    {
        // if buf was stored in RAM
        strcpy(buf, def);
    }
    else*/
    {
        // if buf was stored in FLASH (PROGMEM)
        strcpy_P(buf, def);
        if (strncmp_P(buf, HTTP_PREFIX, 7) != 0)
        {
            strcpy_P(buf, OGC_DEF_PREFIX);
            strcat_P(buf, def);
        }
    }
}


/**
 * Measurement class represents a sensor measurement output
 */
class Measurement
{
    OSH_GETSETVAR(const char*, Name);
    OSH_GETSETVAR(const char*, Type);
    OSH_GETSETVAR(const char*, Definition);
    OSH_GETSETVAR(const char*, Label);
    OSH_GETSETVAR(const char*, Uom);
    OSH_GETSETVAR(const char*, RefFrame);
    OSH_GETSETVAR(const char*, AxisID);

public:
    Measurement();
    virtual ~Measurement() {};

#ifndef OSH_NOXML
    virtual void writeXML(XMLWriter& w);
#endif

#ifndef OSH_NOJSON
    // TODO JSON serialization
#endif
};


/**
 * Vector measurement for representing location and other vector quantities
 */
class VectorMeas: public Measurement
{
private:
    int numCoords = 0;
    Measurement* coords[4];

public:
    ~VectorMeas();
    void addCoordinate(const char* axisID, const char* uom, const char* label = 0, const char* type = 0);

#ifndef OSH_NOXML
    virtual void writeXML(XMLWriter& w);
#endif

#ifndef OSH_NOJSON
    // TODO JSON serialization
#endif
};


/**
 * Base class for sensors and systems
 */
class Device
{
    OSH_GETSETVAR(const char*, UniqueID);
    OSH_GETSETVAR(const char*, Name);
    OSH_GETSETVAR(const char*, Description);
    OSH_GETSETVAR(const double*, Location); // array of size 3 in EPSG 4979

public:
    virtual ~Device() {};
    virtual bool isSystem() = 0;

#ifndef OSH_NOXML
public:
    void toXML(Print& out);
    virtual void writeXML(XMLWriter& w, bool nested) = 0;
protected:
    void writeNamespaces(XMLWriter& w);
#endif

#ifndef OSH_NOJSON
public:
    void toJSON(Print& out);
#endif

};


/**
 * Sensor class allowing to declare outputs and send data to a stream
 */
class Sensor: public Device
{
    friend class System;
    OSH_GETSETVAR(String, StreamID);

protected:
    int numOutputs = 0;
    Measurement* outputs[10];

public:
    ~Sensor();
    void addTimeStampUTC();
    void addTimeStampOBC(const char* uom);
    void addTimeStamp(const char* uom, const char* refFrame);
    void addMeasurement(Measurement* meas);
    void addMeasurement(const char* name, const char* def, const char* uom, const char* label = 0, const char* type = 0);
    void addLocationLLA(const char* def, const char* label = 0);
    bool isSystem() { return false; };

#ifndef OSH_NOXML
public:
    void writeXML(XMLWriter& w, bool nested);
    void writeOutput(XMLWriter& w);
#endif

#ifndef OSH_NOJSON
    // TODO JSON serialization
#endif
};


/**
 * System class is an aggregate of multiple sensors, with multiple
 * (usually asynchronous) outputs
 */
class System: public Device
{
private:
    int numSensors = 0;
    Sensor* sensors[10];

public:
    void addSensor(Sensor* sensor);
    int getNumSensors() { return numSensors; };
    Sensor** getSensors() { return sensors; };
    bool isSystem() { return true; };

#ifndef OSH_NOXML
public:
    void writeXML(XMLWriter& w, bool nested);
#endif

#ifndef OSH_NOJSON
    // TODO JSON serialization
#endif
};


#endif // OSH_NOMETADATA


/**
 * Base class for client connecting to an OSH node
 */
class OSHClient
{
public:
    virtual ~OSHClient() {};
    virtual void registerDevice(Device* device) = 0;
    virtual void pushInt(int val) = 0;
    virtual void pushUInt(unsigned int val) = 0;
    virtual void pushFloat(float val) = 0;
    virtual void pushDouble(double val) = 0;
    virtual void pushString(char* val) = 0;
    virtual void startMeasurement(Sensor* s) = 0;
    virtual void sendMeasurement() = 0;
};


/**
 * SOS client able to register the sensor and then push data to the server
 */
class SOSClient: public OSHClient
{
private:
    Client* client;
    const char* server;
    const char* path;
    int port;
    Device* device;
    Sensor* sendingSensor;
    String measBuf;

public:
    SOSClient(Client& client, const char* hostNameOrIp, int httpPort, const char* sosPath);
    void registerDevice(Device* device);
    void pushInt(int val);
    void pushUInt(unsigned int val);
    void pushFloat(float val);
    void pushDouble(double val);
    void pushString(char* val);
    void startMeasurement(Sensor* s);
    void sendMeasurement();

private:
    String sendInsertSensor(Device* device);
    void sendResultTemplate(const char* offeringID, Sensor* s);
    String readResponseTag(const char* tagName);

};


#ifdef OSH_MQTT

/**
 * MQTT client publishing sensor description and data to predefined topics
 */
class MQTTClient: public OSHClient
{


}

#endif // OSH_MQTT

} // namespace osh

#endif // OSH_H
