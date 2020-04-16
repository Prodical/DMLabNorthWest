//
//  ScaleManager.cpp
//  Leap_Midi_DatGui_working
//
//  Created by Dr X on 12/05/2019.
//

#include "ScaleManager.h"

ScaleManager::ScaleManager() {
}

ScaleManager::ScaleManager(bool DEFAULTS) {
    if (DEFAULTS) {
        addScale("Chromatic",  (const unsigned int[]) {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, 12);
        addScale("Major",      (const unsigned int[]) {0, 2, 4, 5, 7, 9, 11 },                7 );
        addScale("Minor",      (const unsigned int[]) {0, 2, 3, 5, 7, 8, 10 },                7 );
        addScale("Pentatonic", (const unsigned int[]) {0, 2, 4, 7, 9 },                       5 );
    }
}

int ScaleManager::getScaleNote(int NOTE) {
    return fundamental + scales[currentScale].getNote(NOTE);
}

float ScaleManager::getScaleNoteFrequency(int NOTE) {
    return getFrequency(getScaleNote(NOTE));
}

float ScaleManager::getFrequency(int NOTE) {
    return 220*pow(2,(float(NOTE-57))/12);
}

char* ScaleManager::getNoteName(int NOTE){
    return (char*) noteNames[NOTE%12];
}

char* ScaleManager::getScaleNoteName(int NOTE){
    return getNoteName(getScaleNote(NOTE));
}

unsigned int ScaleManager::getNoteOctave(int NOTE){
    return NOTE/12;
}

unsigned int ScaleManager::getScaleNoteOctave(int NOTE){
    return getNoteOctave(getScaleNote(NOTE));
}

char* ScaleManager::getFundamentalName() {
    return getNoteName(fundamental);
}

unsigned int ScaleManager::getFundamentalOctave(){
    return getNoteOctave(fundamental);
}

char* ScaleManager::getScaleName() {
    return scales[currentScale].scaleName;
}

unsigned int ScaleManager::getNumScales(){
    return scalesLoaded;
}

void ScaleManager::setFundamental(unsigned int F) {
    fundamental = F;
}

void ScaleManager::setCurrentScale(unsigned int S) {
    currentScale = S;
}

unsigned int ScaleManager::addScale(const char* NAME, const unsigned int* NOTES, const unsigned int LENGTH) {
    if (scalesLoaded < MAX_SCALES) {
        scales[scalesLoaded].setScale(NAME, NOTES, LENGTH);
        scalesLoaded++;
        return scalesLoaded;
    }else{
        return 0;
    }
}

