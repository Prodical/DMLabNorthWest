#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxDatGui.h"

class ofApp : public ofBaseApp, public ofxMidiListener {
    
public:
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    /// ofxMidi
    void newMidiMessage(ofxMidiMessage& eventArgs);
    
    ofxMidiIn midiIn;
    ofxMidiMessage midiMessage;
    std::vector<ofxMidiMessage> midiMessages;
    std::size_t maxMessages = 10; //< max number of messages to keep track of
    
    ofxMidiOut midiOut;
    int channel;
    
    unsigned int currentPgm;
    int note, velocity;
    int pan, bend, touch, polytouch;
    
    int midiNoteNo = 0;
    
    /// ofxDatGui
    ofxDatGuiValuePlotter* plotter;
    ofxDatGuiValuePlotter* plotter2;
    ofxDatGuiSlider* slider;
    ofxDatGuiSlider* slider2;
    ofxDatGuiDropdown* dropdownEaseType;
    ofxDatGuiDropdown* dropdownDrawModes;
    ofxDatGuiDropdown* dropdownScale;
    ofxDatGuiComponent* component;
    vector<ofxDatGuiComponent*> components;
    
    void onButtonEvent(ofxDatGuiButtonEvent e);
    void onToggleEvent(ofxDatGuiToggleEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);
    void onDropdownEvent(ofxDatGuiDropdownEvent e);
    void onMatrixEvent(ofxDatGuiMatrixEvent e);
    void onColorPickerEvent(ofxDatGuiColorPickerEvent e);
    void on2dPadEvent(ofxDatGui2dPadEvent e);
    void onTextInputEvent(ofxDatGuiTextInputEvent e);
    
    void ofxDatGuiSetup();
    void ofxDatGuiUpdate();
    void ofxDatGuiDraw();
    
     float ang1 = 0.0f;
    int p1Value, p2Value = 0;
    
    int fundamental = 48;
    int octave = 3;
    int keyNo = 0;
    std::string keyText;
    vector<std::string> scaleKey = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int scaleType = 1;
    
    
};

/*
 
 Running list of Midi Note Nos used
 
 0 - re-initialise sensor
 1 - toggle ease
 2 - ease duration - using velocity for values
 3 - ease type - velocity in steps of 10 for type
 4 - Connect Apple MIDI
 5 - scaleType - velocity in steps of 10 for type
 6 - fundamental - velocity is value
 
 */

 
 
 
 

