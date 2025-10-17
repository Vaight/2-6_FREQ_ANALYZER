#include <FastLED.h>
#include <arduinoFFT.h>

#define LED_PIN         7
#define MIC_PIN         A0
#define NUM_LEDS        256

const uint16_t SAMPLES = 64; //This value MUST ALWAYS be a power of 2
const float signalFrequency = 1000;
const float SAMPLING_FREQ = 5000;
const uint8_t amplitude = 10;

unsigned long sampling_period_us;
unsigned long microseconds;

float vReal[SAMPLES];
float vImag[SAMPLES];

ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, SAMPLES, SAMPLING_FREQ);
int testVal = 0;

class Color {
  public:
  int r, g, b; // 0-255 for each channel.
  Color() : r(0), g(0), b(0) {} // initialized the color to black.
  Color(int rv, int gv, int bv) : r(rv), g(gv), b(bv) {} // if color values are specified, set the values.
  bool isValid() {
    if (r > 255 || g > 255 || b > 255) return false;
    if (r < 0 || g < 0 || b < 0) return false;
    return true;
  }
  void setBrightness(int dMult) {
    r = round(r/dMult);
    g = round(g/dMult);
    b = round(b/dMult);
  }
};

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(9600);
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQ));
  pinMode(MIC_PIN, INPUT);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}

void assignLED(int idx, Color col) {
  if (idx <= NUM_LEDS && col.isValid()) {
    leds[idx] = CRGB(col.r, col.g, col.b);
  }
}

void assignLEDRange(int idx1, int idx2, Color col) {
  if (idx1 <= NUM_LEDS && idx2 <= NUM_LEDS && col.isValid()) {
    for (int i = idx1; i <= idx2; i++) leds[i] = CRGB(col.r, col.g, col.b);
  }
}

void updateDisp() {
  FastLED.show();
}

void drawFrequencyBar(int pos, int height, bool flip, Color col) { // height: 0-16
  assignLEDRange(pos, pos+16, Color(0,0,0));
  if (flip) {
    for (int i = height-1; i >= 0; i--) {
      int led = (pos+15)-i;
      assignLED(led, col);
    }
  }
  else {
    for (int i = 0; i < height; i++) {
      int led = i+pos;
      assignLED(led, col);
    }
  }
}

void loop() {

  int micval = analogRead(MIC_PIN);
  Serial.println(micval);
  /*
  for (int i = 0; i < 16; i++) {
    bool flipPar = false;
    if (i%2) flipPar = true;
    Color bGrad = Color(0+(16*i), 20, 255-(16*i));
    bGrad.setBrightness(5);
    drawFrequencyBar(16*i, random(0,17), flipPar, bGrad);
  }
  updateDisp();
  delay(50);
  */

  for (int i = 0; i < SAMPLES; i++) {
    microseconds = micros();    
    vReal[i] = analogRead(MIC_PIN); // Read mic
    vImag[i] = 0;                   // Imaginary part is 0

    while (micros() - microseconds < sampling_period_us) {
      // wait until sampling period elapses
   }}

  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);  /* Weigh data */
  FFT.compute(FFTDirection::Forward); /* Compute FFT */
  FFT.complexToMagnitude(); /* Compute magnitudes */
  float x = FFT.majorPeak();
  // Rest of the code

  int bins_per_column = (SAMPLES / 2) / 16;  // 64 / 16 = 4 bins per column

  for (int col = 1; col <= 16; col++) {
    double avg = 0;
    for (int i = 0; i < bins_per_column; i++) {
      int bin = col * bins_per_column + i;
      avg += vReal[bin];
    }
    avg /= bins_per_column;

    // Optional: scale avg to display range, e.g. 0-255 or 0-8 levels
    int level = map(avg, 0, 100, 0, 8); // adjust max value as needed
    level = constrain(level, 0, 16);

    int i = col-1;
    Serial.print(level);
    bool flipPar = true;
    if (i%2) flipPar = false;
    Color bGrad = Color(0+(16*i), 20, 255-(16*i));
    bGrad.setBrightness(5);
    drawFrequencyBar(16*i, level, flipPar, bGrad);
    Serial.print(" ");
  }
  updateDisp();
  //delay(50);
  /*
  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB(255,0,0);
    FastLED.show();
    delay(10);
  }
  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB(0,255,0);
    FastLED.show();
    delay(10);
  }
  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB(0,0,255);
    FastLED.show();
    delay(10);
  }
  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB(0,0,0);
    FastLED.show();
    delay(10);
  }
  */
}
