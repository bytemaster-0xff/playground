#include "../include/ActionCompletionHandlers.h"

int delayCount = 18000;
int VALVE_OUTPUT_POWER = D3; /* Pool or Spa Output */
int VALVE_OUTPUT_DIRECTION = D2; /* Pool or Spa Output */

int VALVE_SPA_POWER = D5;
int VALVE_SPA_DIRECTION = D4;

int VALVE_SOURCE_POWER = D7;
int VALVE_SOURCE_DIRECTION = D6;

void ActionCompletionHandlers::init()
{
  pinMode(VALVE_SOURCE_POWER, OUTPUT);
  digitalWrite(VALVE_SOURCE_POWER, HIGH);

  pinMode(VALVE_OUTPUT_POWER, OUTPUT);
  digitalWrite(VALVE_OUTPUT_POWER, HIGH);

  pinMode(VALVE_SPA_POWER, OUTPUT);
  digitalWrite(VALVE_SPA_POWER, HIGH);

  pinMode(VALVE_SOURCE_DIRECTION, OUTPUT);
  digitalWrite(VALVE_SOURCE_DIRECTION, HIGH);

  pinMode(VALVE_OUTPUT_DIRECTION, OUTPUT);
  digitalWrite(VALVE_OUTPUT_DIRECTION, HIGH);

  pinMode(VALVE_SPA_DIRECTION, OUTPUT);
  digitalWrite(VALVE_SPA_DIRECTION, HIGH);
}

void ActionCompletionHandlers::resetOutputPostion()
{
  _currentAction = "Resetting output actuator to pool position";
  Particle.publish("Begin Resetting Actuator", "Output");

  digitalWrite(VALVE_OUTPUT_POWER, LOW);
  digitalWrite(VALVE_OUTPUT_DIRECTION, HIGH);

  delay(delayCount * 2);

  digitalWrite(VALVE_OUTPUT_POWER, HIGH);
  digitalWrite(VALVE_OUTPUT_DIRECTION, HIGH);

  _currentAction = "Idle";

  _outputState = "pool";

   Particle.publish("End Resetting Actuator", "Output");
}

void ActionCompletionHandlers::resetSourcePosition()
{
  _currentAction = "Resetting source actuator";

  Particle.publish("Begin Resetting Actuator", "Source");

  digitalWrite(VALVE_SOURCE_POWER, LOW);
  digitalWrite(VALVE_SOURCE_DIRECTION, HIGH);

  delay(delayCount * 2);

  digitalWrite(VALVE_SOURCE_POWER, HIGH);
  digitalWrite(VALVE_SOURCE_DIRECTION, HIGH);

  _sourceState = "pool";

  _currentAction = "Idle";

  Particle.publish("End Resetting Actuator", "Source");
}

void ActionCompletionHandlers::resetSpaModePosition()
{
  _currentAction = "Resetting spa mode actuator to normal position";
  Particle.publish("Begin Resetting Actuator", "Spa Mode");

  digitalWrite(VALVE_SPA_POWER, LOW);
  digitalWrite(VALVE_SPA_DIRECTION, HIGH);


  delay(delayCount * 2);

  digitalWrite(VALVE_SPA_POWER, HIGH);
  digitalWrite(VALVE_SPA_DIRECTION, HIGH);

  _spaModeState = "normal";
  _currentAction = "Idle";

  Particle.publish("End Resetting Actuator", "Spa Mode");
}

String ActionCompletionHandlers::getCurrentAction()
{
  return _currentAction;
}

String ActionCompletionHandlers::getOutputState()
{
  return _outputState;
}

String ActionCompletionHandlers::getSourceState()
{
  return _sourceState;
}

String ActionCompletionHandlers::getSpaModeState()
{
  return _spaModeState;
}

void ActionCompletionHandlers::setPoolAsSource()
{
  Particle.publish("Begin Water Source Mode", "Pool");

  if(_sourceState == "unknown") {
    resetSourcePosition();
  }

  _currentAction = "Setting State to Pool";

  digitalWrite(VALVE_SOURCE_POWER, LOW);
  digitalWrite(VALVE_SOURCE_DIRECTION, HIGH);

  delay(delayCount * 2);

  digitalWrite(VALVE_SOURCE_POWER, HIGH);
  digitalWrite(VALVE_SOURCE_DIRECTION, HIGH);

  _sourceState = "pool";
  _currentAction = "Idle";

  Particle.publish("End Water Source Mode", "Pool");
}

void ActionCompletionHandlers::setSpaAsSource()
{
  Particle.publish("Begin Water Source Mode", "Spa");

  if(_sourceState == "unknown") {
    resetSourcePosition();
  }

  _currentAction = "Setting water source to spa.";

  digitalWrite(VALVE_SOURCE_POWER, LOW);
  digitalWrite(VALVE_SOURCE_DIRECTION, LOW);

  delay(delayCount * 2);

  digitalWrite(VALVE_SOURCE_POWER, HIGH);
  digitalWrite(VALVE_SOURCE_DIRECTION, HIGH);

  _sourceState = "spa";
  _currentAction = "Idle";

  Particle.publish("End Source Mode", "Spa");
}

