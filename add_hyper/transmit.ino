#include <SPI.h>
#include <RF24.h>

// CE와 CSN 핀 정의 (아두이노 보드에 따라 변경 가능)
#define CE_PIN   9
#define CSN_PIN  10

const int Up = 2;
const int Right = 3;
const int Down = 4;
const int Left = 5;

// RF24 객체 생성
RF24 radio(CE_PIN, CSN_PIN);
unsigned long lastSendTime = 0;

// 수신 주소 설정 (바이트 배열로 정의)
const byte address[6] = {'R','x','A','A','A'};
const byte router_address[6]= {'R','x','C','C','C'};

void setup() {
  // 시리얼 통신 시작
  Serial.begin(9600);
  Serial.println("Serial ON");
  delay(500);
  // RF24 모듈 초기화
  radio.begin();
  radio.openWritingPipe(router_address); // 송신 파이프 열기
  radio.setPALevel(RF24_PA_MAX);  // 전송 전력 설정
  radio.stopListening();          // 송신 모드 설정
  Serial.println("Radio ON");
  delay(500);

  pinMode(Up, INPUT_PULLUP);
  pinMode(Right, INPUT_PULLUP);
  pinMode(Down, INPUT_PULLUP);
  pinMode(Left, INPUT_PULLUP);
  Serial.println("Button ON");
} 

void loop() {
  WriteRF();
}

void WriteRF(){
  if(digitalRead(Up) == LOW){
    radio.write("Up", sizeof("Up"));
    Serial.println("Up");
  }
  if(digitalRead(Right) == LOW){
    radio.write("Right", sizeof("Right"));
    Serial.println("Right");
  }
  if(digitalRead(Down)== LOW){
    radio.write("Down", sizeof("Down"));
    Serial.println("Down");
  }
  if(digitalRead(Left) == LOW){
    radio.write("Left", sizeof("Left"));
    Serial.println("Left");
  }
  if(millis() - lastSendTime >= 1000){
    radio.write("HB", sizeof("HB"));
    lastSendTime = millis();
    Serial.println("HB");
  }
}


