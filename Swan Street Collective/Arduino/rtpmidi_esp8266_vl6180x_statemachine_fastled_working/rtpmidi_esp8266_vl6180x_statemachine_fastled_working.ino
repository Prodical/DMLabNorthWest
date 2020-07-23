#include <vector>
//#include <pnew.cpp>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiMulti.h>
#include "AppleMidi.h"
#include <Wire.h>
//#include "Adafruit_VL6180X.h"
#include <VL6180X.h> // alternative Pololu library
#include "Adafruit_DRV2605.h"
#include <ScaleManager.h>
#include <Easing.h>
//Andy Brown's Easing Library
//#include "EasingLibrary.h"
#include <ResponsiveAnalogRead.h>
// StopWatch library
//#include <StopWatch.h>
// Debounce buttons and switches, https://github.com/thomasfredericks/Bounce2/wiki
// Define the following here or in Bounce2.h. This make button change detection more responsive.
//#define BOUNCE_LOCK_OUT
#include <Bounce2.h>
#include <FastLED.h>
FASTLED_USING_NAMESPACE
// Basic demo for accelerometer readings from Adafruit LIS3DH
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// pin outs
#define DATA_PIN    14

ESP8266WiFiMulti wifiMulti;
const char SSID[] = "GL-AR300M-015-NOR";
const char PASS[] = "l3W1s4Rm";
//IPAddress remote(192, 168, 8, 142); // replace with remote ip
//TO DO - fix this in router admin - DONE

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h
bool AppleMIDIConnected = false;
bool dumpAppleMIDISessionFlag = false;
bool connectAppleMIDISessionFlag = true;

