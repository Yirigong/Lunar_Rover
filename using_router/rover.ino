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

const byte controller_address[6] = {'R','x','A','A','A'}; //컨트롤러와 직접 통신할 때 사용하는 주소
const byte router_address[6] = {'R','x','C','C','C'}; //라우터를 사용해 통신 할 때 사용하는 주소

byte address[6] = {}; //직접 사용할 주소

unsigned long lastReceiveTime = 0;
unsigned long lastSendTime = 0;
const unsigned long timeout = 10000;

void setup() {
  Serial.begin(2400);
  Serial.println("Serial ON");  
  delay(500);

  for(int i = 0; i<6;i++){
    address[i] = router_address[i]; //사용할 주소를 설정하는 부분
  }

  pinMode(ENA, OUTPUT); //모터 설정
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  Serial.println("Motor ON");

  pinMode(TRIG_PIN, OUTPUT); //초음파 센서 설정
  pinMode(ECHO_PIN, INPUT);
  Serial.println("Hyper ON");

  radio.begin(); //RF통신 시작
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();
  Serial.println("Radio ON");
  delay(500);
  lastReceiveTime = millis();
}

void loop() {
  unsigned long currentTime = millis(); //10초이상 HB를 못받으면 로버 재부팅
  if (currentTime - lastReceiveTime >= timeout) {
    Serial.println("connect die... Reboot!");
    asm volatile("jmp 0");
  }

  if (radio.available()) {
    char receivedMessage[32] = "";
    radio.read(&receivedMessage, sizeof(receivedMessage)); //설정한 주소로부터 메시지를 받아옴
    Serial.println(receivedMessage);
    usemotor(receivedMessage); //받은 명령에 따라 모터 작동
  } else {
    stopMotors();  
  }

  if (millis() - lastSendTime >= 500) { //0.5초마다 초음파 센서로 받아온 거리 전송
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

double getDistance() { //거리 계산
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  double cycletime = pulseIn(ECHO_PIN, HIGH);
  double distance = (cycletime * 0.0343) / 2; 
  return distance; 
}

void sendDistance() { //거리 전송
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
