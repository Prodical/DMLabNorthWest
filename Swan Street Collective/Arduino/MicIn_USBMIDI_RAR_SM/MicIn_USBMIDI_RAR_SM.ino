// Advanced Microcontroller-based Audio Workshop
//
// http://www.pjrc.com/store/audio_tutorial_kit.html
// https://hackaday.io/project/8292-microcontroller-audio-workshop-had-supercon-2015
//
// Part 2-4: Using The Microphone


///////////////////////////////////
// copy the Design Tool code here
///////////////////////////////////

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <ScaleManager.h>
#include <MIDI.h>

#if defined(USBCON)
#include <midi_UsbTransport.h>
static const unsigned sUsbTransportBufferSize = 16;
typedef midi::UsbTransport<sUsbTransportBufferSize> UsbTransport;
UsbTransport sUsbTransport;
MIDI_CREATE_INSTANCE(UsbTransport, sUsbTransport, MIDI);
#else // No USB available, fallback to Serial
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

ScaleManager sm(true);  //use 'true' to load default scales: Chromatic, Major, Minor, Pentatonic

#include <ResponsiveAnalogRead.h>
ResponsiveAnalogRead freqRAR(0, true);
ResponsiveAnalogRead leftRMSRAR(0, false);

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=198,173
AudioMixer4              mixer1;         //xy=431,278
AudioAnalyzeRMS          rmsL;           //xy=731,154
AudioAnalyzeNoteFrequency noteFreq;      //xy=731,220
AudioOutputI2S           i2s2;           //xy=754,418
AudioConnection          patchCord1(i2s1, 0, mixer1, 0);
//AudioConnection          patchCord2(i2s1, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, noteFreq);
AudioConnection          patchCord4(mixer1, 0, i2s2, 0);
AudioConnection          patchCord5(mixer1, 0, i2s2, 1);
AudioConnection          patchCord6(mixer1, rmsL);
AudioControlSGTL5000     audioShield;     //xy=203,278
// GUItool: end automatically generated code

float freq, prob, freq95, freqRaw, freqSM;
int midiNoteNo, midiNoteNoLast;
float midiNoteNoF, midiNoteNoFRaw;
float midiNoteNoPitch;
int pitchBend, pitchBendLast;
float pitchBendDec;
int velocity = 127;

bool pitchBendOn = true;

uint8_t leftRMS = 0;
uint8_t leftRMSMin = 10;
uint8_t leftRMSMax = 0;
int endThreshold = 7;
int startThreshold = 15;

int setMidiCount, trigMidiCount = 0;

// simple timer
long currentTime, lastTime;
int delayTime = 250;


void setup() {

  Serial.begin(230400);

  AudioMemory(100);
  audioShield.enable();
  audioShield.volume(0.0);
  audioShield.inputSelect(AUDIO_INPUT_MIC);
  audioShield.micGain(16); //16
  audioShield.unmuteLineout();
  delay(1000);

  noteFreq.begin(.50);

  MIDI.begin();

  freqRAR.setActivityThreshold(10.0);
  freqRAR.setSnapMultiplier(1.0);
  freqRAR.setAnalogResolution(750);
  freqRAR.disableSleep();
  //freqRAR.enableSleep();
  leftRMSRAR.setActivityThreshold(5.0);
  leftRMSRAR.setSnapMultiplier(0.01);
  leftRMSRAR.setAnalogResolution(255);
  leftRMSRAR.disableSleep();
  //leftRMSRAR.enableSleep();

  sm.setFundamental(36);
  sm.setCurrentScale(MAJOR);

}

//////////////////////////////////////////////
/// state machine implementation
//////////////////////////////////////////////

enum State_enum {SET_MIDI, TRIGGER_MIDI, PANIC_MIDI};
enum AANFState_enum {OUT_PROBABILITY, IN_PROBABILITY};
enum AARMSState_enum {START_OUT_THRESHOLD, START_IN_THRESHOLD, END_OUT_THRESHOLD, END_IN_THRESHOLD};
enum ButtonState_enum {B_OFF, B_ON};

void state_machine_run(uint8_t AANFStateReturn, uint8_t AARMSStateReturn); //, uint8_t GPI0ButtonStateReturn
void setMidi();
void setPitchBend();
void triggerMidi();
void panicMidi();
uint8_t AANFState();
uint8_t AARMSState();
//uint8_t GPI0ButtonState();
uint8_t state = SET_MIDI;

