// 
// 
// 

#include "A4988.h"


A4988::A4988(int dirPin, int stepPin, int enablePin, String axisName) {
	pinMode(dirPin, OUTPUT);
	pinMode(stepPin, OUTPUT);
	pinMode(enablePin, OUTPUT);

	m_stepPin = stepPin;
	m_dirPin = dirPin;
	m_enablePin = enablePin;
	m_axisName = axisName;

	Disable();
}

void A4988::SetMinLimitPin(uint8_t minlimitPin)
{
	m_minLimitPin = minlimitPin;
}

void A4988::SetMaxLimitPin(uint8_t maxLimitPin)
{
	m_maxLimitPin = maxLimitPin;
}


void A4988::Enable() {
	digitalWrite(m_enablePin, LOW);
}

void A4988::Disable() {
	digitalWrite(m_enablePin, HIGH);
}

bool A4988::GetIsBusy()
{
	return m_isBusy;
}

void A4988::Move(float cm, float feed) {
	Enable();

	float delta = cm - m_currentLocation;


	digitalWrite(m_dirPin, delta< 0 ? LOW : HIGH);

	long steps = abs(delta * 300);

	for (int idx = 0; idx < steps; ++idx) {
		digitalWrite(m_stepPin, HIGH);
		delayMicroseconds(150);
		digitalWrite(m_stepPin, LOW);
		delayMicroseconds(150);

		if (delta < 0 && m_minLimitPin != -1 && digitalRead(m_minLimitPin) == LOW)
		{
			idx = steps;
			Serial.println(m_axisName);
			Serial.println(" ");
			Serial.println("MIN-LIMIT-HIT");
			m_minLimitPin = true;
		}

		if (delta > 0 && m_maxLimitPin != -1 && digitalRead(m_maxLimitPin) == LOW)
		{
			idx = steps;
			Serial.println(m_axisName);
			Serial.println(" ");
			Serial.println("MAX-LIMIT-HIT");
			m_maxLimitPin = true;
		}
	}

	m_currentLocation = cm;
}

void A4988::Home()
{
	bool endStopHit = false;
	digitalWrite(m_dirPin, LOW);

	while (endStopHit == false)
	{
		digitalWrite(m_stepPin, HIGH);
		delayMicroseconds(250);

		digitalWrite(m_stepPin, LOW);
		delayMicroseconds(250);

		endStopHit = digitalRead(m_minLimitPin) == LOW;
	}

	m_currentLocation = 0;
}


void A4988::Update()
{

}

float A4988::GetCurrentLocation() {
	return m_currentLocation;
}