// RADIO REMOTE (TRANSMITTER) BY CAM (MINIMAL VERSION, AS COMPACT AS I CAN GET IT)
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#define CE_PIN 9
#define CSN_PIN 10
#define SIG_LED_PIN 8
#define BTN_PWR_PIN 4
#define BTN_MDE_PIN 5
#define BTN_SUP_PIN 3
#define BTN_SDN_PIN 2
// INITIALIZE ARRAYS AND RADIO OBJECT
int prvBtnSt[4], curBtnSt[4], pns[4] = {0, 0, 0, 0};
char dataToSend[1] = "0"; // this must match dataRecieved in the RX
const byte slaveAddress[5] = {'R','x','A','A','A'};
RF24 radio(CE_PIN, CSN_PIN);
// SETUP: RUNS ONCE. SETS UP RADIO AND PIN MODES BEFORE ANYTHING ELSE
void setup() {
  Serial.begin(9600);
  int pnstmp[] = {BTN_PWR_PIN, BTN_MDE_PIN, BTN_SUP_PIN, BTN_SDN_PIN};
  for (int i=0; i<4; i++) pns[i] = pnstmp[i];
  free(pnstmp);
  for (int p=0; p<sizeof(pns); p++) pinMode(pns[p], INPUT_PULLUP);
  pinMode(SIG_LED_PIN, OUTPUT);
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(3,5); // delay, count
  radio.openWritingPipe(slaveAddress);
}
// UPDATE AND PROCESS BUTTON INPUTS ON THE DEFINED PINS ABOVE
void updateSt(int pin, int idx) {if (digitalRead(pin) == LOW) curBtnSt[idx] = 1; else curBtnSt[idx] = 0;}
void processBtnInp(int idx) {
  if (btnJustPressed(idx)) {
    dataToSend[0] = (char)(idx+48);
    Serial.println(dataToSend);
    send(dataToSend);
  }
}
// LOOPS EVERY PROGRAM TICK, UPDATES BUTTON INPUT.
void loop() {
  for (int i=0; i<4; i++) updateSt(pns[i], i);
  for (int i=0; i<4; i++) processBtnInp(i);
  for (int i=0; i<4; i++) prvBtnSt[i] = curBtnSt[i];
}
// FUNCTION TO CHECK IF A BUTTON HAS BEEN PRESSED THIS PROGRAM TICK
bool btnJustPressed(int i) {
  if (curBtnSt[i] == 1 && prvBtnSt[i] == 0) return true;
  return false;
}
// FUNCTION TO FLASH THE SIGNAL LED ONCE WITH DEFINED DELAYS
void blinkSigLED(int del1, int del2) {
  digitalWrite(SIG_LED_PIN, HIGH);
  delay(del1);
  digitalWrite(SIG_LED_PIN, LOW);
  delay(del2);
}
// FUNCTION TO FLASH THE SIGNAL LED MANY TIMES TO INDICATE AN ERROR WITH THE RADIO TRANSMISSION
void flashSigLED() {for (int c=0; c<15; c++) blinkSigLED(10,35);}
// FUNCTION TO SEND A RADIO MESSAGE USING THE ABOVE DEFINED VARIABLES.
void send(char msg[]) {
  if (radio.write(&msg, sizeof(msg))) blinkSigLED(50,0);
  else flashSigLED();
}
