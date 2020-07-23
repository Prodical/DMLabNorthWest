#pragma once

#include "ofMain.h"
#include "ofxLeapMotion2.h"
#include "ofxMidi.h"
#include "StopWatch.h"
#include "ScaleManager.h"
#include "ofxDatGui.h"

//#include <iostream>
//#include <iomanip>
#include <string>
#include <cstring>


class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
	
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit();
    
    /// ofxLeapMotion2
	ofxLeapMotion leap;
	vector <ofxLeapMotionSimpleHand> simpleHands;
    vector <Hand> hands;
	vector <int> fingersFound;
    int LMGestureNo, LMGestureNoLast = 0;
    vector<std::string> LMGestureS = {"none", "forward poke", "down tap", "swipe right", "swipe left", "swipe down", "swipe up", "swipe forward", "swipe back", "circle anti-clockwise", "circle clockwise"};
    bool LMGestureDetected = false;
    StopWatch LMGestureSW;
    
    /// ofxMidi
    ofxMidiOut midiOut;
    int channel;
    unsigned int currentPgm;
    int note, velocity;
    int pan, bend, touch, polytouch;
    
    /// ofEasyCam
	ofEasyCam cam;
    
    // lighting
    ofLight pointLight;
    ofLight pointLight2;
    ofLight pointLight3;
    ofMaterial material;
    bool movingLights = true;
    
    // trails
    ofPoint indexTipR, indexTipL;
    deque<ofPoint> indexTipsR, indexTipsL;
    bool isLeft;
    
    bool handsDetected = false;
    
    // strings
    int noStrings = 12;
    ofCylinderPrimitive string;
    vector<ofCylinderPrimitive> strings;
    int stringSpacing;
    ofVec2f stringXInOut;
    vector<ofVec2f> stringsXInOut;
    void setupStrings(int _noStrings);
    int diameter = 10;
    int sSpacing = 50;
    
    // pedals
    int noPedals = 8;
    ofBoxPrimitive pedal;
    vector<ofBoxPrimitive> pedals;
    int pedalSpacing;
    ofVec2f pedalYInOut;
    vector<ofVec2f> pedalsYInOut;
    void setupPedals(int _noPedals);
    int pWidth = 65;
    int pHeight = 150;
    int pDepth = 10;
    int pSpacing = 40;
    
    int playMode = 0;
    
    // Chris Ball's ScaleManager library - ported from Arduino
    ScaleManager sm;
    int fundamentalS = 48;
    int fundamentalP = 36;
    int scaleType = 1;
    void initialiseScale();
    void setupScale(int _fundamentalS, int _scaleType);
    vector<int> midiScaleS, midiScaleP;
    vector<std::string> scaleKey = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    vector<std::string> scaleTypeString = {"CHROMATIC", "MAJOR", "MINOR", "PENTATONIC"};
    int keyNo = 0;
    std::string keyText;
    int octave = 3;
    
    //int noteNoBottom = 48;
    //int noteRange = 25;
    int lastMidiNoteNoR, lastMidiNoteNoL = 60;
    vector<int> sTriggerOnR, sTriggerOffR, sTriggerOnL, sTriggerOffL, pTriggerOnR, pTriggerOffR, pTriggerOnL, pTriggerOffL;

    /// StopWatch - ported from Arduino
    StopWatch triggerRSW, triggerLSW;
    
    bool midiOn = true;
    int velocityR, velocityL = 0;
    
    /// ofxDatGui
    ofxDatGui* gui;
    //ofxDatGuiValuePlotter* plotter;
    //ofxDatGuiValuePlotter* plotter2;
    //ofxDatGuiSlider* slider;
    //ofxDatGuiSlider* slider2;
    ofxDatGuiDropdown* dropdown;
    //ofxDatGuiDropdown* dropdown2;
    
    
    ofxDatGuiComponent* component;
    //vector<ofxDatGuiComponent*> components;
    
    //ofxDatGuiTextInput* KeyTI;
    //ofxDatGuiTextInput* noStringsTI;
    
    //ofxDatGuiFolder* setKeyFolder;
    ofxDatGuiFolder* setStringsFolder;
    
    //void onButtonEvent(ofxDatGuiButtonEvent e);
    //void onToggleEvent(ofxDatGuiToggleEvent e);
    //void onSliderEvent(ofxDatGuiSliderEvent e);
    void onDropdownEvent(ofxDatGuiDropdownEvent e);
    //void onMatrixEvent(ofxDatGuiMatrixEvent e);
    //void onColorPickerEvent(ofxDatGuiColorPickerEvent e);
    //void on2dPadEvent(ofxDatGui2dPadEvent e);
    void onTextInputEvent(ofxDatGuiTextInputEvent e);
    
    void ofxDatGuiSetup();
    void ofxDatGuiUpdate();
    //void ofxDatGuiDraw();
    
    bool mFullscreen;
    void refreshWindow();
    void toggleFullscreen();
    
    uint tIndex;
    vector<ofxDatGuiTheme*> themes;
    
       int p1Value, p2Value = 0;
    
    // on display fonts
    ofTrueTypeFont NewMediaFett18;
    ofTrueTypeFont NewMediaFett14;
    
};
