// RADIO REMOTE (TRANSMITTER) BY CAM

#include  <SPI.h>             // SPI communication to radio transciever
#include  <nRF24L01.h>        // NRF20L01 radio transmitter board is what is being used here
#include  <RF24.h>            // RF24 library to simplify radio communication code

#define   CE_PIN         9    // The pin that connects to CE on the transciever (refer to electrical diagram)
#define   CSN_PIN        10   // The pin that connects to CSN on the transciever (refer to electrical diagram)
#define   SIG_LED_PIN    8    // Signal led pin, for showing signals were sent
#define   BTN_PWR_PIN    4    // The pull-up pin that represents the "standby" button for the remote
#define   BTN_MDE_PIN    5    // The pull-up pin that represents the "mode" button for the remote
#define   BTN_SUP_PIN    3    // The pull-up pin that represents the "setting up" button for the remote
#define   BTN_SDN_PIN    2    // The pull-up pin that represents the "setting down" button for the remote

int prvBtnSt[] = {0, 0, 0, 0};                         // Previous button states at the current tick
int curBtnSt[] = {0, 0, 0, 0};                         // Current button states at the current tick
const byte slaveAddress[5] = {'R','x','A','A','A'};    // The address for the radio transciever board
char msg[1] = "0";
RF24 radio(CE_PIN, CSN_PIN);                           // Initialize the radio object

void setup() {
  // Set up the buttons
  pinMode(BTN_PWR_PIN, INPUT_PULLUP);
  pinMode(BTN_MDE_PIN, INPUT_PULLUP);
  pinMode(BTN_SUP_PIN, INPUT_PULLUP);
  pinMode(BTN_SDN_PIN, INPUT_PULLUP);
  // Set up LED
  pinMode(SIG_LED_PIN, OUTPUT);
  // Set up the radio library
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(3,5); // delay, count
  radio.openWritingPipe(slaveAddress);
}

void loop() {
  // Update button states based on if the voltage at the button pins is low
  if (digitalRead(BTN_PWR_PIN) == LOW) curBtnSt[0] = 1; else curBtnSt[0] = 0;
  if (digitalRead(BTN_MDE_PIN) == LOW) curBtnSt[1] = 1; else curBtnSt[1] = 0;
  if (digitalRead(BTN_SUP_PIN) == LOW) curBtnSt[2] = 1; else curBtnSt[2] = 0;
  if (digitalRead(BTN_SDN_PIN) == LOW) curBtnSt[3] = 1; else curBtnSt[3] = 0;
  // Process button inputs for sending instructions via RF
  if (btnJustPressed(0)) setAndSend('0');
  else if (btnJustPressed(1)) setAndSend('1');
  else if (btnJustPressed(2)) setAndSend('2');
  else if (btnJustPressed(3)) setAndSend('3');
  // Set previous button state array so we can check if a button is 'just' pressed.
  for (int i=0; i<4; i++) prvBtnSt[i] = curBtnSt[i];
}

bool btnJustPressed(int i) {
  if (curBtnSt[i] == 1 && prvBtnSt[i] == 0) return true;     // if the states are different between current & previous (and current state is down)
  return false;                                              // the button isn't pressed in this tick
}

void flashSigLED(bool sucess) {  // flashes the signal LED (yellow) on the board when a signal is attempted.
  if (sucess == true) {
    digitalWrite(SIG_LED_PIN, HIGH);
    delay(50);
    digitalWrite(SIG_LED_PIN, LOW);
    return;
  }
  for (int c=0; c<10; c++) {
    digitalWrite(SIG_LED_PIN, HIGH);
    delay(10);
    digitalWrite(SIG_LED_PIN, LOW);
    delay(40);
  }
}

void setAndSend(char m) {
  msg[0] = m;
  send();
}

void send() {
  bool result = radio.write(&msg, sizeof(msg));
  if (result) flashSigLED(true);
  else flashSigLED(false);
}
