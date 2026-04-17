
#include <Arduino.h>
// Define pins
const int  trigPin = 5;
const int  echoPin = 18;
const int pumpPin = 4;
const int waterUsePin = 6; // Make sure this connects to USE_OUT in diagram.json

// Define variables
long duration;
float distance;
int isUsingWater; // Variable to store the read state

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(waterUsePin, INPUT);
}

void loop() {
  // clear pins
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sends 10 microsecond HIGH pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo pin
  duration = pulseIn(echoPin, HIGH);

  // Calculate distance
  distance = duration * 0.034 / 2;

  // Read the actual HIGH/LOW state from the water tank simulator
  isUsingWater = digitalRead(waterUsePin);

  // Print distance
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm  |  Water in use: ");

  // Print the status clearly
  if (isUsingWater == HIGH) {
    Serial.println("YES");
  } else {
    Serial.println("NO");
  }

  delay(100);
}

