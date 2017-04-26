// 
// 
// 

#include "A4988.h"


A4988::A4988(int dirPin, int stepPin, int enablePin) {
	pinMode(dirPin, OUTPUT);
	pinMode(stepPin, OUTPUT);
	pinMode(enablePin, OUTPUT);

	m_stepPin = stepPin;
	m_dirPin = dirPin;
	m_enablePin = enablePin;
}

void A4988::Enable() {
	digitalWrite(m_enablePin, LOW);
}

void A4988::Disable() {
	digitalWrite(m_enablePin, HIGH);
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
	}

	m_currentLocation = cm;
}

float A4988::GetCurrentLocation() {
	return m_currentLocation;
}