#include <Servo.h>


// ================= MOTOR PINS =================
const int leftMotorForwardPin  = 4;
const int leftMotorBackwardPin = 2;
const int rightMotorForwardPin = 8;
const int rightMotorBackwardPin = 7;
const int leftMotorSpeedPin  = 5;   // PWM
const int rightMotorSpeedPin = 6;   // PWM


// ================= MICROPHONE PINS =================
const int mic1pin = A3;
const int mic2pin = A4;


// ================= ULTRASONIC + SERVO =================
Servo myservo;
const int Echo_Pin = A0;
const int Trig_Pin = A1;


// ================= RGB LED (COMMON GND) =================
const int redPin   = 9;
const int greenPin = 10;
const int bluePin  = 11;


// ================= SOUND VARIABLES =================
const int sample_window = 3000;
int mic1, mic2;
int mic1_max, mic1_min, mic2_max, mic2_min;
int amp1, amp2, difference;


const int lowSoundThreshold = 25;
const int turnDifferenceThreshold = 20;
const int maxThreshold = 1000;


// ================= SPEED SETTINGS =================
int speedHigh = 255;
int speedMed  = 220;
int speedLow  = 180;


// ================= OBSTACLE =================
const int obstacleStopDistance = 30;


// =====================================================
// RGB LED FUNCTIONS (COMMON CATHODE)
// =====================================================
void ledOff() {
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
}


void ledRed() {        // Obstacle
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
}


void ledBlue() {       // Searching
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, HIGH);
}


void ledGreen() {      // Target reached
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, LOW);
}


// =====================================================
// ULTRASONIC DISTANCE FUNCTION
// =====================================================
float checkdistance() {
  digitalWrite(Trig_Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig_Pin, LOW);


  float distance = pulseIn(Echo_Pin, HIGH) / 58.0;
  delay(20);
  return distance;
}


// =====================================================
// MOTOR FUNCTIONS
// =====================================================
void driveForward() {
  digitalWrite(leftMotorForwardPin, LOW);
  digitalWrite(leftMotorBackwardPin, HIGH);
  digitalWrite(rightMotorForwardPin, LOW);
  digitalWrite(rightMotorBackwardPin, HIGH);
}


void driveBackward() {
  digitalWrite(leftMotorForwardPin, HIGH);
  digitalWrite(leftMotorBackwardPin, LOW);
  digitalWrite(rightMotorForwardPin, HIGH);
  digitalWrite(rightMotorBackwardPin, LOW);
}


void turnRight() {
  digitalWrite(leftMotorForwardPin, HIGH);
  digitalWrite(leftMotorBackwardPin, LOW);
  digitalWrite(rightMotorForwardPin, LOW);
  digitalWrite(rightMotorBackwardPin, HIGH);
}


void turnLeft() {
  digitalWrite(leftMotorForwardPin, LOW);
  digitalWrite(leftMotorBackwardPin, HIGH);
  digitalWrite(rightMotorForwardPin, HIGH);
  digitalWrite(rightMotorBackwardPin, LOW);
}


void stopDriving() {
  digitalWrite(leftMotorForwardPin, LOW);
  digitalWrite(leftMotorBackwardPin, LOW);
  digitalWrite(rightMotorForwardPin, LOW);
  digitalWrite(rightMotorBackwardPin, LOW);
}


// =====================================================
// OBSTACLE AVOIDANCE
// =====================================================
bool handleObstacle() {
  float d = checkdistance();
  Serial.print("Front distance: ");
  Serial.println(d);


  if (d > 0 && d <= obstacleStopDistance) {
    ledRed();                 // OBSTACLE → RED
    stopDriving();
    delay(150);


    // scan left
    myservo.write(180);
    delay(350);
    float leftD = checkdistance();


    // scan right
    myservo.write(0);
    delay(350);
    float rightD = checkdistance();


    // return servo to front
    myservo.write(90);
    delay(200);


    if (leftD > rightD) {
      turnLeft();
      delay(350);
    } else {
      turnRight();
      delay(350);
    }


    return true;
  }


  return false;
}


// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(9600);


  pinMode(leftMotorForwardPin, OUTPUT);
  pinMode(leftMotorBackwardPin, OUTPUT);
  pinMode(rightMotorForwardPin, OUTPUT);
  pinMode(rightMotorBackwardPin, OUTPUT);
  pinMode(leftMotorSpeedPin, OUTPUT);
  pinMode(rightMotorSpeedPin, OUTPUT);


  pinMode(Trig_Pin, OUTPUT);
  pinMode(Echo_Pin, INPUT);


  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);


  myservo.attach(A2);
  myservo.write(90);


  ledOff();
}


// =====================================================
// LOOP
// =====================================================
void loop() {


  // OBSTACLE CHECK FIRST
  if (handleObstacle()) return;


  // SEARCHING FOR SOUND → BLUE
  ledBlue();


  stopDriving();
  delay(300);


  mic1_min = mic2_min = 1023;
  mic1_max = mic2_max = 0;


  unsigned long start_time = millis();


  while (millis() - start_time < sample_window) {
    mic1 = analogRead(mic1pin);
    mic2 = analogRead(mic2pin);


    mic1_min = min(mic1_min, mic1);
    mic1_max = max(mic1_max, mic1);
    mic2_min = min(mic2_min, mic2);
    mic2_max = max(mic2_max, mic2);
  }


  amp1 = mic1_max - mic1_min;
  amp2 = mic2_max - mic2_min;
  difference = amp1 - amp2;
  int avgAmp = (amp1 + amp2) / 2;


  Serial.print("Mic_right Amp: ");
  Serial.print(amp1);
  Serial.print(" | Mic_left Amp: ");
  Serial.print(amp2);
  Serial.print(" | Difference: ");
  Serial.print(difference);
  Serial.print(" | AvgAmp: ");
  Serial.println(avgAmp);


  // MAX THRESHOLD → GREEN
  if (avgAmp > maxThreshold) {
    ledGreen();
    stopDriving();
    return;
  }


  // SPEED SCALING
  int motorSpeed = speedHigh;
  if (avgAmp > 150) motorSpeed = speedLow;
  else if (avgAmp > 80) motorSpeed = speedMed;


  analogWrite(leftMotorSpeedPin, motorSpeed);
  analogWrite(rightMotorSpeedPin, motorSpeed);


  // WEAK SOUND → KEEP SEARCHING
  if (amp1 < lowSoundThreshold && amp2 < lowSoundThreshold) {
    stopDriving();
    return;
  }


  // STEERING
  if (difference > turnDifferenceThreshold) {
    turnLeft();
    delay(180);
    driveForward();
    delay(250);
  }
  else if (difference < -turnDifferenceThreshold) {
    turnRight();
    delay(180);
 driveForward();
    delay(250);
  }
  else {
    driveForward();
    delay(400);
  }
}                