#include <SPI.h>
#include <RF24.h>
#include <string.h>  // 문자열 비교를 위한 라이브러리

#define ENA 2
#define ENB 7
#define IN1 3
#define IN2 4
#define IN3 5
#define IN4 6
#define CE_PIN   8
#define CSN_PIN  9
#define TRIG_PIN A1
#define ECHO_PIN A0



// RF24 객체 생성
RF24 radio(CE_PIN, CSN_PIN);

// 송신기와 동일한 수신 주소 설정 (송신기와 동일하게 바이트 배열로 정의)
const byte direct_address[6] = {'R','x','A','A','A'};
const byte router_address[6] = {'R','x','C','C','C'};

unsigned long lastReceiveTime = 0;
const unsigned long timeout = 5000;

void setup() {
  // 시리얼 통신 시작
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

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("Hyper ON");

  // RF24 모듈 초기화
  radio.begin();
  radio.openReadingPipe(1, router_address); // 수신 파이프 열기
  radio.setPALevel(RF24_PA_MAX);     // 전송 전력 설정
  radio.startListening();            // 수신 모드 설정
  Serial.println("Radio ON");
  delay(500);
  lastReceiveTime = millis();
}

float cycletime;
float distance;

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastReceiveTime >= timeout){
    Serial.println("connect die... Reboot!");
    asm volatile("jmp 0");
  }
  // 수신할 데이터가 있는지 확인
  if (radio.available()) {
    char receivedMessage[32] = "";  // 수신 버퍼 (최대 32바이트)

    // 데이터 크기가 0보다 크면 읽기
    radio.read(&receivedMessage, sizeof(receivedMessage)); // 데이터 읽기
    
    // 수신된 메시지를 시리얼 모니터에 출력
    Serial.print("Received message: ");
    Serial.println(receivedMessage);
    usemotor(receivedMessage);
  }
  else{
    stopMotors();
  }
  getDistance();
}


void usemotor(char* receivedMessage){
  if(strcmp(receivedMessage, "HB")==0){
      lastReceiveTime = millis();
    }
    // 문자열 비교를 사용한 제어
  else if (strcmp(receivedMessage, "Up") == 0) { // 앞으로
    stopMotors();
    moveMotor1Forward();
    moveMotor2Forward();
  }
  else if (strcmp(receivedMessage, "Down") == 0) { // 뒤로
    stopMotors();
    moveMotor1Backward();
    moveMotor2Backward();
  }
  else if (strcmp(receivedMessage, "Left") == 0) { // 좌회전
    stopMotors();
    moveMotor1Backward();
    moveMotor2Forward();
  }
  else if (strcmp(receivedMessage, "Right") == 0) { // 우회전
    stopMotors();
    moveMotor1Forward();
    moveMotor2Backward();
  }
}

void getDistance(){
  digitalWrite(TRIG_PIN, HIGH);
  delay(10);
  digitalWrite(TRIG_PIN, LOW);
  cycletime = pulseIn(ECHO_PIN, HIGH);
  distance = ((340*cycletime)/10000)/2;
  Serial.print("Distance:");
  Serial.print(distance);
  Serial.println("cm");
}

// 모터 1 앞으로 회전
void moveMotor1Forward() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, 255); 
}

// 모터 1 뒤로 회전
void moveMotor1Backward() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 255);
}

// 모터 2 앞으로 회전
void moveMotor2Forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 255);
}

// 모터 2 뒤로 회전
void moveMotor2Backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 255);
}

// 모터 멈춤
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
