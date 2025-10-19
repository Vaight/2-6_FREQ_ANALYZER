#include <FastLED.h>
#include <arduinoFFT.h>

#define LED_PIN   7
#define MIC_PIN   A0
#define NUM_LEDS  256

const uint16_t samples = 64;       // must be a power of 2, for now, leave it at 64, its a nice round number.
const float signalFreq = 1000;     // essentially functions as resolution, HAS to be less than 1/2 samplingFreq.
const float samplingFreq = 5000;   // samples from the audio input (microphone or otherwise) frequency per second.

unsigned long sampling_period_us;  // the period for samples in microseconds
unsigned long microseconds;        // variable to store a certain timestamp in microseconds so we can calculate how many microseconds a sample is.

float vReal[samples];   // the "x" axis of our 2D array. This will contain the frequencies with respect to a cosine wave via FFT.
float vImag[samples];   // the "y" axis of our 2D array. This will contain the frequencies with respect to a sine wive via FFT.

ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, samples, samplingFreq);  // initialize our FFT with our variables defined above.
CRGB leds[NUM_LEDS]; // initialize the LED display's color array

// ----------------------------------------------- CLASS DECLARATIONS AND CUSTOM FUNCTIONS ----------------------------------------------- //

class Color {                                               // my custom color class for assigning colors to the LEDs
  public:
  int r, g, b;                                              // 0-255 for each channel.
  Color() : r(0), g(0), b(0) {}                             // initialized the color to black.
  Color(int rv, int gv, int bv) : r(rv), g(gv), b(bv) {}    // if color values are specified, set the values.

  bool isValid() {                                          // checking if the color is valid, just for avoiding errors.
    if (r > 255 || g > 255 || b > 255) return false;        // if the color values exceed 255 in any of the channels, it's invalid.
    if (r < 0 || g < 0 || b < 0) return false;              // if the color values are below 0 in any of the channels, it's invalid.
    return true;                                            // otherwise, the color is valid. carry on.
  }
  void setBrightness(int dMult) {                           // function that can "dim" a color by dividing all the rgb values by a multiplier. Easier than modifying voltage to the display
    r = round(r/dMult);                                     // r / division factor = new r
    g = round(g/dMult);                                     // g / division factor = new g
    b = round(b/dMult);                                     // b / division factor = new b
  }
};

void setup() {                                                   // runs once once the program is loaded.
  Serial.begin(9600);                                            // serial communication to the computer for debugging.
  sampling_period_us = round(1000000 * (1.0 / samplingFreq));    // calculate our sampling period (in microseconds) based on our sampling frequency.
  pinMode(MIC_PIN, INPUT);                                       // set our mic input pin to input. 
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);         // initialize the LED display with the WS2812 protocol.
}

void assignLED(int idx, Color col) {         // custom function to assign a color object to a LED index on the matrix. (will probably replace this with an x,y system later for less math elsewhere).
  if (idx <= NUM_LEDS && col.isValid()) {    // if the LED is in the matrix and the color is valid.
    leds[idx] = CRGB(col.r, col.g, col.b);   // set the led value in the array at a certain point to the specified color objects components.
  }
}

void assignLEDRange(int idx1, int idx2, Color col) {                           // very similar to the above function, but can draw LEDs in a range between index 1 and index 2.
  if (idx1 <= NUM_LEDS && idx2 <= NUM_LEDS && col.isValid()) {                // if indexes are within LED matrix boundary.
    for (int i = idx1; i <= idx2; i++) leds[i] = CRGB(col.r, col.g, col.b);   // set the values on that range to the specified color.
  }
}

void display() {   // shorthand function for updating the LED matrix.
  FastLED.show();  // built in FastLED library function to send the buffer data to the display.
}

void drawFrequencyBar(int pos, int height, bool flip, Color col) {  // function to draw a frequency bar at a led index. height: 0-16.
  assignLEDRange(pos, pos+16, Color(0,0,0));                        // clear colors at on the frequency bar range.
  if (flip) {                                                       // since the matrix is wired in a snaking pattern up the display, some bars need to be flipped, displaying values from top to bottom.
    for (int i = height-1; i >= 0; i--) {                           // backwards for loop, assigns leds down the bar based on the height value.
      int led = (pos+15)-i;
      assignLED(led, col);
    }
  }
  else {                                                            // otherwise, the bar is right-side up, and the values fill from bottom to top.
    for (int i = 0; i < height; i++) {                              // forwards for loop, assigns leds up the bar based on the height value.
      int led = i+pos;
      assignLED(led, col);
    }
  }
}

// ----------------------------------------------- MAIN LOOP AND AUDIO PROCESSING ----------------------------------------------- //

void loop() {                                                 // while the arduino is powered on

  for (int i = 0; i < samples; i++) {                         // for loop to sample the mic data for a certain amount of microseconds.
    microseconds = micros();                                  // store the initial microsecond value at the current tick.
    vReal[i] = analogRead(MIC_PIN);                           // read the analog voltage data from the microphone pin.
    vImag[i] = 0;                                             // the imaginary part (y-axis) of the 2d array is set to 0. will be filled in by the FFT.
    while (micros() - microseconds < sampling_period_us) {}   // wait until the sampling period in microseconds has passed.
  }

  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);   // from arduinoFFT wiki, weigh the data by windowing it.
  FFT.compute(FFTDirection::Forward);                         // from arduinoFFT wiki, actually compute the FFT.
  FFT.complexToMagnitude();                                   // from arduinoFFT wiki, convert the vImag (y-axis) into magnitudes for the frequency bands.
  //float xDom = FFT.majorPeak();                             // from arduinoFFT wiki, this value contains the most dominant frequency in the spectrum. Not using this at the moment.

  int bins_per_column = (samples / 2) / 16;                   // 64 / 16 = 4 bins per column for 16 columns. used to round 64 bands into our 16 columns.
  for (int col = 1; col <= 16; col++) {                       // loop over the columns, all 16 of them.
    double avg = 0;                                           // define a variable to hold our averaged value.
    for (int i = 0; i < bins_per_column; i++) {               // add the values of the 4 bands to 1 average
      int bin = col * bins_per_column + i;
      avg += vReal[bin];
    }
    avg /= bins_per_column;                                   // divide average over the range.
    int level = map(avg, 0, 100, 0, 16);                      // map the 0-100 range to a 0-max range, basically dividing the range.
    level = constrain(level, 0, 16);                          // constrain the max value to 16, so it doesn't go off our display.

    int i = col-1;                                            // set the indexer to the current column-1 since i'm lazy
    Serial.print(level);                                      // add the level for the col to the serial display
    bool flipPar = true;                                      // default to a flipped bar
    if (i%2) flipPar = false;                                 // if there is an even column, flip the bar to false
    Color bGrad = Color(0, 0+(16*i), 255-(16*i));             // compute a gradient based on the index value
    bGrad.setBrightness(5);                                   // set the color brightness 
    drawFrequencyBar(16*i, level, flipPar, bGrad);            // draw the bar
    Serial.print(" ");                                        // space between chars
  }
  display(); // update display
}