void ActionCompletionHandlers::setBothAsSource()
{
  Particle.publish("Begin Water Source Mode", "Pool and Spa");

  if(_sourceState == "unknown") {
    resetSourcePosition();
  }

  _currentAction = "Setting water source to pool and spa";

  digitalWrite(VALVE_SOURCE_POWER, LOW);
  if(_sourceState == "pool")
      digitalWrite(VALVE_SOURCE_DIRECTION, LOW);
  else
      digitalWrite(VALVE_SOURCE_DIRECTION, HIGH);

  delay(delayCount);

  digitalWrite(VALVE_SOURCE_POWER, HIGH);
  digitalWrite(VALVE_SOURCE_DIRECTION, HIGH);

  _sourceState = "both";
  _currentAction = "Idle";

  Particle.publish("End Water Source Mode", "Pool and Spa");
}

void ActionCompletionHandlers::setPoolAsOutput()
{
  Particle.publish("Begin Set Output Mode", "Pool");

  if(_outputState == "unknown") {
    resetOutputPostion();
  }

  _currentAction = "Setting output mode to pool";

  digitalWrite(VALVE_OUTPUT_POWER, LOW);
  digitalWrite(VALVE_OUTPUT_DIRECTION, HIGH);

  delay(delayCount * 2);

  digitalWrite(VALVE_OUTPUT_POWER, HIGH);
  digitalWrite(VALVE_OUTPUT_DIRECTION, HIGH);

  _outputState = "pool";
  _currentAction = "Idle";

  Particle.publish("End Set Output Mode","Pool");
}

void ActionCompletionHandlers::setSpaAsOutput()
{
  Particle.publish("Begin Set Output Mode", "Spa");

  if(_outputState == "unknown") {
    resetOutputPostion();
  }

  _currentAction = "Setting output mode to spa";

  digitalWrite(VALVE_OUTPUT_POWER, LOW);
  digitalWrite(VALVE_OUTPUT_DIRECTION, LOW);

  delay(delayCount * 2);

  digitalWrite(VALVE_OUTPUT_POWER, HIGH);
  digitalWrite(VALVE_OUTPUT_DIRECTION, HIGH);

  _outputState = "spa";
  _currentAction = "Idle";

  Particle.publish("End Set Output Mode", "Spa");
}

void ActionCompletionHandlers::setBothAsOutput()
{
  Particle.publish("Beging Set Output Mode", "Pool and Spa");

  if(_outputState == "unknown") {
    resetOutputPostion();
  }

  _currentAction = "Setting output mode to pool and spa";

  digitalWrite(VALVE_OUTPUT_POWER, LOW);

  if(_outputState == "pool")
      digitalWrite(VALVE_OUTPUT_DIRECTION, LOW);
  else
      digitalWrite(VALVE_OUTPUT_DIRECTION, HIGH);

  delay(delayCount);

  digitalWrite(VALVE_OUTPUT_POWER, HIGH);
  digitalWrite(VALVE_OUTPUT_DIRECTION, HIGH);

  _outputState = "both";
  _currentAction = "Idle";

  Particle.publish("End Set Output Mode", "Pool and Spa");
}

void ActionCompletionHandlers::setSpaModeNormal()
{
  Particle.publish("Begin Set Spa Mode", "Normal");

  if(_spaModeState == "unknown") {
    resetSpaModePosition();
  }

  _currentAction = "Setting spa mode to normal";

  digitalWrite(VALVE_SPA_POWER, LOW);
  digitalWrite(VALVE_SPA_DIRECTION, HIGH);

  delay(delayCount * 2);

  digitalWrite(VALVE_SPA_POWER, HIGH);
  digitalWrite(VALVE_SPA_DIRECTION, HIGH);

  _spaModeState = "normal";
  _currentAction = "Idle";

  Particle.publish("End Set Spa Mode", "Normal");
}

void ActionCompletionHandlers::setSpaModeJets()
{
  Particle.publish("Begin Set Spa Mode", "Jets");

  if(_spaModeState == "unknown") {
    resetSpaModePosition();
  }

  _currentAction = "Setting spa mode to jets";

  digitalWrite(VALVE_SPA_POWER, LOW);
  digitalWrite(VALVE_SPA_DIRECTION, LOW);

  delay(delayCount * 2);

  digitalWrite(VALVE_SPA_POWER, HIGH);
  digitalWrite(VALVE_SPA_DIRECTION, HIGH);

  _spaModeState = "jets";
  _currentAction = "Idle";

  Particle.publish("End Set Spa Mode", "Jets");
}
