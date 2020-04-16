//
//  Scale.cpp
//  Leap_Midi_DatGui_working
//
//  Created by Dr X on 12/05/2019.
//

#include "Scale.h"

Scale::Scale() {
}

void Scale::setScale(const char* NAME, const unsigned int* NOTES, unsigned int LENGTH) {
    //scales are given as semitones relative to a fundamental, usually beginning with the fundamental (0)
    scaleName = (char*) NAME;
    scaleNotes = (unsigned int*) NOTES;
    scaleLength = LENGTH;
}

int Scale::getNote(int INDEX) {
    //returns the nth note in the scale in semitones. E.g. asking for the 3rd note in a major scale will return 4 (the 3rd in a major scale is always 4 semitones above the fundamental)
    //also handles numbers above 12 and below 0, e.g. asking for the -1th note in a major scale will return the major seventh (one semitone below).
    
    int sensibleIndex = INDEX;
    int low = 0;
    while (sensibleIndex < 0) {
        sensibleIndex += scaleLength;
        low--;
    }
    return scaleNotes[sensibleIndex % scaleLength] + 12 * ((sensibleIndex / scaleLength) + low);
}

