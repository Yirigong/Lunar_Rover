#include <SPI.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10

const int Up = 2;
const int Right = 3;
const int Down = 4;
const int Left = 5;

RF24 radio(CE_PIN, CSN_PIN);
unsigned long lastSendTime = 0;

const byte rover_address[6] = {'R','x','A','A','A'};
const byte router_address[6] = {'R','x','B','B','B'};

byte address[6] = {};


void setup() {
  Serial.begin(9600);
  Serial.println("Serial ON");
  delay(500);
  
  for(int i = 0; i < 6 ; i++){
  address[i] = router_address[i];
}

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
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
  ListeningRF(); 
}

void WriteRF() {
  if(digitalRead(Up) == LOW){
    sendCommand("Up");
  }
  else if(digitalRead(Right) == LOW){
    sendCommand("Right");
  }
  else if(digitalRead(Down) == LOW){
    sendCommand("Down");
  }
  else if(digitalRead(Left) == LOW){
    sendCommand("Left");
  }
  else if(millis() - lastSendTime >= 1000){
    sendCommand("HB");  
    lastSendTime = millis();
  }
}

void sendCommand(const char* command) {
  radio.stopListening();
  radio.openWritingPipe(address);
  radio.write(command, strlen(command) + 1);
}

void ListeningRF() {
  radio.openReadingPipe(1,address);
  radio.startListening();  
  if (radio.available()) { 
    char receivedMessage[32] = ""; 
    radio.read(&receivedMessage, sizeof(receivedMessage)); 
    Serial.print("Received Message: ");
    Serial.println(receivedMessage);  
  }
}
