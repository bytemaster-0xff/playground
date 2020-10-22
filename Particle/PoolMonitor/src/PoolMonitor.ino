// This #include statement was automatically added by the Particle IDE.
#include <HttpClient.h>
#include "../include/ActionCompletionHandlers.h"

int COMPRESSOR1 = RX;
int COMPRESSOR2 = TX;

int FAN = DAC;

int WATER_TEMP_IN = A1;
int WATER_TEMP_OUT = A7;

int FREEZE_SENSOR = A2;

int FLOW_SENSOR = A4;
int LOW_PRESSURE = A5;
int HIGH_PRESSURE = D0;

int temperatureIn;
int temperatureOut;
int _spaSetpoint = 102;
int _poolSetpoint = 88;

int temperatureCountIn;
int temperatureCountOut;

int freezeSensorCount;

int rssi = 0;

HttpClient isyClient;
HttpClient nuviotClient;

String _mode = "off";

String _flow = "unknown";
String _lowPressure = "unknown";
String _highPressure = "unknown";
String _compressorState = "off";
String _fanState = "off";

/*
Old Sensor
 temperature Calibration ~3900 104
    3832 102
    3734    99
   3560  93
   ~3490 91
   temperature Calibration ~3369 87
   temperature Calibration ~3287 81
   temperature Calibration ~2886 72

   temperature out
   count 2982 79
   count 3328 81
   count 3476 84
*/

/*
New Sensor:
    ~2555 65
    ~2751 73
    ~2906 78
    ~3099 84
    ~3286 90
    ~3398 94.5
    ~3525 99

    ~3089 82
    ~3123 83
    ~3162 84
    ~3250 85
*/

ActionCompletionHandlers actionHandlers;

void setup() {
   Particle.variable("RSSI", &rssi, INT);

    pinMode(COMPRESSOR2, OUTPUT);
    digitalWrite(COMPRESSOR2, HIGH);

    pinMode(COMPRESSOR1, OUTPUT);
    digitalWrite(COMPRESSOR1, HIGH);

    pinMode(FAN, OUTPUT);
    digitalWrite(FAN, HIGH);

    pinMode(FLOW_SENSOR, INPUT_PULLDOWN);
    pinMode(LOW_PRESSURE, INPUT_PULLDOWN);
    pinMode(HIGH_PRESSURE, INPUT_PULLDOWN);

    pinMode(WATER_TEMP_IN, INPUT);
    pinMode(WATER_TEMP_OUT, INPUT);
    pinMode(FREEZE_SENSOR, INPUT);

    pinMode(COMPRESSOR2, OUTPUT);
    digitalWrite(COMPRESSOR2, HIGH);

    Particle.variable("tempCountIn", temperatureCountIn);
    Particle.variable("tempIn", temperatureIn);
    Particle.variable("tempCountOut", temperatureCountOut);
    Particle.variable("tempOut", temperatureOut);
    Particle.variable("lp", _lowPressure);
    Particle.variable("flow", _flow);
    Particle.variable("hp", _highPressure);
    Particle.variable("mode", _mode);
    Particle.variable("spasetpoint", _spaSetpoint);
    Particle.variable("poolsetpoint", _poolSetpoint);
    Particle.variable("compressor", _fanState);
    Particle.variable("fan", _compressorState);

    Particle.function("poolon", poolOn);
    Particle.function("pooloff", poolOff);

    Particle.function("compon", compressorOn);
    Particle.function("compoff", compressorOff);
    Particle.function("fanon", fanOn);
    Particle.function("fanoff", fanOff);

    Particle.function("source", setWaterSource);
    Particle.function("output", setWaterOutput);
    Particle.function("spamode", setSpaMode);
    Particle.function("mode", setMode);

    Particle.function("spasetpoint", setSpaSetpoint);
    Particle.function("poolsetpoint", setPoolSetpoint);

    actionHandlers.init();

    Particle.publish("Pool Controller Startup", "Completed");
}

#define  ValveLeft 1
#define  ValveMiddle 2
#define  ValveRight 3

int setSpaSetpoint(String spaSetpoint) {
    _spaSetpoint = spaSetpoint.toInt();
    return 0;
}

int setPoolSetpoint(String poolSetpoint) {
    _poolSetpoint = poolSetpoint.toInt();
    return 0;
}

/*
Valve #1 - Divert to pool or overflow R Closes output to hot tub
Valve #2 - Divert Spa High-Pressure or Spa Low Pressure R closes output to spa jets
Valve #3 - Pull from Hot Tub or Pool R from Pool
*/

int setMode(String mode) {
    _mode = mode;
    return 200;
}

Timer setPoolAsSourceTimer(100, &ActionCompletionHandlers::setPoolAsSource, actionHandlers, true);
Timer setBothAsSourceTimer(100, &ActionCompletionHandlers::setBothAsSource, actionHandlers, true);
Timer setSpaAsSourceTimer(100, &ActionCompletionHandlers::setSpaAsSource, actionHandlers, true);

Timer setPoolAsOutputTimer(100, &ActionCompletionHandlers::setPoolAsOutput, actionHandlers, true);
Timer setSpaAsOutputTimer(100, &ActionCompletionHandlers::setSpaAsOutput, actionHandlers, true);
Timer setBothAsOutputTimer(100, &ActionCompletionHandlers::setBothAsOutput, actionHandlers, true);

Timer setSpaModeJetsTimer(100, &ActionCompletionHandlers::setSpaModeJets, actionHandlers, true);
Timer setSpaModeNormalTimer(100, &ActionCompletionHandlers::setSpaModeNormal, actionHandlers, true);