//--------------------------------------------------------------
void state_machine_run(uint8_t AANFStateReturn, uint8_t AARMSStateReturn) //, uint8_t GPI0ButtonStateReturn
{

  switch (state)
  {
    case SET_MIDI:

      //      if (GPI0ButtonStateReturn == B_ON) {
      //        state = PANIC_MIDI;
      //      }

      if (pitchBendOn == true)
      {
        setPitchBend();
      }
      else
      {
      }

      if (AANFStateReturn == IN_PROBABILITY)
      {
        //Serial.println("IN_PROBABILITY");
        setMidi();
        if (midiNoteNo != midiNoteNoLast)
        {
          // debugging
          //          Serial.print("midiNoteNo: ");
          //          Serial.println(midiNoteNo);
          //          Serial.print("midiNoteNoLast: ");
          //          Serial.println(midiNoteNoLast);
          usbMIDI.sendNoteOff(midiNoteNoLast, 64, 1); // turns the last MIDI note triggered off
          setMidiCount = 0; // reset setMidiCount ready for next AANF OUT_PROB
          midiNoteNoLast = midiNoteNo;
          state = TRIGGER_MIDI;
        }
      }
      else if (AANFStateReturn == OUT_PROBABILITY)
      {
        //Serial.println("OUT_PROBABILITY");
        if (setMidiCount == 0)
        {
          usbMIDI.sendNoteOff(midiNoteNoLast, 64, 1); // turns the last MIDI note triggered off
          //midiNoteNoLast = 0; // reset this to retrigger when back IN_PROBABILITY
          setMidiCount = 1; // to only do this once
        }
      }
      break;
    case TRIGGER_MIDI:

      if (pitchBendOn == true)
      {
        setPitchBend();
      }
      else
      {
      }

      if ((AARMSStateReturn == START_IN_THRESHOLD) && (trigMidiCount == 0))
      {
        //Serial.println("triggerMidi()");
        triggerMidi();
        trigMidiCount = 1;
        //state = SET_MIDI;
      }
      if (AARMSStateReturn == END_IN_THRESHOLD)
      {
        usbMIDI.sendNoteOff(midiNoteNoLast, 64, 1); // turns the last MIDI note triggered off
        //midiNoteNoLast = 0; // reset this to retrigger when back IN_RANGE
        trigMidiCount = 0;
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
void setMidi()
{
  // update the ResponsiveAnalogRead object
  freq = freqRAR.getValue();

  midiNoteNoF = 69.0 + 12.0 * log(freq / 440.0) / log(2);

  if (midiNoteNoF >= 38.0)
  {
    midiNoteNo = round(midiNoteNoF);
  }

   // debugging
//    currentTime = millis();
//    if (currentTime - lastTime >= delayTime)
//    {
//      Serial.print("freqRAR: ");
//      Serial.print(freqRAR.getRawValue());
//      Serial.print("\t");
//      Serial.print(freqRAR.getValue());
//      // if the repsonsive value has change, print out 'changed'
//      if (freqRAR.hasChanged()) {
//        Serial.print("\tchanged");
//      }
//      Serial.print("midiNoteNoF: ");
//      Serial.print(midiNoteNoF);
//      Serial.print(" | midiNoteNo: ");
//      Serial.print(midiNoteNo);
//      Serial.println();
//      lastTime = currentTime;
//    }

}

//--------------------------------------------------------------
void setPitchBend()
{
  // get freq from scalemanager
  freqSM = sm.getFrequency(midiNoteNo);

  midiNoteNoFRaw = 69.0 + 12.0 * log(freqRaw / 440.0) / log(2);
  if (midiNoteNoFRaw < 33.0)
  {
    midiNoteNoFRaw = midiNoteNo;
  }

  pitchBendDec = midiNoteNoFRaw - midiNoteNo;

  midiNoteNoPitch = 440.0 * pow(2, (midiNoteNo + pitchBendDec - 69.0) / 12.0);
  pitchBend = -round(4096 * 12.0 * log(freqSM / midiNoteNoPitch) / log(2));

  if (pitchBend < -8192)
  {
    pitchBend = -8192;
  }
  else if (pitchBend > 8192)
  {
    pitchBend = 8192;
  }

  // mapping pitchbend - not using this
  //  if (pitchBend > 0)
  //  {
  //    pitchBend = map(pitchBend, 0, 4096 * pitchBendDec, 0, 16384);
  //  }
  //  else if (pitchBend < 0)
  //  {
  //    pitchBend = map(pitchBend, 0, -4096 * pitchBendDec, 0, -16384);
  //  }

  if (pitchBend != pitchBendLast) {
    usbMIDI.sendPitchBend(pitchBend, 1);
    pitchBendLast = pitchBend;
  }

  // debugging
    currentTime = millis();
    if (currentTime - lastTime >= delayTime)
    {
  //    Serial.print("freqSM: ");
  //    Serial.println(freqSM);
      Serial.print("midiNoteNo: ");
      Serial.println(midiNoteNo);
      Serial.print("midiNoteNoFRaw: ");
      Serial.println(midiNoteNoFRaw);
      Serial.print("pitchBendDec: ");
      Serial.println(pitchBendDec);
      Serial.print("pitchBend: ");
      Serial.println(pitchBend);
      lastTime = currentTime;
    }

}

//--------------------------------------------------------------
void triggerMidi()
{
  //    usbMIDI.sendNoteOff(midiNoteNoLast, 64, 1);
  usbMIDI.sendNoteOn(midiNoteNo, velocity, 1);

}

//--------------------------------------------------------------
void panicMidi()
{
  //  // MIDI Panic button. Send All Notes Off on all channels.
  //  // Useful if a MIDI device has a note stuck on.
  //  for (uint8_t chan = 1; chan < 17; chan++) {
  //    // button pressed so send All Notes Off on all channels
  //    //AppleMIDI.controlChange(0x7b, 0x00, chan);
  //    AppleMIDI.controlChange(123, 0, chan);
  //    delay(2);
  //  }

}

//--------------------------------------------------------------
uint8_t AANFState()
{
  if (noteFreq.available())
  {
    //Serial.println("noteFreq.available()");
    freqRaw = noteFreq.read();
    prob = noteFreq.probability();
    if (prob >= 0.95) {
      freq95 = freqRaw;
      // update the ResponsiveAnalogRead object
      freqRAR.update(freq95);
      //Serial.println("IN_PROBABILITY");
      return IN_PROBABILITY;
    } else {
      //Serial.println("OUT_PROBABILITY");
      return OUT_PROBABILITY;
    }
  }

  //      // debugging
  //      // Serial.printf("Note: %3.2f | Probability: %.2f\n", freq, prob);
  //            currentTime = millis();
  //            if (currentTime - lastTime >= delayTime)
  //            {
  //              Serial.print("freqRaw: ");
  //              Serial.print(freqRaw);
  //              Serial.print(" | prob: ");
  //              Serial.print(prob);
  //              Serial.print(" | freq95: ");
  //              Serial.print(freq95);
  //              Serial.println();
  //              lastTime = currentTime;
  //            }
}

//--------------------------------------------------------------
uint8_t AARMSState() {

  if (rmsL.available())
  {
    //Serial.println("rmsL.available()");
    leftRMS = rmsL.read() * 1000.0;

    //        // detect min and max values
    //        if (leftRMS < leftRMSMin) {
    //          leftRMSMin = leftRMS;
    //        }
    //        if (leftRMS > leftRMSMax) {
    //          leftRMSMax = leftRMS;
    //        }

    leftRMSRAR.update(leftRMS);

    //    currentTime = millis();
    //    if (currentTime - lastTime >= delayTime)
    //    {
    //      Serial.print("leftRMS: ");
    //      Serial.print(leftRMS);
    //      Serial.print("| leftRMSRAR: ");
    //      Serial.println(leftRMSRAR.getValue());
    //      lastTime = currentTime;
    //    }

    if (leftRMSRAR.getValue() < endThreshold)
      //if (leftRMS < endThreshold)
    {
      //rmsEndTriggerFlag = true;
      //Serial.println("END_IN_THRESHOLD");
      return END_IN_THRESHOLD;
    }
    else if ((leftRMSRAR.getValue() > endThreshold) && (leftRMSRAR.getValue() < startThreshold))
      //else if ((leftRMS > endThreshold) && (leftRMS < startThreshold))
    {
      //rmsEndTriggerFlag = false;
      //Serial.println("END_OUT_THRESHOLD");
      return END_OUT_THRESHOLD;
      //rmsStartTriggerFlag = false;
      //Serial.println("START_OUT_THRESHOLD");
      return START_OUT_THRESHOLD;
    }
    else if (leftRMSRAR.getValue() > startThreshold)
      //else if (leftRMS > startThreshold)
    {
      //rmsStartTriggerFlag = true;
      //Serial.println("START_IN_THRESHOLD");
      return START_IN_THRESHOLD;
    }
    //
    //    //    // debugging
    //        currentTime = millis();
    //        if (currentTime - lastTime >= delayTime)
    //        {
    //    //            Serial.print("leftRMS: ");
    //    //            Serial.print(leftRMS);
    //    ////            Serial.print(" | leftRMSMin: ");
    //    ////            Serial.print(leftRMSMin);
    //    ////            Serial.print(" | leftRMSMax: ");
    //    ////            Serial.print(leftRMSMax);
    //                Serial.print("leftRMSRAR: ");
    //                Serial.print(leftRMSRAR.getRawValue());
    //                Serial.print(" | ");
    //                Serial.println(leftRMSRAR.getValue());
    //
    //                Serial.println();
    //          lastTime = currentTime;
    //        }
  }
}

////--------------------------------------------------------------
//uint8_t GPI0ButtonState()
//{
//  // Update the Bounce instance :
//  debouncer.update();
//
//  if (debouncer.fell()) {
//    return B_ON;
//    count = 0;
//  } else {
//    return B_OFF;
//  }
//
//}


//--------------------------------------------------------------
void loop() {

  float vol = analogRead(15);
  vol = vol / 1024;
  //Serial.println(vol);
  audioShield.volume(vol);
  //delay(20);

  state_machine_run(AANFState(), AARMSState());

  //  // debugging
  //  Serial.print("AudioProcessorUsage(): ");
  //  Serial.print(AudioProcessorUsage());
  //  Serial.print(" | AudioProcessorUsageMax(): ");
  //  Serial.println(AudioProcessorUsageMax());

}
