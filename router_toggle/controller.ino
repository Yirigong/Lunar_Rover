#include <SPI.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10

const int Up = 2;
const int Right = 3;
const int Down = 4;
const int Left = 5;
const int Small_Left = 7;
const int Small_Right = 6;

RF24 radio(CE_PIN, CSN_PIN);
unsigned long lastSendTime = 0;

const byte rover_address[6] = {'R','x','A','A','A'}; //로버와 직접 통신할 때 사용하는 주소
const byte router_address[6] = {'R','x','B','B','B'}; //라우터를 통해 통신할 때 사용하는 주소

byte address[6] = {}; //직접 사용할 주소


void setup() {
  Serial.begin(9600);
  Serial.println("Serial ON");
  delay(500);
  
  for(int i = 0; i < 6 ; i++){
    address[i] = rover_address[i];  //사용할 주소를 설정하는 부분
  }

  radio.begin(); //rf통신 시작
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
  Serial.println("Radio ON");
  delay(500);

  pinMode(Up, INPUT_PULLUP); //버튼 사용을 위한 설정
  pinMode(Right, INPUT_PULLUP);
  pinMode(Down, INPUT_PULLUP);
  pinMode(Left, INPUT_PULLUP);
  Serial.println("Button ON");
} 

void loop() {
  WriteRF(); //입력한 명령과 HB를 로버 또는 라우터로 보냄
  ListeningRF(); //초음파 센서로 측정한 거리를 받아옴
  SetChannel();
}


void sendCommand(const char* command) {  //커맨드를 설정한 주소로 보냄
  radio.stopListening();
  radio.openWritingPipe(address);
  radio.write(command, strlen(command) + 1); //NULL문자 때문에 크기+1
}

void WriteRF() {
  if(digitalRead(Up) == LOW){
    sendCommand("Up");
    delay(300);
  }
  else if(digitalRead(Right) == LOW){
    sendCommand("Right");
    delay(300);
  }
  else if(digitalRead(Down) == LOW){
    sendCommand("Stop");
    delay(300);
  }
  else if(digitalRead(Left) == LOW){
    sendCommand("Left");
    delay(300);
  }
  else if(millis() - lastSendTime >= 1000){ //연결 확인을 위해 HeartBeat방식 사용
    sendCommand("HB");  
    lastSendTime = millis();
  }
}


void ListeningRF() { //설정한 주소로부터 메시지를 받아 화면에 출력함ㄴ
  radio.openReadingPipe(1,address);
  radio.startListening();  
  if (radio.available()) { 
    char receivedMessage[32] = ""; 
    radio.read(&receivedMessage, sizeof(receivedMessage)); 
    Serial.print("Received Message: ");
    Serial.println(receivedMessage);  
  }
}

void SetChannel(){
  if(digitalRead(Small_Left)==LOW){
    Serial.println("Change Channel to Direct");
    sendCommand("Change_to_direct");
    for(int i = 0; i < 6 ; i++){
      address[i] = rover_address[i];  //사용하는 주소를 로버 주소로 변경 -> 직접 통신
    }
    delay(1000);
  }
  else if(digitalRead(Small_Right) == LOW){
    Serial.println("Change Channel to Router");
    sendCommand("Change_to_router");
    for(int i = 0; i < 6 ; i++){
      address[i] = router_address[i];  //사용하는 주소를 라우터 주소로 변경 -> 라우터 통신
    }
    delay(1000);
  }
}
