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

const byte controller_address[6] = {'R','x','B','B','B'}; //컨트롤러로부터 메시지를 받을 때 쓰는 주소
const byte rover_address[6] = {'R','x','C','C','C'}; //로버로 메시지를 보낼 때 쓰는 주소

RF24 radio(CE_PIN, CSN_PIN);

const long timeout = 5000;
unsigned long lastReceiveTime = 0;
unsigned long checktime = 0;
bool check_routing = false;

bool isMovingUp = false;
bool isMovingDown = false;
bool isMovingLeft = false;
bool isMovingRight = false;

void setup() {
    Serial.begin(9600);
    Serial.println("Serial ON");  
    delay(500);

    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    Serial.println("Motor ON");

    radio.begin(); //RF통신 시작
    radio.openReadingPipe(1, controller_address);  
    radio.openWritingPipe(rover_address);           
    radio.setPALevel(RF24_PA_MAX);
    radio.startListening(); 
}


void loop() {
  if (radio.available()) {
    char receivedMessage[32] = "";
    radio.read(&receivedMessage, sizeof(receivedMessage)); //컨트롤러에서 메시지를 받아옴
    Serial.println(receivedMessage);
    if(strcmp(receivedMessage, "HB") == 0){
      lastReceiveTime = millis();
    }
    else if(strcmp(receivedMessage, "Change_to_router")==0){  
      check_routing = true;
      lastReceiveTime = millis();
    }
    else if(strcmp(receivedMessage, "Change_to_direct")==0){
      check_routing = false;
    }
    if(check_routing == false){
      usemotor(receivedMessage);
    }
    sendMessageToRover(receivedMessage);
  }
  if(millis()-lastReceiveTime >= timeout && check_routing == true){
      Serial.println("connect die... Reboot!");
      delay(5000);
      asm volatile("jmp 0");
  }
  if(millis()-checktime >=100){
    checktime = millis();
    Serial.println(millis()-lastReceiveTime);
  }
} 

void usemotor(char* receivedMessage) {
  if (strcmp(receivedMessage, "Up") == 0) {
    toggleMovement(&isMovingUp, &moveMotor1Forward, &moveMotor2Forward);
  }
  else if (strcmp(receivedMessage, "Left") == 0) {
    toggleMovement(&isMovingLeft, &moveMotor1Backward, &moveMotor2Forward);
  }
  else if (strcmp(receivedMessage, "Right") == 0) {
    toggleMovement(&isMovingRight, &moveMotor1Forward, &moveMotor2Backward);
  }
  else if (strcmp(receivedMessage, "Stop") == 0) {
    resetMovement();
  }
}


void toggleMovement(bool* movementFlag, void (*motorFunc1)(), void (*motorFunc2)()) {
  if (*movementFlag) {
    *movementFlag = false;
    stopMotors();
  } else {
    *movementFlag = true;
    motorFunc1();
    motorFunc2();
  }
}

void moveMotor1Forward() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 255); 
}

void moveMotor1Backward() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, 255);
}

void moveMotor2Forward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 255);
}

void moveMotor2Backward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 255);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void resetMovement() {
  isMovingUp = false;
  isMovingDown = false;
  isMovingLeft = false;
  isMovingRight = false;
  stopMotors();
}

void sendMessageToRover(const char* message) {
    radio.stopListening(); 
    radio.write(message, strlen(message) + 1); //로버로 메시지를 보내줌
    Serial.println(message);
    radio.startListening(); 
}
