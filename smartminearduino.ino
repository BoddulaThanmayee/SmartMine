#include <AFMotor.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

#define REAR_IR A0
#define FRONT_IR A1

#define GAS_SENSOR A2
#define GAS_THRESHOLD 500

#define TRIG_PIN 2
#define ECHO_PIN 13

#define SERVO_PIN 10

Servo frontServo;

#define BUZZER_PIN 9

#define DRIVE_SPEED 210
#define TURN_SPEED 230

#define SAFE_DISTANCE 35
#define CRITICAL_DISTANCE 15

#define LEFT_ANGLE 45
#define CENTER_ANGLE 90
#define RIGHT_ANGLE 135

enum RoverMode
{
  AUTO_MODE,
  MANUAL_MODE,
  EMERGENCY_MODE
};

RoverMode currentMode = AUTO_MODE;

int frontState = HIGH;
int rearState = HIGH;

int gasValue = 0;

long distanceCM = 0;

char manualCommand = 'S';

unsigned long lastTelemetry = 0;

void setMotorSpeed(int speed)
{
  speed = constrain(speed, 0, 255);

  motor1.setSpeed(speed);
  motor2.setSpeed(speed);
  motor3.setSpeed(speed);
  motor4.setSpeed(speed);
}

void moveForward()
{
  setMotorSpeed(DRIVE_SPEED);

  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void moveBackward()
{
  setMotorSpeed(DRIVE_SPEED);

  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void turnLeft()
{
  setMotorSpeed(TURN_SPEED);

  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void turnRight()
{
  setMotorSpeed(TURN_SPEED);

  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void stopMotors()
{
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}

long readDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
  {
    return 400;
  }

  long distance = duration * 0.0343 / 2;

  return distance;
}

void readIRSensors()
{
  frontState = digitalRead(FRONT_IR);
  rearState = digitalRead(REAR_IR);
}

void readGasSensor()
{
  gasValue = analogRead(GAS_SENSOR);
}

bool gasDanger()
{
  return gasValue >= GAS_THRESHOLD;
}

void buzzerOn()
{
  digitalWrite(BUZZER_PIN, HIGH);
}

void buzzerOff()
{
  digitalWrite(BUZZER_PIN, LOW);
}

void warningBeep()
{
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

long scanDirection(int angle)
{
  frontServo.write(angle);
  delay(300);

  return readDistance();
}

void updateDisplay()
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("SMARTMINE ROVER");

  display.setCursor(0, 10);
  display.print("MODE: ");

  if (currentMode == AUTO_MODE)
    display.println("AUTO");
  else if (currentMode == MANUAL_MODE)
    display.println("MANUAL");
  else
    display.println("EMERGENCY");

  display.setCursor(0, 20);
  display.print("DIST: ");
  display.print(distanceCM);
  display.println("cm");

  display.setCursor(0, 30);
  display.print("FRONT: ");

  if (frontState == LOW)
    display.println("OBJ");
  else
    display.println("CLEAR");

  display.setCursor(0, 40);
  display.print("REAR : ");

  if (rearState == LOW)
    display.println("OBJ");
  else
    display.println("CLEAR");

  display.setCursor(0, 50);
  display.print("GAS: ");

  if (gasDanger())
    display.println("DANGER");
  else
    display.println(gasValue);

  display.display();
}

void startupDisplay()
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(10, 15);
  display.println("SMARTMINE ROVER");

  display.setCursor(15, 30);
  display.println("SYSTEM READY");

  display.setCursor(20, 45);
  display.println("AUTO MODE");

  display.display();

  delay(2000);
}

void autonomousNavigation()
{
  distanceCM = readDistance();

  readIRSensors();

  readGasSensor();

  if (gasDanger())
  {
    stopMotors();
    buzzerOn();

    Serial.println("ALERT:GAS");

    return;
  }

  buzzerOff();

  if (distanceCM <= CRITICAL_DISTANCE)
  {
    stopMotors();

    warningBeep();

    delay(200);

    moveBackward();

    delay(500);

    stopMotors();

    delay(200);

    long leftDistance = scanDirection(LEFT_ANGLE);
    long rightDistance = scanDirection(RIGHT_ANGLE);

    frontServo.write(CENTER_ANGLE);

    if (leftDistance > rightDistance)
    {
      turnLeft();
      delay(600);
    }
    else
    {
      turnRight();
      delay(600);
    }

    stopMotors();

    return;
  }

  if (distanceCM <= SAFE_DISTANCE)
  {
    stopMotors();

    warningBeep();

    delay(150);

    long leftDistance = scanDirection(LEFT_ANGLE);
    long rightDistance = scanDirection(RIGHT_ANGLE);

    frontServo.write(CENTER_ANGLE);

    if (leftDistance > rightDistance &&
        leftDistance > SAFE_DISTANCE)
    {
      turnLeft();
      delay(450);
    }
    else if (rightDistance > SAFE_DISTANCE)
    {
      turnRight();
      delay(450);
    }
    else
    {
      moveBackward();
      delay(500);

      turnRight();
      delay(700);
    }

    stopMotors();

    return;
  }

  if (frontState == LOW && rearState == HIGH)
  {
    stopMotors();

    delay(150);

    moveBackward();

    delay(400);

    return;
  }

  if (rearState == LOW && frontState == HIGH)
  {
    stopMotors();

    delay(150);

    moveForward();

    delay(400);

    return;
  }

  moveForward();
}

void manualControl()
{
  switch (manualCommand)
  {
    case 'F':
      moveForward();
      break;

    case 'B':
      moveBackward();
      break;

    case 'L':
      turnLeft();
      break;

    case 'R':
      turnRight();
      break;

    case 'S':
      stopMotors();
      break;

    default:
      stopMotors();
      break;
  }
}

void processCommand(char command)
{
  command = toupper(command);

  if (command == 'A')
  {
    currentMode = AUTO_MODE;
    manualCommand = 'S';
    buzzerOff();
    stopMotors();

    Serial.println("MODE:AUTO");

    return;
  }

  if (command == 'M')
  {
    currentMode = MANUAL_MODE;
    manualCommand = 'S';
    buzzerOff();
    stopMotors();

    Serial.println("MODE:MANUAL");

    return;
  }

  if (command == 'X')
  {
    currentMode = EMERGENCY_MODE;

    stopMotors();
    buzzerOn();

    Serial.println("EMERGENCY:STOP");

    return;
  }

  if (command == 'Z')
  {
    currentMode = AUTO_MODE;

    buzzerOff();
    stopMotors();

    Serial.println("EMERGENCY:CLEARED");

    return;
  }

  if (currentMode == MANUAL_MODE)
  {
    if (command == 'F' ||
        command == 'B' ||
        command == 'L' ||
        command == 'R' ||
        command == 'S')
    {
      manualCommand = command;
    }

    if (command == 'Q')
    {
      frontServo.write(LEFT_ANGLE);
    }

    if (command == 'C')
    {
      frontServo.write(CENTER_ANGLE);
    }

    if (command == 'E')
    {
      frontServo.write(RIGHT_ANGLE);
    }
  }
}

void sendTelemetry()
{
  if (millis() - lastTelemetry < 1000)
    return;

  lastTelemetry = millis();

  Serial.print("DATA,");

  Serial.print("MODE=");

  if (currentMode == AUTO_MODE)
    Serial.print("AUTO");
  else if (currentMode == MANUAL_MODE)
    Serial.print("MANUAL");
  else
    Serial.print("EMERGENCY");

  Serial.print(",DIST=");
  Serial.print(distanceCM);

  Serial.print(",FRONT_IR=");
  Serial.print(frontState);

  Serial.print(",REAR_IR=");
  Serial.print(rearState);

  Serial.print(",GAS=");
  Serial.print(gasValue);

  Serial.println();
}

void setup()
{
  Serial.begin(115200);

  pinMode(FRONT_IR, INPUT_PULLUP);
  pinMode(REAR_IR, INPUT_PULLUP);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(GAS_SENSOR, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  buzzerOff();

  frontServo.attach(SERVO_PIN);
  frontServo.write(CENTER_ANGLE);

  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    while (true)
    {
    }
  }

  stopMotors();

  startupDisplay();

  readIRSensors();
  readGasSensor();

  distanceCM = readDistance();

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(0, 20);
  display.println("SMARTMINE READY");

  display.setCursor(0, 35);
  display.println("SAFETY SYSTEM ON");

  display.display();

  delay(1500);
}

void loop()
{
  while (Serial.available())
  {
    char command = Serial.read();

    processCommand(command);
  }

  if (currentMode == AUTO_MODE)
  {
    autonomousNavigation();
  }
  else if (currentMode == MANUAL_MODE)
  {
    distanceCM = readDistance();

    readIRSensors();

    readGasSensor();

    if (gasDanger())
    {
      stopMotors();
      buzzerOn();
    }
    else
    {
      buzzerOff();
      manualControl();
    }
  }
  else if (currentMode == EMERGENCY_MODE)
  {
    stopMotors();
    buzzerOn();
  }

  updateDisplay();

  sendTelemetry();

  delay(50);
}