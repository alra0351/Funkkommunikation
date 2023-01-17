#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>


RF24 radio(9,8);
RF24Network network(radio);
const uint16_t master=00;
const uint16_t kuka01=01;
const uint16_t kuka02=02;
const uint16_t kuka03=03;
const uint16_t igus01=04;
const uint16_t igus02=05;
const uint16_t linmot=06;
const uint16_t this_node=kuka03;


// Ein-, Ausgänge auf Kuka bezogen
bool Inputs[8]={0,0,0,0,0,0,0,0}; 
bool Outputs[8]={0,0,0,0,0,0,0,0}; 


// Auf den Nano bezogen
uint8_t pinI[8] = {A7, A6, A5, A4, A3, A2, A1, A0}; //umgedreht, da Verkabelung am Arduino so besser war
uint8_t pinO[8] = {1, 0, 2, 3, 4, 5, 6, 7};


// Komprimieren von Daten
uint8_t packBools8(bool *b) {
  uint8_t package = 0;
  for (uint8_t i = 0; i < 8; i++) {
    package = package | b[i];
    if (i != 7) package <<= 1;
  }
  return package;
}

void unpackBools8(uint8_t package, bool *b) {
  for (int8_t i = 7; i > -1; i--) {
    b[i] = package & 0x01;
    package >>= 1;
  }
}

void setup() {
  SPI.begin();
  radio.begin();
  network.begin(90, this_node);  //(channel, node address)
  radio.setDataRate(RF24_2MBPS);
  radio.setPayloadSize(1);  // sendet max. 1 Byte
  radio.setRetries(5,15);   // Delay zwischen Sende-/Empfangsversuchen, Anzahl an Versuchen
  for(uint8_t i = 0 ; i < 8; i++){  //Definition der Ein- und Ausgänge
    pinMode(pinI[i], INPUT);
    pinMode(pinO[i], OUTPUT);
  }
}

void loop() {
  network.update();   // Muss Funktion für Bibliothek


  //===== Receiving =====//
  delay(5);   // hier sollte ein Delay hin, da der loop sonst zu schnell ist und keine Daten empfangen werden werden
  while ( network.available()){   // Empfangen
    RF24NetworkHeader header;
    uint8_t buffer;
    network.read(header, &buffer, sizeof(buffer));
    unpackBools8(buffer, Inputs);
  }

  //===== Sending =====//
  uint8_t package= packBools8(Outputs);
  RF24NetworkHeader header(master);
  network.write(header, &package, sizeof(package));


  //========AUSGÄNGE========
  // Ausgänge vom Kuka einlesen
  for(uint8_t i = 0; i < 8; i++){
    Outputs[i] = (analogRead(pinI[i]) >= 1000) ? true : false;
  }
  
  // Eingänge am Kuka setzen
  for(uint8_t i = 0; i < 8; i++){
    digitalWrite(pinO[i], Inputs[i]);
  }
}
