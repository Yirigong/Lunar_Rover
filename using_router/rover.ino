#include <SPI.h>
#include <RF24.h>
#include <string.h>

#define ENA 2
#define ENB 7
#define IN1 3
#define IN2 4
#define IN3 5
#define IN4 6
#define CE_PIN 8
#define CSN_PIN 9
#define TRIG_PIN A1
#define ECHO_PIN A0

RF24 radio(CE_PIN, CSN_PIN);

const byte controller_address[6] = {'R','x','A','A','A'};
const byte router_address[6] = {'R','x','C','C','C'};

byte address[6] = {};

unsigned long lastReceiveTime = 0;
unsigned long lastSendTime = 0;
const unsigned long timeout = 10000;

void setup() {
  Serial.begin(2400);
  Serial.println("Serial ON");  
  delay(500);

  for(int i = 0; i<6;i++){
    address[i] = router_address[i];
  }

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  Serial.println("Motor ON");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("Hyper ON");

  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();
  Serial.println("Radio ON");
  delay(500);
  lastReceiveTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastReceiveTime >= timeout) {
    Serial.println("connect die... Reboot!");
    asm volatile("jmp 0");
  }

  if (radio.available()) {
    char receivedMessage[32] = "";
    radio.read(&receivedMessage, sizeof(receivedMessage));
    Serial.println(receivedMessage);
    usemotor(receivedMessage); 
  } else {
    stopMotors();  
  }

  if (millis() - lastSendTime >= 500) {
    sendDistance();  
    lastSendTime = millis();
  }
}

void usemotor(char* receivedMessage) {
  if (strcmp(receivedMessage, "HB") == 0) {
    lastReceiveTime = millis();
  }
  else if (strcmp(receivedMessage, "Up") == 0) {
    stopMotors();
    moveMotor1Forward();
    moveMotor2Forward();
  }
  else if (strcmp(receivedMessage, "Down") == 0) {
    stopMotors();
    moveMotor1Backward();
    moveMotor2Backward();
  }
  else if (strcmp(receivedMessage, "Left") == 0) {
    stopMotors();
    moveMotor1Backward();
    moveMotor2Forward();
  }
  else if (strcmp(receivedMessage, "Right") == 0) {
    stopMotors();
    moveMotor1Forward();
    moveMotor2Backward();
  }
}

double getDistance() {
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  double cycletime = pulseIn(ECHO_PIN, HIGH);
  double distance = (cycletime * 0.0343) / 2; 
  return distance; 
}

void sendDistance() {
  radio.stopListening();  
  radio.openWritingPipe(address); 

  double distance = getDistance(); 
  char Message[32];
  dtostrf(distance, 6, 2, Message);  
  radio.write(Message, sizeof(Message)); 
  Serial.print("Sent Distance: ");
  Serial.println(Message);  

  radio.startListening();  
}

void moveMotor1Forward() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, 255); 
}

void moveMotor1Backward() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 255);
}

void moveMotor2Forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 255);
}

void moveMotor2Backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 255);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
