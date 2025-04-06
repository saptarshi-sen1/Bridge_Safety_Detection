#include <Arduino.h>
#include <ESP32Servo.h>
#include <DHT.h>

// === PIN DEFINITIONS ===
#define TRIG_PIN 5
#define ECHO_PIN 18

#define X_PIN 34
#define Y_PIN 35
#define Z_PIN 32

#define DHTPIN 4
#define DHTTYPE DHT11

#define SERVO_PIN 16
#define LED_PIN 2
#define BUZZER_PIN 15

// === OBJECTS ===
DHT dht(DHTPIN, DHTTYPE);
Servo myServo;

// === CONSTANTS ===
const float SOUND_SPEED = 0.0343; // cm/us
float temperature = 0;
float distance = 0;
float xTilt = 0, yTilt = 0, zTilt = 0;

// === FUNCTIONS ===
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) return -1;
  return (duration * SOUND_SPEED) / 2.0;
}

void readTilt() {
  int xRaw = analogRead(X_PIN);
  int yRaw = analogRead(Y_PIN);
  int zRaw = analogRead(Z_PIN);

  float xVolt = xRaw * (3.3 / 4095.0);
  float yVolt = yRaw * (3.3 / 4095.0);
  float zVolt = zRaw * (3.3 / 4095.0);

  xTilt = (xVolt - 1.65) / 0.3;
  yTilt = (yVolt - 1.65) / 0.3;
  zTilt = (zVolt - 1.65) / 0.3;
}

void readTemperature() {
  temperature = dht.readTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    temperature = -999;
  }
}

// === SETUP ===
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();
  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2400);
  myServo.write(0); // Default to 0°

  Serial.println("System ready.");
}

// === LOOP ===
void loop() {
  readTemperature();
  readTilt();

  Serial.println("=== SENSOR READINGS ===");
  Serial.print("Temp: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Tilt X: "); Serial.print(xTilt); Serial.print(" | Y: "); Serial.print(yTilt); Serial.println();

  bool highTemp = temperature >= 50;
  bool abnormalTilt = (xTilt < -0.8 || xTilt > -0.35 || yTilt < -0.9 || yTilt > -0.35);

  if (highTemp || abnormalTilt) {
    Serial.println("Abnormality detected: High Temp or Tilt.");

    // Turn ON LED & Buzzer
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    distance = getDistance();
    Serial.print("Distance: "); Serial.print(distance); Serial.println(" cm");

    if (distance > 10 || distance < 0) {
      Serial.println("No object in front. Moving servo to 90°.");
      myServo.write(90);
    } else {
      Serial.println("Object detected in front. Keeping servo at 0°.");
      myServo.write(0);
    }

  } else {
    Serial.println("All conditions normal.");

    // Turn OFF LED & Buzzer
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    myServo.write(0);
  }

  Serial.println("=========================\n");
  delay(1000);
}