#include <Servo.h>

// --- Pin Definitions ---
// Motor Driver (L293D / L298N)
#define ENA 5   // Left Motor Speed
#define IN1 6   // Left Motor Forward
#define IN2 7   // Left Motor Reverse

#define ENB 10  // Right Motor Speed
#define IN3 8   // Right Motor Forward
#define IN4 9   // Right Motor Reverse

// Ultrasonic Sensor (HC-SR04)
#define TRIG_PIN 11
#define ECHO_PIN 12

// Servo Motor
#define SERVO_PIN 3

// --- Constants & Thresholds ---
#define STOP_DISTANCE 20 // Avoidance threshold (cm)
#define MOTOR_SPEED 150  // Base cruising speed (0-255)
#define SWEEP_DELAY 250  // ms to allow servo to reach position

Servo scannerServo;

void setup() {
  Serial.begin(115200);
  
  // Configure motor control pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Configure HC-SR04 pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initialize Servo
  scannerServo.attach(SERVO_PIN);
  scannerServo.write(90); // Center position (Look forward)
  delay(1000);            // Allow servo to stabilize
}

void loop() {
  // 1. Continuously map surroundings (Forward)
  int distanceForward = getDistance();
  
  // 2. Threshold-based logic (trigger avoidance at 20 cm)
  if (distanceForward > 0 && distanceForward <= STOP_DISTANCE) {
    stopMotors();
    delay(50); // Braking time
    
    // Reverse slightly to create clearance
    moveBackward();
    delay(200);
    stopMotors();
    
    // 3. Dynamic Maneuvers: Scan both directions
    int distanceLeft = scanDirection(150);  // Look Left
    int distanceRight = scanDirection(30);  // Look Right
    
    // 4. Compare clearances & select optimal path
    if (distanceLeft >= distanceRight) {
      turnLeft();
      delay(300); // 90 degree turn (tune based on chassis)
    } else {
      turnRight();
      delay(300); // 90 degree turn (tune based on chassis)
    }
    
    stopMotors();
    delay(50);
  } else {
    // Path clear, continue forward
    moveForward();
  }
  
  // 5. Tuned sensor sampling for sub-100 ms reaction time
  delay(30); // ~30ms loop delay ensures rapid reaction without acoustic echo interference
}

/* 
 * getDistance(): 
 * Sends a 10us ultrasonic pulse and calculates the distance in cm.
 * Incorporates a 30,000 us timeout to prevent hanging when no object is detected.
 */
int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // 30,000 microsecond timeout ensures function returns quickly (sub-100ms reaction)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
  
  if (duration == 0) return 999; // No echo received, path is clear
  
  // Calculate distance in cm (Speed of sound is 343 m/s)
  int distance = duration * 0.034 / 2;
  return distance;
}

/*
 * scanDirection():
 * Sweeps servo to a specific angle, reads distance, and returns servo to center.
 */
int scanDirection(int angle) {
  scannerServo.write(angle);
  delay(SWEEP_DELAY); 
  
  int dist = getDistance();
  
  scannerServo.write(90); // Return to center immediately
  delay(SWEEP_DELAY);
  
  return dist;
}

// --- Motor Control Functions ---

void moveForward() {
  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void turnLeft() {
  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH); // Left motors reverse
  digitalWrite(IN3, HIGH); // Right motors forward
  digitalWrite(IN4, LOW);
}

void turnRight() {
  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
  digitalWrite(IN1, HIGH); // Left motors forward
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH); // Right motors reverse
}
