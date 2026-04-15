
#include <Arduino.h>
// Define pins
const int  trigPin = 5;
const int  echoPin = 18;

// Define variables
long duration;
float distance;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  // clear pins
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sends 10 ms HIHG pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo pin
  duration = pulseIn(echoPin, HIGH);

  distance = duration * 0.034 / 2;

  Serial.print("Dinstance: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(100);
}