int noteNoBottom = 60;
//int noteRange = 25;
//int midiScale[15] = {60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83, 84}; // C Major scale 2 octaves
int noteRange = 13;
int midiScale[8] = {60, 62, 64, 65, 67, 69, 71, 72}; // C Major scale single octave
int scaleLEDs[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int midiNoteNo, midiNoteNoScale, midiNoteNoScaleLast = 0;
float freq, midiNoteNoFreq;
int pitchBend, pitchBendLast, pitchBendScaled, pitchBendAdjusted;
int irCount, oorCount = 0;

int scaleType = 1;

VL6180X tofSensor;
// To try different scaling factors, change the following define.
// Valid scaling factors are 1, 2, or 3.
#define SCALING 2
int range, rangeMaxCalibrate = 0;
int rangeMinCalibrate = 510;
int rangeMin = 10; // these values from calibration
int rangeMax = 370;
int topMargin = 10;
int lux; // TO DO - Pololu VL6180X library has a readAmbientSingle() funcion - implement and test

//Adafruit_VL6180X vl = Adafruit_VL6180X();
//float lux;
//uint8_t range;
//uint8_t status;

Adafruit_DRV2605 drv;
uint8_t effect = 1;

/// Adafruit LIS3DH
// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
float xAccel, yAccel, zAccel = 0.0;
// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define CLICKTHRESHHOLD 40

// FastLED library

#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    24
CRGB leds[NUM_LEDS];
#define BRIGHTNESS          32
#define FRAMES_PER_SECOND  120
int ledsRange = 0;

#define BUTTON_PIN 0
#define THUMB_BUTTON_PIN 12
int TBState = 0;
// Instantiate a Bounce object
Bounce debouncerGPIO = Bounce();
Bounce debouncerThumb = Bounce();

ScaleManager sm(true);  //use 'true' to load default scales: Chromatic, Major, Minor, Pentatonic
int fundamental = 60;

// easing global variables + booleans
boolean easeOn = false;
int easeType = 0;
int easeDuration = 250;
float easePosition; // was double
float easeRange, easeRangeRaw, easeRangeSquared;
boolean easeFlipFlag = false;
boolean easeOnFlag, easeDoneFlag = false;
int dir;
int easeTriggerCount = 0;

////Andy Brown's Easing Library - can't get this to work - might try switching back after implementing state machine
//BackEase back;
//BounceEase bounce;
//CircularEase circular;
//CubicEase cubic;
//ElasticEase elastic;
//ExponentialEase exponential;
//LinearEase linear;
//QuadraticEase quadratic;
//QuarticEase quartic;
//QuinticEase quintic;
//SineEase sine;

ResponsiveAnalogRead rangeRAR(0, true);
ResponsiveAnalogRead XRAR(0, true);
ResponsiveAnalogRead YRAR(0, true);
ResponsiveAnalogRead ZRAR(0, true);

// simple timer for printing debugging info to Serial Monitor
unsigned long currentTime;
unsigned long lastTime;
unsigned long delayTime = 100;
// and for flashing scale LEDs
//unsigned long currentTimeLEDs;
//unsigned long lastTimeLEDs;
//unsigned long delayTimeLEDs = 100;
//int scaleLEDsCount = 0;

std::vector<int> midiNoteNoVec;
std::vector<int>::const_iterator it;
int midiNoteNoScale4Vec, midiNoteNoScale4VecLast = 0;

// to delete ?
//int outOfRangeFlag = 0;
// StopWatch library for easing + scheduler
//StopWatch easeSW;
//int repeatMode = 0;
//float durMax = 30000;
//float easeDurationF;


//--------------------------------------------------------------
void initToFSensor()
{
  Wire.begin();
  delay(100);
  Wire.setClock(400000);
  delay(100);
  tofSensor.setAddress(0x29);
  tofSensor.init();
  delay(100);
  tofSensor.stopContinuous();
  tofSensor.configureDefault();
  tofSensor.setScaling(SCALING);
  //tofSensor.setTimeout(500);

}

//--------------------------------------------------------------
void drv2605(uint8_t _effect)
{

  effect = _effect;

  // set the effect to play
  drv.setWaveform(0, effect);  // play effect
  drv.setWaveform(1, 0);       // end waveform
  // play the effect!
  drv.go();

}

//--------------------------------------------------------------
void setup()
{

  // try doing the Wire.h stuff first
  // initialise ToF sensor
  initToFSensor();
  // initialise DRV2605 haptic controller
  drv.begin();
  drv.selectLibrary(1);
  // I2C trigger by sending 'go' command
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);
  effect = 10;
  drv2605(effect);
  //drv2605(effect);



  // initialise Serial
  Serial.begin(115200);
  // wait for serial port to open on native usb devices
  while (!Serial) {
    delay(1);
  }
  Serial.println(F("MIDI over Wifi setup"));

  pinMode(DATA_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(THUMB_BUTTON_PIN, INPUT);
  // After setting up the button, setup the Bounce instance :
  debouncerGPIO.attach(BUTTON_PIN);
  debouncerGPIO.interval(5); // interval in ms
  debouncerThumb.attach(THUMB_BUTTON_PIN);
  debouncerThumb.interval(5); // interval in ms

  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("LIS3DH found!");
  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!
  Serial.print("Range = "); Serial.print(2 << lis.getRange());
  Serial.println("G");
  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive
  lis.setClick(2, CLICKTHRESHHOLD);
  delay(100);

  wifiMulti.addAP(SSID, PASS);

  //  Serial.println("Adafruit VL6180x test!");
  //  if (! vl.begin()) {
  //    Serial.println("Failed to find sensor");
  //    while (1);
  //  }
  //  Serial.println("Sensor found!");

  //sm.setFundamental(60);
  //sm.setCurrentScale(MAJOR);
  setupScale(60, 1);

  // use ScaleManager library to set a 2 octave scale
  //  for (int i = 0; i < 8; i++) {
  //    midiScale[i] = sm.getScaleNote(i);
  //  }
  //  for (int i = 8; i < 16; i++) {
  //    midiScale[i] = sm.getScaleNote(i-8) + 12;
  //  }
  // debugging
  //  for (int i = 0; i < 16; i++) {
  //    Serial.print("midiNoteNoScale[");
  //    Serial.print(i);
  //    Serial.print("]: ");
  //    Serial.println("midiNoteNoScale[i]");
  //  }
  //
  // TO DO - use scale manager to update scale + also scaleLED array

  // initilaise ResponsiveAnalogRead
  rangeRAR.setActivityThreshold(9.0);
  rangeRAR.setSnapMultiplier(0.01);
  rangeRAR.setAnalogResolution(rangeMax);
  rangeRAR.disableSleep();

  XRAR.setActivityThreshold(0.5);
  XRAR.setSnapMultiplier(0.1);
  XRAR.setAnalogResolution(20.0);
  XRAR.disableSleep();
  YRAR.setActivityThreshold(0.5);
  YRAR.setSnapMultiplier(0.1);
  YRAR.setAnalogResolution(10.0);
  YRAR.disableSleep();
  ZRAR.setActivityThreshold(0.5);
  ZRAR.setSnapMultiplier(0.1);
  ZRAR.setAnalogResolution(10.0);
  ZRAR.disableSleep();

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  delay(1000);

  // start up LED test
  for ( int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::White;
  }
  FastLED.show();
  delay(2000);
  for ( int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(1000);

//  // set the LEDs that show notes in the scale from the midiScale array
//  //for (int i = 0; i < 14; i++) { // only 24 LEDs so drop top octave from scale
//  for (int i = 0; i < 8; i++) { // one octave scale
//    int noteRange = midiScale[i] - noteNoBottom;
//    int noteRangeMapped = map(noteRange, 0, 13, 0, 24);
//    scaleLEDs[noteRangeMapped] = 1;
//  }
  //  // debugging
  //    for (int i = 0; i < 24; i++) {
  //      Serial.print("scaleLEDs[");
  //      Serial.print(i);
  //      Serial.print("]: ");
  //      Serial.println(scaleLEDs[i]);
  //    }
  //  // turn on LEDs for scale
  //  for (int i = 0; i < NUM_LEDS; i++)
  //  {
  //    if (scaleLEDs[i] == 1)
  //    {
  //      leds[i] = CRGB::Red;
  //    }
  //    else
  //    {
  //      leds[i] = CRGB::Black;
  //    }
  //  }
  //  FastLED.show();



  // initialise midiNoteNoVec
  midiNoteNoVec.reserve(2);
  midiNoteNoVec.push_back(0);
  midiNoteNoVec.push_back(0);

}


//////////////////////////////////////////////
/// state machine implementation
//////////////////////////////////////////////

enum State_enum {CONNECT_WIFI, INIT_SENSOR, SET_MIDI, TRIGGER_MIDI, EASE_MIDI, PANIC_MIDI};
enum SensorState_enum {IN_RANGE, OUT_OF_RANGE};
enum GPI0ButtonState_enum {B_ON, B_OFF};
enum ThumbButtonState_enum {TB_ON, TB_OFF};

void state_machine_run(uint8_t tofSensorStateReturn, uint8_t ThumbButtonStateReturn, uint8_t GPI0ButtonStateReturn);
void connectWifiMidi();
void initToFSensor();
void setMidi();
void fastLED();
void drv2605(uint8_t _effect);
void lis3gh();
void easeMidi();
void triggerMidi();
void panicMidi();
uint8_t tofSensorState();
uint8_t GPI0ButtonState();
uint8_t ThumbButtonState();
uint8_t state = CONNECT_WIFI; //SET_MIDI;
bool wifiMidiConnected = false;

//--------------------------------------------------------------
void state_machine_run(uint8_t tofSensorStateReturn, uint8_t ThumbButtonStateReturn, uint8_t GPI0ButtonStateReturn)
{
  switch (state)
  {
    case CONNECT_WIFI:
      connectWifiMidi();
      if (wifiMidiConnected == true) {
        state = SET_MIDI;
      }
      break;
    case INIT_SENSOR:
      initToFSensor();
      state = SET_MIDI;
      break;
    case SET_MIDI:
      if (GPI0ButtonStateReturn == B_ON) {
        state = PANIC_MIDI;
      }
      //      if (ThumbButtonStateReturn == TB_ON) {
      //        //Serial.println("ThumbButtonState: TB_ON");
      //      }
      //      else
      //      {
      //        //Serial.println("ThumbButtonState: TB_OFF");
      //      }
      lis3gh();
      if (tofSensorStateReturn == IN_RANGE) {
        setMidi();
        fastLED();
        //if (midiNoteNoVec.at(1 != midiNoteNoScaleLast) {
        // debugging
        //          Serial.print("midiNoteNoScale: ");
        //          Serial.println(midiNoteNoScale);
        //          Serial.print("midiNoteNoScaleLast: ");
        //          Serial.println(midiNoteNoScaleLast);
        if (irCount == 0)
        {
          //AppleMIDI.sendNoteOff(midiNoteNoVec.at(0), 64, 1); // turns the last MIDI note triggered off
          oorCount = 0; // reset count ready for next sensor OUT_OF_RANGE
          irCount = 1;
        }
        if (midiNoteNoVec.at(1) != midiNoteNoScaleLast)
        {
          if (TBState == 0)
          {
            AppleMIDI.sendNoteOff(midiNoteNoVec.at(0), 64, 1); // turns the last MIDI note triggered off
            state = TRIGGER_MIDI;
          }
          else
          {
          }
        }
        midiNoteNoScaleLast = midiNoteNoVec.at(1);

      } else if (tofSensorStateReturn == OUT_OF_RANGE) {
        if (oorCount == 0) {
          panicMidi();
          //AppleMIDI.sendNoteOff(midiNoteNoVec.at(0), 64, 1); // turns the last MIDI note triggered off
          //midiNoteNoScaleLast = 0; // reset this to retrigger when back IN_RANGE
          oorCount = 1; // to only do this once
        }
      }
      break;
    case EASE_MIDI:
      easeMidi();
      irCount = 0;
      state = SET_MIDI;
      break;
    case TRIGGER_MIDI:
      easeTriggerCount = 0;
      triggerMidi();
      if (easeOn == true) {
        state = EASE_MIDI;
      } else {
        irCount = 0;
        state = SET_MIDI;
      }
      break;
    case PANIC_MIDI:
      panicMidi();
      state = SET_MIDI;
      break;
  }
}

//--------------------------------------------------------------
void connectWifiMidi()
{
  static bool connected = false;

  if (wifiMulti.run() == WL_CONNECTED) {
    if (!connected) {
      Serial.println(F("WiFi connected!"));
      AppleMIDI_setup();
      wifiMidiConnected = true;
      connected = true;
    }
  }
  else {
    if (connected) {
      Serial.println(F("WiFi not connected!"));
      connected = false;
    }
    delay(500);
  }
}

//--------------------------------------------------------------
void fastLED()
{

  //  // turn on LEDs for scale
  //  for (int i = 0; i < 14; i++) {
  //    leds[midiScaleLEDs[i]] = CRGB::Red;
  //  }
  //  FastLED.show();

  ledsRange = map(rangeRAR.getValue(), rangeMin, rangeMax, 0, 24); // rangeMax - topMargin

  for (int i = 0; i < ledsRange; i++)
  {
    if (scaleLEDs[i] == 0)
    {
      leds[i] = CRGB::Green;
    }
    else
    {
      leds[i] = CRGB::Blue;
    }
  }
  for (int i = ledsRange; i < NUM_LEDS; i++)
  {
    if (scaleLEDs[i] == 0)
    {
      leds[i] = CRGB::Black;
    }
    else
    {
      leds[i] = CRGB::Red;
    }

  }
  FastLED.show();

}

//--------------------------------------------------------------
void setMidi()
{
  // update the ResponsiveAnalogRead object
  rangeRAR.update(range);

//  Serial.print("range: ");
//  Serial.println(range);
  Serial.print("rangeRAR: ");
  Serial.println(rangeRAR.getValue());

  // use the ResponsiveAnalogRead object to set the MIDI note
  midiNoteNo = map(rangeRAR.getValue(), rangeMin, rangeMax, fundamental, fundamental + 13); //noteRange); // rangeMax - topMargin
  Serial.print("midiNoteNo: ");
  Serial.println(midiNoteNo);

  //for (int i = 0; i < 15; i++) {
  for (int i = 0; i < 8; i++) {
    if (midiScale[i] == midiNoteNo) {
      midiNoteNoScale = midiNoteNo;
      midiNoteNoScale4Vec = midiNoteNoScale;
      break;
    }
  }

  if (midiNoteNoScale4Vec != midiNoteNoScale4VecLast)
  {
    midiNoteNoVec.push_back(midiNoteNoScale);
    if (midiNoteNoVec.size() > 2)
    {
      midiNoteNoVec.erase(midiNoteNoVec.begin());
    }
    //debugging
    //    Serial.print("midiNoteNoVec.size(): ");
    //    Serial.println(midiNoteNoVec.size());
    //    Serial.print("midiNoteNoVec[0]: ");
    //    Serial.println(midiNoteNoVec[0]);
    //    Serial.print("midiNoteNoVec[1]: ");
    //    Serial.println(midiNoteNoVec[1]);
  }
  midiNoteNoScale4VecLast = midiNoteNoScale4Vec;

}

//--------------------------------------------------------------
void easeMidi()
{
  // additonal variables for elastic easing
  float amplitude = 0.5;
  float period = 100.0;
  // and back
  float overshoot = 1.70158;

  easeRange = float(midiNoteNoVec.at(1) - midiNoteNoVec.at(0));
  //Serial.print(F("easeRange: "));
  //Serial.println(easeRange);

  if (easeRange < 0) {
    dir = -1;
  } else {
    dir = 1;
  }

  //  // determine the ease direction + for an increase and - for a decrease in note value
  //  easeRangeRaw = float(midiNoteNoScale - midiNoteNoScaleLast);
  //    if (easeRangeRaw < 0) {
  //      dir = -1;
  //    } else {
  //      dir = 1;
  //    }
  //  easeRangeSquared = easeRangeRaw * easeRangeRaw;
  //  easeRange = sqrt(easeRangeSquared);





  //float midiNoteNoScaleF = (float)midiNoteNoScale;

  for (int t = 0; t < easeDuration; t++) {

    switch (easeType) {
      case 1:
        easePosition = Easing::linearTween(float(t), float(midiNoteNoVec.at(1)), easeRange, float(easeDuration));
        break;
      case 2:
        easePosition = Easing::easeInOutQuad(float(t), float(midiNoteNoVec.at(1)), easeRange, float(easeDuration));
        break;
      case 3:
        easePosition = Easing::easeInOutCubic(float(t), float(midiNoteNoVec.at(1)), easeRange, float(easeDuration));
        break;
      case 4:
        easePosition = Easing::easeInOutQuart(float(t), float(midiNoteNoVec.at(1)), easeRange, float(easeDuration));
        break;
      case 5:
        easePosition = Easing::easeInOutQuint(float(t), float(midiNoteNoVec.at(1)), easeRange, float(easeDuration));
        break;
      case 6:
        easePosition = Easing::easeInOutSine(float(t), float(midiNoteNoVec.at(1)), easeRange, float(easeDuration));
        break;
      case 7:
        easePosition = Easing::easeInOutExpo(float(t), float(midiNoteNoVec.at(1)), easeRange, float(easeDuration));
        break;
      case 8:
        easePosition = Easing::easeInOutCirc(float(t), float(midiNoteNoVec.at(1)), easeRange, float(easeDuration));
        break;
      case 9:
        easePosition = Easing::easeInOutElastic(float(t), float(midiNoteNoVec.at(1)), easeRange, float(easeDuration), amplitude, period);
        break;
      case 10:
        easePosition = Easing::easeInOutBack(float(t), float(midiNoteNoVec.at(1)), easeRange, float(easeDuration), overshoot);
        break;
      case 11:
        easePosition = Easing::easeInOutBounce(float(t), float(midiNoteNoVec.at(1)), easeRange, float(easeDuration));
        break;

        Serial.print("easePosition: ");
        Serial.println(easePosition);

      // TO DO - add updates to amplitude and period variables via oF ofxDatGui interface
      // TO DO - add options for ease in or out only ??
      // TO DO - integrate hand drawn ease curve data
      default:
        break;
    }
    //    Serial.print(F("midiNoteNoScaleLast: "));
    //    Serial.println(midiNoteNoScaleLast);
    //        Serial.print(F("easePosition: "));
    //        Serial.println(easePosition);
    //    if (easePosition == midiNoteNoScaleF) {
    //      Serial.println(F("ease done"));
    //    }

    freq = sm.getFrequency(midiNoteNoVec.at(1));
    //        Serial.print(F("frq: "));
    //        Serial.println(freq);
    midiNoteNoFreq = 440.0 * pow(2, ((easePosition) - 69.0) / 12.0);
    //        Serial.print(F("midiNoteNoFreq: "));
    //        Serial.println(midiNoteNoFreq);
    pitchBend = round(4096 * 12.0 * log(freq / midiNoteNoFreq) / log(2));
    pitchBend = sqrt(pitchBend * pitchBend) * dir;
    //Serial.print(F("pitchBend: "));
    //Serial.println(pitchBend);
    pitchBendAdjusted = -((4096 * sqrt(easeRange * easeRange)) * dir - pitchBend);
    //Serial.print(F("pitchBendAdjusted: "));
    //Serial.println(pitchBendAdjusted);

    //    if (easeRange == 1.0) {
    //      pitchBendScaled = int(pitchBend / 2) + 8192;
    //    } else if (easeRange == 2.0) {
    //      pitchBendScaled = pitchBend + 8192;
    //    }

    //AppleMIDI.sendPitchBend(pitchBendScaled, 1);

    AppleMIDI.sendPitchBend(pitchBendAdjusted, 1);
    //send the note on after the first pitch bend message - but only trigger it once
    if (easeTriggerCount == 0)
    {
      AppleMIDI.sendNoteOn(midiNoteNoVec.at(1), 127, 1);
      easeTriggerCount = 1;
    }

    //    if (int(easePosition) == midiNoteNoScale) {
    //      easeDoneFlag = true;
    //      //Serial.println(F("ease done"));
    //    } else {
    //      //Serial.println(F("ease working"));
    //    }

    yield();

  }
  //midiNoteNoScaleLast = midiNoteNoScale;
}

//--------------------------------------------------------------
void triggerMidi()
{
  //Serial.print("midiNoteNoVec.at(1): ");
  //Serial.println(midiNoteNoVec.at(1));
  //if (count2 == 0) {
  //    AppleMIDI.sendNoteOff(midiNoteNoScaleLast, 64, 1);
  if (easeOn == false)
  {
    AppleMIDI.sendNoteOn(midiNoteNoVec.at(1), 127, 1);
    effect = 10;
    drv2605(effect);
    //drv2605(effect);
    //delay(500);
  } else
  {
  }
  //    count2 = 1;
  //  }
}

//--------------------------------------------------------------
void panicMidi()
{
  // MIDI Panic button. Send All Notes Off on all channels.
  // Useful if a MIDI device has a note stuck on.
  for (uint8_t chan = 1; chan < 17; chan++) {
    // button pressed so send All Notes Off on all channels
    //AppleMIDI.controlChange(0x7b, 0x00, chan);
    AppleMIDI.controlChange(123, 0, chan);
    delay(2);
  }
}

//--------------------------------------------------------------
uint8_t tofSensorState()
{
  /// Adafruit VL6180X library
  //lux = vl.readLux(VL6180X_ALS_GAIN_5);
  //range = vl.readRange();

  /// Pololu VL6180X library
  range = tofSensor.readRangeSingleMillimeters();
  //  if (tofSensor.timeoutOccurred()) {
  //    Serial.print(" TIMEOUT");
  //  }

  // output the range to the Serial Monitor
  //Serial.print("range: ");
  //Serial.println(range);
  // detect min and max range values
  //  if (rangeMinCalibrate > range) {
  //    rangeMinCalibrate = range;
  //  }
  //  if ((rangeMaxCalibrate < range) && (range != 510)) {
  //    rangeMaxCalibrate = range;
  //  }
  //  Serial.print(F("rangeMinCalibrate: "));
  //  Serial.println(rangeMinCalibrate);
  //  Serial.print(F("rangeMaxCalibrate: "));
  //  Serial.println(rangeMaxCalibrate);

  if (range == 510) {
    return OUT_OF_RANGE;
  } else if ((range >= 10) && (range <= 378)) {
    return IN_RANGE;
  }
}

//--------------------------------------------------------------
uint8_t GPI0ButtonState()
{
  // Update the Bounce instance :
  debouncerGPIO.update();

  if (debouncerGPIO.fell()) {
    return B_ON;
    irCount = 0;
  } else {
    return B_OFF;
  }

}

//--------------------------------------------------------------
uint8_t ThumbButtonState()
{

  TBState = digitalRead(THUMB_BUTTON_PIN);
  //Serial.println("TBState: ");
  //Serial.println(TBState);

  debouncerThumb.update();

  if (debouncerThumb.rose()) {
    //Serial.println("ThumbButtonState: TB_ON");
    return TB_ON;
    //irCount = 0;
  } else {
    //Serial.println("ThumbButtonState: TB_OFF");
    return TB_OFF;
  }

}

//--------------------------------------------------------------
void lis3gh()
{

  lis.read();      // get X Y and Z data at once
  // Then print out the raw data
  //  Serial.print("X:  "); Serial.print(lis.x);
  //  Serial.print("  \tY:  "); Serial.print(lis.y);
  //  Serial.print("  \tZ:  "); Serial.print(lis.z);

  /* Or....get a new sensor event, normalized */
  sensors_event_t event;
  lis.getEvent(&event);

  //  /* Display the results (acceleration is measured in m/s^2) */
  //  Serial.print("\tX: "); Serial.print(event.acceleration.x);
  //  Serial.print(" \tY: "); Serial.print(event.acceleration.y);
  //  Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
  //  Serial.println(" m/s^2 ");
  //  Serial.println();

  // update the ResponsiveAnalogRead object
  XRAR.update(event.acceleration.x);
  YRAR.update(event.acceleration.y);
  ZRAR.update(event.acceleration.z);

  xAccel = XRAR.getValue();
  yAccel = YRAR.getValue();
  zAccel = ZRAR.getValue();

  //  Serial.print("\txAccel: "); Serial.print(xAccel);
  //  Serial.print(" \tyAccel: "); Serial.print(yAccel);
  //  Serial.print(" \tzAccel: "); Serial.print(zAccel);
  //  Serial.println(" m/s^2 ");
  //  Serial.println();

  if (yAccel > 2.0)
  {
    AppleMIDI.controlChange(1, map(int(yAccel), 0, 10, 0, 127), 1);
  }
  else
  {
  }

  uint8_t click = lis.getClick();
  if (click == 0) return;
  if (! (click & 0x30)) return;
  Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.print("): ");
  if (click & 0x10) Serial.print(" single click");
  if (click & 0x20) Serial.print(" double click");
  Serial.println();
  //delay(100);
  return;

}


//--------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  AppleMIDI.run();

  FastLED.setBrightness(BRIGHTNESS);

  // flashing scale LEDs - doesn't really work
  //  currentTimeLEDs = millis();
  //  if (currentTimeLEDs - lastTimeLEDs > delayTimeLEDs)
  //  {
  //    if (scaleLEDsCount == 0)
  //    {
  //      // turn on LEDs for scale
  //      for (int i = 0; i < 14; i++) {
  //        leds[midiScaleLEDs[i]] = CRGB::Red;
  //      }
  //      FastLED.show();
  //    }
  //    else if (scaleLEDsCount == 1)
  //    {
  //      FastLED.clear();
  //      FastLED.show();
  //    }
  //    scaleLEDsCount++;
  //    if (scaleLEDsCount > 1)
  //    {
  //      scaleLEDsCount = 0;
  //    }
  //    lastTimeLEDs = currentTimeLEDs;
  //  }

  state_machine_run(tofSensorState(), ThumbButtonState(), GPI0ButtonState());

  // TO DO - sort and delete code below

  //  if (range == 510) {
  //    //midiNoteNo = 0;
  //    if (outOfRangeFlag == 0) {
  //      AppleMIDI.sendNoteOff(midiNoteNoScaleLast, 64, 1);
  //      outOfRangeFlag = 1;
  //    }
  //  } else {
  //    outOfRangeFlag = 0;
  //    midiNoteNo = map(range, 12, 378, noteNoBottom, noteNoBottom + noteRange + 1);
  //    //        Serial.print("midiNoteNo: ");
  //    //        Serial.println(midiNoteNo);
  //  }
  //
  //  for (int i = 0; i < 14; i++) {
  //    if (midiScale[i] == midiNoteNo) {
  //      midiNoteNoScale = midiNoteNo;
  //      //      Serial.print("midiNoteNoScale: ");
  //      //      Serial.println(midiNoteNoScale);
  //      break;
  //    }
  //  }
  //
  //  if (midiNoteNoScale != midiNoteNoScaleLast) {
  //    //    easeSW.reset();
  //    //    easeSW.start();
  //    //    count = 0;
  //    easeOnFlag = true;
  //    AppleMIDI.sendNoteOff(midiNoteNoScaleLast, 64, 1);
  //    AppleMIDI.sendNoteOn(midiNoteNoScale, 127, 1);
  //  } else {
  //    easeOnFlag = false;
  //    //    easeSW.stop();
  //  }
  //
  //
  //
  //
  //  //  if (easeOnFlag == true) {
  //  //    //ABEasing();
  //  //  } else {
  //  //    easeSW.stop();
  //  //  }
  //
  //  //      if (easeOnFlag == true) {
  //  //        ABEasing();
  //  //      }
  //
  //  if (easeOnFlag == true) {
  //    //ABEasing();
  //    EasingFunction();
  //
  //    //    yield();
  //  }
  //
  //  if (easeDoneFlag == true) {
  //    easeOnFlag = false;
  //
  //
  //  }
  //
  //  //  if (easePosition == float(midiNoteNoScale)) {
  //  //    Serial.println(F("ease done"));
  //  //  }
  //
  //  //    do {
  //  //      ABEasing();
  //  //      yield();
  //  //    } while (easeOnFlag == true); //(easePosition < easeRange);
  //
  //  //midiNoteNoFreq = 440.0 * pow(2, ((midiNoteNoScaleLast + easePosition) - 69.0) / 12.0);
  //  //freq = sm.getFrequency(midiNoteNoScaleLast);
  //  //pitchBend = round(4096 * 12.0 * log(freq / midiNoteNoFreq) / log(2));
  //
  //
  //  currentTime = millis();
  //  if (currentTime - lastTime > delayTime) {
  //    //    Serial.print(F("easeOnFlag: "));
  //    //    Serial.println(easeOnFlag);
  //    //        Serial.print(F("freq: "));
  //    //        Serial.println(freq);
  //    //    Serial.print(F("midiNoteNoFreq: "));
  //    //    Serial.println(midiNoteNoFreq);
  //    //    Serial.print(F("pitchBend: "));
  //    //    Serial.println(pitchBend);
  //    //        Serial.print("easeSW.elapsed(): ");
  //    //        Serial.println(easeSW.elapsed());
  //    //        Serial.print("easePosition: ");
  //    //        Serial.println(easePosition);
  //    lastTime = currentTime;
  //  }
  //
  //
  //
  //  //  if (easeOnFlag == true) {
  //  //    ABEasing();
  //  //  if (easePosition < easeRange) {
  //  //    easeOnFlag = false;
  //  //  }
  //  //  }
  //
  //  //  Serial.print("easePosition: ");
  //  //  Serial.println(easePosition);
  //
  //  //  //Serial.print("easeSW.elapsed(): ");
  //  //  //Serial.println(easeSW.elapsed());
  //  //  //  Serial.print("easePosition: ");
  //  //  //  Serial.println(easePosition);
  //  //
  //  //  if (easeOnFlag == true) {
  //  //
  //  //  // turn off the last note
  //  //
  //  //
  //
  //
  //  //    //      Serial.print("freq: ");
  //  //    //      Serial.println(freq);
  //  //    do {
  //  //      ABEasing();
  //  //      AppleMIDI.sendPitchBend(pitchBend, 1);
  //  //    } while (easePosition < easeRange);
  //  //
  //  //    AppleMIDI.sendNoteOff(midiNoteNoScaleLast, 64, 1);
  //  //    AppleMIDI.sendNoteOn(midiNoteNoScale, 127, 1);
  //  //
  //  //    easeOnFlag = false;
  //  //  }
  //
  //  midiNoteNoScaleLast = midiNoteNoScale;

}

//--------------------------------------------------------------
void setupScale(int _fundamental, int _scaleType)
{

  fundamental = _fundamental;
  scaleType = _scaleType;

  sm.setFundamental(fundamental);
    
    if (scaleType == 0)
    {
        sm.setCurrentScale(CHROMATIC);
    }
    else if (scaleType == 1)
    {
        sm.setCurrentScale(MAJOR);
    }
    else if (scaleType == 2)
    {
        sm.setCurrentScale(MINOR);
    }
    else if (scaleType == 3)
    {
        sm.setCurrentScale(PENTATONIC);
    }
    
    // use ScaleManager library to set notes for the virtual strings
    for (int i = 0; i < 9; i++) {
        midiScale[i] = sm.getScaleNote(i);
    }
    // debugging
    //      for (int i = 0; i < 8; i++) {
    //          cout << "midiScale[" << i << "]: " << midiScale[i] << endl;
    //      }

    // set the LEDs that show notes in the scale from the midiScale array
  //for (int i = 0; i < 14; i++) { // only 24 LEDs so drop top octave from scale
  for (int i = 0; i < 8; i++) { // one octave scale
    int noteRangeI = midiScale[i] - fundamental; //noteNoBottom;
    int noteRangeIMapped = map(noteRangeI, 0, 13, 0, 24);
    scaleLEDs[noteRangeIMapped] = 1;
  }

}


//--------------------------------------------------------------
/// Apple MIDI setup
void AppleMIDI_setup()
{
  if (connectAppleMIDISessionFlag == true)
  {
    Serial.println(F("Apple MIDI setup"));

    Serial.println();
    Serial.print(F("IP address: "));
    Serial.print(WiFi.localIP());
    Serial.print(F(", Port: "));
    Serial.println(F("5004"));

    Serial.println(F("Enable a MIDI Network session on Mac and connect to Adafruit Huzzah"));
    // Create a session and wait for a remote host to connect to us
    AppleMIDI.stop();

    Serial.print(AppleMIDI.getSessionName());
    Serial.print(" with SSRC 0x");
    Serial.println(AppleMIDI.getSynchronizationSource(), HEX);


    AppleMIDI.begin("Adafruit Huzzah");

    // TO DO - look at  examples and study library code to automate session connect & disconnect at end
    // use a die() function ?? - http://forum.arduino.cc/index.php?topic=58956.msg424464#msg424464

    // This is the invite to the remote participant
    AppleMIDI.invite(remote, 5004);

    AppleMIDI.OnConnected(OnAppleMidiConnected);
    AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

    AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
    AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);

    connectAppleMIDISessionFlag = false;
  }
}