int setWaterSource(String source){
  source = source.toLowerCase();

  if(source == actionHandlers.getSourceState()) {
    return 304;
  }

  if(source == "pool") {
    setPoolAsSourceTimer.start();
    return 200;
  }

  if(source == "spa") {
    setSpaAsSourceTimer.start();
    return 200;
  }

  if(source == "both") {
    setBothAsSourceTimer.start();
    return 200;
  }

  return 401;
}

int setWaterOutput(String output) {
    output = output.toLowerCase();
    if(output == actionHandlers.getOutputState()){
      return 304;
    }

    if(output == "pool"){
      setPoolAsOutputTimer.start();
      return 200;
    }

    if(output == "spa") {
      setSpaAsOutputTimer.start();
      return 200;
    }

    if(output == "both") {
      setBothAsOutputTimer.start();
      return 200;
    }

    return 401;
}

int setSpaMode(String mode) {
  mode = mode.toLowerCase();
  if(mode == actionHandlers.getSpaModeState()){
    return 304;
  }

  if(mode.toLowerCase() == "normal") {
    setSpaModeNormalTimer.start();
    return 200;
  }

  if(mode.toLowerCase() == "jets") {
    setSpaModeJetsTimer.start();
    return 200;
  }

  return 401;
}

int compressorOn(String in) {
    digitalWrite(COMPRESSOR1, LOW);
    digitalWrite(COMPRESSOR2, LOW);
    _compressorState = "on";
    return 0;
}

int fanOn(String in) {
    digitalWrite(FAN, LOW);
    _fanState = "on";
    return 0;
}

int compressorOff(String in) {
    digitalWrite(COMPRESSOR1, HIGH);
    digitalWrite(COMPRESSOR2, HIGH);
    _compressorState = "off";
    return 0;
}

int fanOff(String in) {
    digitalWrite(FAN, HIGH);
    _fanState = "off";
    return 0;
}

http_header_t headers[] = {
 { "Accept" , "text/xml"},
 { "Authorization" , "Basic a2V2aW53Ok15MTdEYXkh"},
 { NULL, NULL } // NOTE: Always terminate headers will NULL
};

int poolOn(String in) {
    http_request_t  request;
    http_response_t response;
    request.hostname = "slsys.homeip.net";
    request.port = 80;
    request.path = "/rest/nodes/2E%208C%20AA%201/cmd/DON";
    isyClient.get(request, response, headers);
    return response.status;
}

int poolOff(String off) {
    http_request_t  request;
    http_response_t response;
    request.hostname = "slsys.homeip.net";
    request.port = 80;
    request.path = "/rest/nodes/2E%208C%20AA%201/cmd/DOF";
    isyClient.get(request, response, headers);

    return response.status;
}

http_header_t nuvIoTHeaders[] = {
    { "Content-Type" , "application/json"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

int sendPoolStatus(){
    http_request_t  request;
    http_response_t response;

    request.hostname = "lagovista.softwarelogistics.iothost.net";
    request.port = 9000;
    request.path = "/poolstatus/" + Spark.deviceID();

    rssi = WiFi.RSSI();

    request.body = "{\"mode\":\"" + _mode + "\",\"temperatureIn\":" + temperatureIn + ",\"temperatureOut\":" + temperatureOut +
            ",\"flow\":\"" + _flow + "\",\"lowPressure\":\"" + _lowPressure + "\",\"highPressure\":\"" + _highPressure + "\",\"fanState\":\"" + _fanState +
       "\",\"compressorState\":\"" + _compressorState + "\",\"currentSpaMode\":\"" + actionHandlers.getSpaModeState() +
       "\",\"currentSource\":\"" + actionHandlers.getSourceState() + "\",\"currentOutput\":\"" + actionHandlers.getOutputState() +
       "\",\"rssi\":\"" + rssi + "\"}";

    http_header_t  nuvIoTHeaders[] = {
        { "Content-Type" , "application/json"},
        { NULL, NULL } // NOTE: Always terminate headers will NULL
    };

    nuviotClient.post(request, response, nuvIoTHeaders);
    if(response.status != 200) {
      String status = "status: " + String(response.status);
      Particle.publish("Http Response Status", status);
    }
}

void loop() {
    long temperatureSumIn = 0;
    long temperatureSumOut = 0;
    for(int idx = 0; idx < 10; ++idx)
    {
      temperatureSumIn += analogRead(WATER_TEMP_IN);
      temperatureSumOut += analogRead(WATER_TEMP_OUT);
      delay(100);
    }

    temperatureCountOut = temperatureSumOut / 10;
    temperatureOut = temperatureCountOut * 0.03324519 - 19.13;

    temperatureCountIn = temperatureSumIn / 10;
    temperatureIn = temperatureCountIn * 0.03324519 - 19.13;

    if(_flow == "warning"){
        fanOff("");
        compressorOff("");
    }
    else if(_mode == "spa"){
        if(temperatureIn < _spaSetpoint){
            fanOn("");
            compressorOn("");
        }

        if(temperatureIn  > _spaSetpoint ) {
            fanOff("");
            compressorOff("");
        }
    }
    else if(_mode == "pool") {
      if(temperatureIn < _poolSetpoint){
          fanOn("");
          compressorOn("");
      }

      if(temperatureIn  > _poolSetpoint ) {
          fanOff("");
          compressorOff("");
      }
    }
    else if(_mode == "off") {
        fanOff("");
        compressorOff("");
    }

    _flow = digitalRead(FLOW_SENSOR) == 1 ? "ok" : "warning";
    _lowPressure = digitalRead(LOW_PRESSURE) == 1 ? "ok" : "warning";
    _highPressure = digitalRead(HIGH_PRESSURE) == 1 ? "ok" : "warning";

    sendPoolStatus();
}
