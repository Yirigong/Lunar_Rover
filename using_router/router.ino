#include <SPI.h>
#include <RF24.h>
#include <string.h>

#define CE_PIN 7
#define CSN_PIN 8

const byte controller_address[6] = {'R','x','B','B','B'};
const byte rover_address[6] = {'R','x','C','C','C'};

RF24 radio(CE_PIN, CSN_PIN);

void setup() {
    Serial.begin(9600);
    Serial.println("Serial ON");  
    delay(500);

    // Initialize the radio
    radio.begin();
    radio.openReadingPipe(1, controller_address);  
    radio.openWritingPipe(rover_address);           
    radio.setPALevel(RF24_PA_MAX);
    radio.startListening(); 
    Serial.println("Radio ON");
}

void loop() {
    if (radio.available()) {
        char receivedMessage[32] = "";
        radio.read(&receivedMessage, sizeof(receivedMessage));
        sendMessageToRover(receivedMessage);
    }
}

void sendMessageToRover(const char* message) {
    radio.stopListening(); 
    radio.write(message, strlen(message) + 1); 
    radio.startListening(); 
}
