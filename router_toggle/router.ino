#include <SPI.h>
#include <RF24.h>
#include <string.h>

#define CE_PIN 8
#define CSN_PIN 9

const byte controller_address[6] = {'R','x','B','B','B'}; //컨트롤러로부터 메시지를 받을 때 쓰는 주소
const byte rover_address[6] = {'R','x','C','C','C'}; //로버로 메시지를 보낼 때 쓰는 주소

RF24 radio(CE_PIN, CSN_PIN);

const long timeout = 5000;
unsigned long lastReceiveTime = 0;
unsigned long checktime = 0;
bool check_routing = false;

void setup() {
    Serial.begin(9600);
    Serial.println("Serial ON");  
    delay(500);

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

void sendMessageToRover(const char* message) {
    radio.stopListening(); 
    radio.write(message, strlen(message) + 1); //로버로 메시지를 보내줌
    Serial.println(message);
    radio.startListening(); 
}