void AppleMidiDumpSession()
{
  if (dumpAppleMIDISessionFlag == true)
  {
    //AppleMIDI.DumpSession();
    //AppleMIDI.DeleteSession(AppleMIDI.getSynchronizationSource());
    dumpAppleMIDISessionFlag = false;
  }
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, char* name)
{
  AppleMIDIConnected = true;
  Serial.print(F("Apple MIDI connected to session on "));
  Serial.println(name);

  // flash LEDs green
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
  delay(1000);
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(1000);

}


// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc)
{
  AppleMIDIConnected = false;
  Serial.println(F("Apple MIDI disconnected"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOn(byte channel, byte note, byte velocity)
{
  //  Serial.print(F("Incoming NoteOn from channel:"));
  //  Serial.print(channel);
  Serial.print(F("note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();

  if (note == 0) {
    state = INIT_SENSOR;
    Serial.println(F("re-initilaising sensor"));
  } else if (note == 1) {
    easeOn = true;
    Serial.print(F("easeOn: "));
    Serial.println(easeOn);
  } else if (note == 2) {
    easeDuration = int(float(velocity) * 1000 / 127);
    Serial.print(F("easeDuration: "));
    Serial.println(easeDuration);
  } else if (note == 3) {
    if (velocity == 10) {
      easeType = 1;
      Serial.println(F("Linear"));
    } else if (velocity == 20) {
      easeType = 2;
      Serial.println(F("Quadratic"));
    } else if (velocity == 30) {
      easeType = 3;
      Serial.println(F("Cubic"));
    } else if (velocity == 40) {
      easeType = 4;
      Serial.println(F("Quartic"));
    } else if (velocity == 50) {
      easeType = 5;
      Serial.println(F("Quintic"));
    } else if (velocity == 60) {
      easeType = 6;
      Serial.println(F("Sinusoidal"));
    } else if (velocity == 70) {
      easeType = 7;
      Serial.println(F("Exponential"));
    } else if (velocity == 80) {
      easeType = 8;
      Serial.println(F("Circular"));
    } else if (velocity == 90) {
      easeType = 9;
      Serial.println(F("Elastic"));
    } else if (velocity == 100) {
      easeType = 10;
      Serial.println(F("Back"));
    } else if (velocity == 110) {
      easeType = 11;
      Serial.println(F("Bounce"));
    }
  }
  else if (note == 4)
  {
    connectAppleMIDISessionFlag = true;
    Serial.println("running Apple MIDI setup");
    AppleMIDI_setup();
  }
  // scale type
  else if (note == 5)
  {
    if (velocity == 10) {
      scaleType = 0;
      //sm.setCurrentScale(CHROMATIC);
      Serial.println(F("Chromatic"));
      setupScale(fundamental, 0);
    }
    else if (velocity == 20)
    {
      scaleType = 1;
      //sm.setCurrentScale(MAJOR);
      Serial.println(F("Major"));
      setupScale(fundamental, 1);
    }
    else if (velocity == 30)
    {
      scaleType = 2;
      //sm.setCurrentScale(MINOR);
      Serial.println(F("Minor"));
      setupScale(fundamental, 2);
    }
    else if (velocity == 40)
    {
      scaleType = 3;
      //sm.setCurrentScale(PENTATONIC);
      Serial.println(F("Pentatonic"));
      setupScale(fundamental, 3);
    }

  }
  // fundamental
  else if (note == 6)
  {
    fundamental = velocity;
    //sm.setFundamental(fundamental);
    setupScale(fundamental, scaleType);
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOff(byte channel, byte note, byte velocity)
{
  //  Serial.print(F("Incoming NoteOff from channel:"));
  //  Serial.print(channel);
  Serial.print(F("note:"));
  Serial.println(note);
  //  Serial.print(F(" velocity:"));
  //  Serial.print(velocity);
  //  Serial.println();

  if (note == 1) {
    easeOn = false;
    Serial.print(F("easeOn: "));
    Serial.println(easeOn);
  }
  else if (note == 4)
  {
    dumpAppleMIDISessionFlag = true;
    Serial.println("dumping Apple MIDI session");
    AppleMidiDumpSession();
  }

}


/////////////////////////////////////////
///// ANDY BROWN's EASING LIBRARY
/////////////////////////////////////////
//
//void ABEasing() {
//
//  // set the easing  range
//  //  easeRange = float(midiNoteNoScale - midiNoteNoScaleLast);
//  //      Serial.print("easeRange: ");
//  //      Serial.println(easeRange);
//
//  easeRange = 3.0;
//
//  // set parameters for the various ease types
//  // 2 units, easeDuration we will interpret as milliseconds - some functions such as elastic have more
//  linear.setDuration(easeDuration);
//  linear.setTotalChangeInPosition(int(easeRange));
//
//  back.setDuration(easeDuration);
//  back.setTotalChangeInPosition(int(easeRange));
//  back.setOvershoot(1.5);
//
//  bounce.setDuration(easeDuration);
//  bounce.setTotalChangeInPosition(int(easeRange));
//
//  circular.setDuration(easeDuration);
//  circular.setTotalChangeInPosition(int(easeRange));
//
//  cubic.setDuration(easeDuration);
//  cubic.setTotalChangeInPosition(int(easeRange));
//
//  elastic.setDuration(easeDuration);
//  elastic.setTotalChangeInPosition(int(easeRange));
//  elastic.setPeriod(750);
//  elastic.setAmplitude(5);
//
//  exponential.setDuration(easeDuration);
//  exponential.setTotalChangeInPosition(int(easeRange));
//
//  quadratic.setDuration(easeDuration);
//  quadratic.setTotalChangeInPosition(int(easeRange));
//
//  quartic.setDuration(easeDuration);
//  quartic.setTotalChangeInPosition(int(easeRange));
//
//  quintic.setDuration(easeDuration);
//  quintic.setTotalChangeInPosition(int(easeRange));
//
//  sine.setDuration(easeDuration);
//  sine.setTotalChangeInPosition(int(easeRange));
//
//
//  if ((easeSW.elapsed() < easeDuration / 2.) && (count == 0)) { // empirically this / 2. and the easeSW.elapsed() * 2 below results in an ease time closer to the displayed seconds
//    // select the ease type
//    switch (easeType) {
//      case 1:
//        easePosition = back.easeInOut(easeSW.elapsed() * 2);
//        break;
//      case 2:
//        easePosition = bounce.easeInOut(easeSW.elapsed() * 2);
//        break;
//      case 3:
//        easePosition = circular.easeInOut(easeSW.elapsed() * 2);
//        break;
//      case 4:
//        easePosition = cubic.easeInOut(easeSW.elapsed() * 2);
//        break;
//      case 5:
//        easePosition = elastic.easeInOut(easeSW.elapsed() * 2);
//        break;
//      case 6:
//        easePosition = exponential.easeInOut(easeSW.elapsed() * 2);
//      case 7:
//        easePosition = quadratic.easeInOut(easeSW.elapsed() * 2);
//      case 8:
//        easePosition = quartic.easeInOut(easeSW.elapsed() * 2);
//      case 9:
//        easePosition = quintic.easeInOut(easeSW.elapsed() * 2);
//      case 10:
//        easePosition = sine.easeInOut(easeSW.elapsed() * 2);
//      case 11:
//        easePosition = linear.easeInOut(easeSW.elapsed() * 2);
//        break;
//      default:
//        break;
//    }
//
//    if (easeSW.elapsed() > 300) {
//      count = 1;
//    }
//
//    //    Serial.print("easeSW.elapsed(): ");
//    //    Serial.println(easeSW.elapsed());
//
//
//
//    //    if (easePosition == easeRange) {
//    //      easeOnFlag = false;
//    //      //easeSW.stop();
//    //    }
//
//    // for up, down and palindome repeats
//    if (easeFlipFlag == false) {
//      //waveform1.frequency(mappedFreq1 + (unsigned char)easePosition);
//    }
//    else if (easeFlipFlag == true) {
//      //waveform1.frequency(mappedFreq2 - (unsigned char)easePosition);
//    }
//
//  } else {
//    if (repeatMode == 2) {
//      easeFlipFlag = !easeFlipFlag;
//    } else if (repeatMode == 1) {
//      easeFlipFlag = true;
//    } else if (repeatMode == 0) {
//      easeFlipFlag = false;
//    }
//    easeSW.reset();
//    easeSW.start();
//  }
//
//  currentTime = millis();
//  if (currentTime - lastTime > delayTime) {
//    //Serial.println(F("working!"));
//    //      Serial.print("easeRange: ");
//    //      Serial.print(easeRange);
//    //    Serial.print(", easeSW.elapsed(): ");
//    //    Serial.print(easeSW.elapsed());
//    //      //  Serial.print("currentTime: ");
//    //      //  Serial.print(currentTime);
//    //            Serial.print(F(", easePosition: "));
//    //            Serial.println(easePosition);
//    lastTime = currentTime;
//  }
//
//
//  //  Serial.print(F("easePosition: "));
//  //  Serial.println(easePosition);
//  //  Serial.print(F("easeSW.elapsed(): "));
//  //  Serial.println(easeSW.elapsed());
//
//  if ((easePosition == easeRange) || (easeSW.elapsed() > 600)) {
//    count = 1;
//    easeSW.stop();
//    easeOnFlag = false;
//  }
//
//  //  if (easeSW.elapsed() > 250) {
//  //    count = 1;
//  //    easeSW.stop();
//  //    easeOnFlag = false;
//  //  }
//
//  //
//  ////    currentTime = millis();
//  ////    if (currentTime - lastTime > delayTime) {
//  //      freq = sm.getFrequency(midiNoteNoScaleLast);
//  //      midiNoteNoFreq = 440.0 * pow(2, ((midiNoteNoScaleLast + easePosition) - 69.0) / 12.0);
//  //      pitchBend = round(4096 * 12.0 * log(freq / midiNoteNoFreq) / log(2));
//  ////      lastTime = currentTime;
//  ////    }
//  //
//  //    if (easePosition == easeRange) {
//  //      easeOnFlag = false;
//  //      //easeSW.stop();
//  //    }
//  //
//  //    // for up, down and palindome repeats
//  //    if (easeFlipFlag == false) {
//  //      //waveform1.frequency(mappedFreq1 + (unsigned char)easePosition);
//  //      //
//  //
//  //
//  //
//  //
//  //      //      currentTime = millis();
//  //      //      if (currentTime - lastTime > delayTime) {
//  //      //        Serial.print("pitchBend: ");
//  //      //        Serial.println(pitchBend);
//  //      //        lastTime = currentTime;
//  //      //      }
//  //
//  //    }
//  //    else if (easeFlipFlag == true) {
//  //      //waveform1.frequency(mappedFreq2 - (unsigned char)easePosition);
//  //    }
//  //
//  //  }
//  //  //  else {
//  //  //    if (repeatMode == 2) {
//  //  //      easeFlipFlag = !easeFlipFlag;
//  //  //    } else if (repeatMode == 1) {
//  //  //      easeFlipFlag = true;
//  //  //    } else if (repeatMode == 0) {
//  //  //      easeFlipFlag = false;
//  //  //    }
//  //  //    easeSW.reset();
//  //  //    easeSW.start();
//  //  //  }
//
//}
