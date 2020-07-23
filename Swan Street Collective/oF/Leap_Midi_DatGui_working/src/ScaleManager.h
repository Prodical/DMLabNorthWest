//
//  ScaleManager.h
//  Leap_Midi_DatGui_working
//
//  Created by Dr X on 12/05/2019.
//

#ifndef ScaleManager_h
#define ScaleManager_h

#include <stdio.h>
//using namespace std;
#include "ofMain.h"

#include "Scale.h"
#include "noteDefs.h"

#define CHROMATIC 0
#define MAJOR 1
#define MINOR 2
#define PENTATONIC 3

#define MAX_SCALES 16



class ScaleManager {
public:  //currently, all variables are public
    const char* noteNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    unsigned int currentScale = 0;
    unsigned int fundamental = 60; //middle C is default
    Scale scales[MAX_SCALES];
    unsigned int scalesLoaded = 0;
    
    ScaleManager();
    ScaleManager(bool DEFAULTS);
    int getScaleNote(int NOTE);
    float getScaleNoteFrequency(int NOTE);
    float getFrequency(int NOTE);
    char* getNoteName(int NOTE);
    char* getScaleNoteName(int NOTE);
    unsigned int getNoteOctave(int NOTE);
    unsigned int getScaleNoteOctave(int NOTE);
    char* getFundamentalName();
    unsigned int getFundamentalOctave();
    char* getScaleName();
    unsigned int getNumScales();
    
    void setFundamental(unsigned int F);
    void setCurrentScale(unsigned int S);
    unsigned int addScale(const char* NAME, const unsigned int* NOTES, unsigned int LENGTH);
};

#endif /* ScaleManager_h */
