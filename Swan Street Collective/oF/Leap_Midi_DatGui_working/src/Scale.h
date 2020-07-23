//
//  Scale.h
//  Leap_Midi_DatGui_working
//
//  Created by Dr X on 12/05/2019.
//

#ifndef Scale_h
#define Scale_h

#include <stdio.h>

/*
 * This model of scale assumes use of the chromatic scale, i.e. 12 notes per octave.
 */

class Scale {
    
public:
    
    char* scaleName;
    unsigned int* scaleNotes;
    unsigned int scaleLength;
    
    Scale();
    void setScale(const char* NAME, const unsigned int* NOTES, unsigned int LENGTH);
    int getNote(int INDEX);
    
};

#endif /* Scale_h */
