#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
	//ofSetLogLevel(OF_LOG_VERBOSE);
    
    // oF window display
    //old OF default is 96 - but this results in fonts looking larger than in other programs.
    ofTrueTypeFont::setGlobalDpi(94); //72
    NewMediaFett18.load("NewMediaFett.ttf", 18, true, true);
    NewMediaFett18.setLineHeight(19.0f);
    NewMediaFett18.setLetterSpacing(1.037);
    NewMediaFett14.load("NewMediaFett.ttf", 14, true, true);
    NewMediaFett14.setLineHeight(15.0f);
    NewMediaFett14.setLetterSpacing(1.037);
    
    /// ofxLepMotion2
	leap.open();
    // keep app receiving data from leap motion even when it's in the background
    leap.setReceiveBackgroundFrames(true);
    // enable default gestures
    leap.setupGestures();
    
	cam.setOrientation(ofPoint(-10, 0, 0));
    cam.setDistance(950.0);
    
    /// ofxMidi
    // print the available output ports to the console
    //midiOut.listOutPorts();
    // connect
    //midiOut.openPort(0); // by number
    //midiOut.openPort("IAC Driver Pure Data In"); // by name
    midiOut.openVirtualPort("ofxMidiOut"); // open a virtual port
    
    channel = 1;
    currentPgm = 0;
    note = 0;
    velocity = 0;
    pan = 0;
    bend = 0;
    touch = 0;
    polytouch = 0;
    
    setupStrings(noStrings);
    setupPedals(noPedals);
    initialiseScale();
    setupScale(fundamentalS, scaleType);
   
    ofSetSmoothLighting(true);
    pointLight.setDiffuseColor(ofFloatColor(.95f, .95f, .95f));
    pointLight.setSpecularColor(ofFloatColor(1.f, 1.f, 1.f));
    
    //pointLight2.setDiffuseColor(ofFloatColor(238.f/255.f, 57.f/255.f, 135.f/255.f));
    pointLight2.setDiffuseColor(ofFloatColor(.65f, .65f, .65f));
    pointLight2.setSpecularColor(ofFloatColor(.8f, .8f, .8f));
    
    //pointLight3.setDiffuseColor(ofFloatColor(19.f/255.f, 94.f/255.f, 77.f/255.f));
    //pointLight3.setSpecularColor(ofFloatColor(18.f/255.f, 150.f/255.f, 135.f/255.f));
    pointLight3.setDiffuseColor(ofFloatColor(.35f, .35f, .35f));
    pointLight3.setSpecularColor(ofFloatColor(.15f, .15f, .15f));
    
    // shininess is a value between 0 - 128, 128 being the most shiny //
    material.setShininess(120);
    // the light highlight of the material //
    material.setSpecularColor(ofColor(255, 255, 255, 255));
    
    /// ofxDatGui
    ofxDatGuiSetup();
    
    
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
}

//--------------------------------------------------------------
void ofApp::setupStrings(int _noStrings)
{
    noStrings = _noStrings;
    //#define noStrings 30
    
    // all elements defined by noStrings
    //int midiScaleS[noStrings] = {48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72}; // C Major scale 2 octaves
    midiScaleS.assign(noStrings, 0);
    
    sTriggerOnR.assign(noStrings, 0);
    sTriggerOffR.assign(noStrings, 0);
    sTriggerOnL.assign(noStrings, 0);
    sTriggerOffL.assign(noStrings, 0);
    
    //int sTriggerOnR[noStrings], sTriggerOffR[noStrings], sTriggerOnL[noStrings], sTriggerOffL[noStrings] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
    indexTipR.set(0.0, 0.0, 0.0);
    indexTipsR.assign(sSpacing, indexTipR);
    indexTipL.set(0.0, 0.0, 0.0);
    indexTipsL.assign(sSpacing, indexTipL);
    
    string.set(diameter, 600.0);
    string.setResolution(20, 20);
    strings.assign(noStrings, string);
    stringSpacing = sSpacing*15/noStrings;
    
    stringXInOut.set(0.0, 0.0);
    stringsXInOut.assign(noStrings, stringXInOut);
    for (int i=0; i<noStrings; i++)
    {
        stringsXInOut[i].x = stringSpacing*(i-(noStrings-1)/2)-string.getRadius()/2;
        stringsXInOut[i].y = stringsXInOut[i].x+string.getRadius();
        //cout << "stringsXInOut[" << i << "]: " << stringsXInOut[i].x << "," << stringsXInOut[i].y << endl;
    }
}

//--------------------------------------------------------------
void ofApp::setupPedals(int _noPedals)
{
    noPedals = _noPedals;
    
    midiScaleP.assign(noPedals, 0);
    
    //pTriggerOnR.assign(noPedals, 0);
    //pTriggerOffR.assign(noPedals, 0);
    pTriggerOnL.assign(noPedals, 0);
    pTriggerOffL.assign(noPedals, 0);
    
    pedal.set(pWidth,pHeight,pDepth);
    pedals.assign(noPedals, pedal);
    pedalSpacing = pSpacing*15/noPedals;
    
    pedalYInOut.set(0.0, 0.0);
    pedalsYInOut.assign(noPedals, pedalYInOut);
    
    for (int i=0; i<noPedals; i++)
    {
        pedalsYInOut[i].x = pedalSpacing*(i-(noPedals-1)/2)-pedal.getWidth()/2;
        pedalsYInOut[i].y = pedalsYInOut[i].x+pedal.getWidth();
        //cout << "pedalsYInOut[" << i << "]: " << pedalsYInOut[i].x << "," << pedalsYInOut[i].y << endl;
    }
    
}

//--------------------------------------------------------------
void ofApp::initialiseScale()
{
    //ScaleManager sm(true);  //use 'true' to load default scales: Chromatic, Major, Minor, Pentatonic
    sm.addScale("Chromatic",  (const unsigned int[]) {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, 12);
    sm.addScale("Major",      (const unsigned int[]) {0, 2, 4, 5, 7, 9, 11 },                7 );
    sm.addScale("Minor",      (const unsigned int[]) {0, 2, 3, 5, 7, 8, 10 },                7 );
    sm.addScale("Pentatonic", (const unsigned int[]) {0, 2, 4, 7, 9 },                       5 );
}

//--------------------------------------------------------------
void ofApp::setupScale(int _fundamentalS, int _scaleType)
{
    fundamentalS = _fundamentalS;
    fundamentalP = 36;
    scaleType = _scaleType;
    
    sm.setFundamental(fundamentalS);
    
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
    for (int i = 0; i < noStrings+1; i++) {
        midiScaleS[i] = sm.getScaleNote(i);
    }
    
    sm.setFundamental(fundamentalP);
    for (int i = 0; i < noPedals+1; i++) {
        midiScaleP[i] = sm.getScaleNote(i);
    }
    // debugging
    //      for (int i = 0; i < noStrings; i++) {
    //          cout << "midiScaleS[" << i << "]: " << midiScaleS[i] << endl;
    //      }
}


//--------------------------------------------------------------
void ofApp::update(){
    
    /// ofxDatGui
    //ofxDatGuiUpdate();
    
    //cout << "cam.getDistance(): " << cam.getDistance() << endl;
    
    
    // lighting
    if (movingLights == false)
    {
        // static position - arbitarily selected - TO DO - refine these positions
        pointLight.setPosition(ofGetWidth()/2, ofGetHeight()/2, 500);
        pointLight2.setPosition(ofGetWidth()/2, ofGetHeight()/2, -300);
        pointLight3.setPosition(ofGetWidth(), ofGetWidth(), 0);
    }
    else
    {
        // in motion - from 3D primitivesExample
        pointLight.setPosition((ofGetWidth()*.5)+cos(ofGetElapsedTimef()*.5)*(ofGetWidth()*.3),
                               ofGetHeight()/2,
                               500);
        pointLight2.setPosition((ofGetWidth()*.5)+cos(ofGetElapsedTimef()*.15)*(ofGetWidth()*.3),
                                ofGetHeight()*.5+sin(ofGetElapsedTimef()*.7)*(ofGetHeight()),
                                -300);
        pointLight3.setPosition(
                                cos(ofGetElapsedTimef()*1.5) * ofGetWidth()*.5,
                                sin(ofGetElapsedTimef()*1.5f) * ofGetWidth()*.5,
                                cos(ofGetElapsedTimef()*.2) * ofGetWidth()
                                );
    }

    // Leap Motion default gestures
    leap.updateGestures();
    //cout << "leap.iGestures: " << leap.iGestures << endl;
    
    // only detect gestures out of X range of the strings
    if (((indexTipR.x < stringsXInOut[0].x) || (indexTipR.x > stringsXInOut[15].y)) && ((indexTipL.x < stringsXInOut[0].x) || (indexTipL.x > stringsXInOut[15].y)))
    {
        if (leap.iGestures == 1) // forward poke
        {
            LMGestureNo = 1;
            //LMGestureDetected = true;
            //cout << "forward poke" << endl;
            //handsDetected = true;
            if (LMGestureNoLast != LMGestureNo)
            {
                LMGestureSW.start();
            }
            LMGestureNoLast = LMGestureNo;
        }
        else if (leap.iGestures == 2) // down tap
        {
            LMGestureNo = 2;
            //LMGestureDetected = true;
            //cout << "down tap" << endl;
            if (LMGestureNoLast != LMGestureNo)
            {
                LMGestureSW.start();
            }
            LMGestureNoLast = LMGestureNo;
        }
        else if (leap.iGestures == 3) // swipe right
        {
            LMGestureNo = 3;
            //LMGestureDetected = true;
            //cout << "swipe right" << endl;
            if (LMGestureNoLast != LMGestureNo)
            {
                LMGestureSW.start();
            }
            if (LMGestureNoLast != LMGestureNo)
            {
                LMGestureSW.start();
//                keyNo = keyNo+1;
//                if (keyNo > 11)
//                {
//                    keyNo = 0;
//                }
//                fundamentalS = 12 + 12*octave + keyNo;
//                setupScale(fundamentalS, scaleType);
//                //LMGestureDetected = false;
            }
            LMGestureNoLast = LMGestureNo;
        }
        else if (leap.iGestures == 4) // swipe left
        {
            LMGestureNo = 4;
            //LMGestureDetected = true;
            if (LMGestureNoLast != LMGestureNo)
            {
                LMGestureSW.start();
//                keyNo = keyNo-1;
//                if (keyNo < 0)
//                {
//                    keyNo = 11;
//                }
//                fundamentalS = 12 + 12*octave + keyNo;
//                setupScale(fundamentalS, scaleType);
//                //LMGestureDetected = false;
            }
            LMGestureNoLast = LMGestureNo;
            
            //cout << "swipe left" << endl;
        }
        else if (leap.iGestures == 5) // swipe down
        {
            LMGestureNo = 5;
            //LMGestureDetected = true;
            //cout << "swipe down" << endl;
            if (LMGestureNoLast != LMGestureNo)
            {
                LMGestureSW.start();
            }
            LMGestureNoLast = LMGestureNo;
        }
        else if (leap.iGestures == 6) // swipe up
        {
            LMGestureNo = 6;
            //LMGestureDetected = true;
            //cout << "swipe up" << endl;
            if (LMGestureNoLast != LMGestureNo)
            {
                LMGestureSW.start();
            }
            LMGestureNoLast = LMGestureNo;
        }
        else if (leap.iGestures == 7) // swipe forward
        {
            LMGestureNo = 7;
            //LMGestureDetected = true;
            //cout << "swipe forward" << endl;
            if (LMGestureNoLast != LMGestureNo)
            {
                LMGestureSW.start();
            }
            LMGestureNoLast = LMGestureNo;
        }
        else if (leap.iGestures == 8) // swipe back
        {
            LMGestureNo = 8;
            //LMGestureDetected = true;
            //cout << "swipe back" << endl;
            if (LMGestureNoLast != LMGestureNo)
            {
                LMGestureSW.start();
            }
            LMGestureNoLast = LMGestureNo;
        }
        else if (leap.iGestures == 9) // circle anti-clockwise
        {
            LMGestureNo = 9;
            //LMGestureDetected = true;
            //cout << "circle anti-clockwise" << endl;
            if (LMGestureNoLast != LMGestureNo)
            {
                LMGestureSW.start();
                midiOn = false;
            }
            LMGestureNoLast = LMGestureNo;
        }
        else if (leap.iGestures == 10) // circle clockwise
        {
            LMGestureNo = 10;
            //LMGestureDetected = true;
            //cout << "circle clockwise" << endl;
            if (LMGestureNoLast != LMGestureNo)
            {
                LMGestureSW.start();
                midiOn = true;
            }
            LMGestureNoLast = LMGestureNo;
        }
    }
    
    if (LMGestureSW.elapsed() > 500)
    {
        LMGestureSW.stop();
        LMGestureSW.reset();
        LMGestureNo = 0;
    }
    
	fingersFound.clear();
    
	//here is a simple example of getting the hands and drawing each finger and joint
	//the leap data is delivered in a threaded callback - so it can be easier to work with this copied hand data
	//if instead you want to get the data as it comes in then you can inherit ofxLeapMotion and implement the onFrame method.
	//there you can work with the frame data directly.
    //Option 1: Use the simple ofxLeapMotionSimpleHand - this gives you quick access to fingers and palms.
    
    simpleHands = leap.getSimpleHands();

    if( leap.isFrameNew() && simpleHands.size() ){

        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
        leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
        leap.setMappingZ(-150, 150, -200, 200);

        fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};

        for(int i = 0; i < simpleHands.size(); i++){
            for (int f=0; f<5; f++) {
                int id = simpleHands[i].fingers[ fingerTypes[f] ].id;
                ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp; // metacarpal
                ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip; // proximal
                ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip; // distal
                ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip; // fingertip
                fingersFound.push_back(id);
            }
        }
    }
    
    //Option 2: Work with the leap data / sdk directly - gives you access to more properties than the simple approach
    //uncomment code below and comment the code above to use this approach. You can also inhereit ofxLeapMotion and get the data directly via the onFrame callback.
    /*
    hands = leap.getLeapHands();
    if( leap.isFrameNew() && hands.size() ){
        
        //leap returns data in mm - lets set a mapping to our world space.
        //you can get back a mapped point by using ofxLeapMotion::getMappedofPoint with the Leap::Vector that tipPosition returns
        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
        leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
        leap.setMappingZ(-150, 150, -200, 200);
        
        fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
        
        for(int i = 0; i < hands.size(); i++){
            for(int j = 0; j < 5; j++){
                ofPoint pt;
                
                const Finger & finger = hands[i].fingers()[ fingerTypes[j] ];
                
                //here we convert the Leap point to an ofPoint - with mapping of coordinates
                //if you just want the raw point - use ofxLeapMotion::getofPoint
                pt = leap.getMappedofPoint( finger.tipPosition() );
                pt = leap.getMappedofPoint( finger.jointPosition(finger.JOINT_DIP) );
                
                fingersFound.push_back(finger.id());
            }
        }
    }
     */
    
    // set velocities according to Y axis
    velocityR = ofMap(indexTipR.y, -string.getHeight()/2, string.getHeight()/2, 0, 127);
    if (playMode == 0)
    {
    velocityL = ofMap(indexTipL.y, -string.getHeight()/2, string.getHeight()/2, 0, 127);
    }
    else if (playMode == 1)
    {
        //cout << "stringsXInOut[0].x: " << stringsXInOut[0].x << endl;
        //cout << "stringsXInOut[noStrings-1].y: " << stringsXInOut[noStrings-1].y << endl;
       velocityL = ofMap(indexTipL.x, stringsXInOut[0].x, stringsXInOut[noStrings-1].y, 40, 127);
        cout << "indexTipL.x: " << indexTipL.x << endl;
        cout << "velocityL: " << velocityL << endl;
    }
    
    // trigger MIDI
    if (midiOn == true)
    {
        for (int i=0; i<noStrings; i++)
        {
            if ((indexTipR.x > stringsXInOut[i].x) && (indexTipR.x < stringsXInOut[i].y))
            {
                if (sTriggerOnR[i] == 0)
                {
                    midiOut.sendNoteOff(1, lastMidiNoteNoR, 64);
                    midiOut.sendNoteOn(1, midiScaleS[i], velocityR);
                    lastMidiNoteNoR = midiScaleS[i];
                    triggerRSW.start();
                    sTriggerOnR[i] = 1;
                    sTriggerOffR[i] = 0;
                }
            }
            else
            {
                if (sTriggerOffR[i] == 0)
                {
                    //midiOut.sendNoteOff(1, lastMidiNoteNo, 64);
                    sTriggerOnR[i] = 0;
                    sTriggerOffR[i] = 1;
                }
            }
            if (playMode == 0)
            {
                if ((indexTipL.x > stringsXInOut[i].x) && (indexTipL.x < stringsXInOut[i].y))
                {
                    if (sTriggerOnL[i] == 0)
                    {
                        midiOut.sendNoteOff(1, lastMidiNoteNoL, 64);
                        midiOut.sendNoteOn(1, midiScaleS[i], velocityL);
                        lastMidiNoteNoL = midiScaleS[i];
                        triggerLSW.start();
                        sTriggerOnL[i] = 1;
                        sTriggerOffL[i] = 0;
                    }
                }
                else
                {
                    if (sTriggerOffL[i] == 0)
                    {
                        //midiOut.sendNoteOff(1, lastMidiNoteNo, 64);
                        sTriggerOnL[i] = 0;
                        sTriggerOffL[i] = 1;
                    }
                }
            }
            else if (playMode == 1)
            {
                if ((indexTipL.y > pedalsYInOut[i].x) && (indexTipL.y < pedalsYInOut[i].y))
                {
                    if (pTriggerOnL[i] == 0)
                    {
                        midiOut.sendNoteOff(1, lastMidiNoteNoL, 64);
                        midiOut.sendNoteOn(1, midiScaleP[i], velocityL);
                        lastMidiNoteNoL = midiScaleP[i];
                        triggerLSW.start();
                        pTriggerOnL[i] = 1;
                        pTriggerOffL[i] = 0;
                    }
                }
                else
                {
                    if (pTriggerOffL[i] == 0)
                    {
                        //midiOut.sendNoteOff(1, lastMidiNoteNo, 64);
                        pTriggerOnL[i] = 0;
                        pTriggerOffL[i] = 1;
                    }
                }
            }
        }
        
        if (triggerRSW.elapsed()>3000)
        {
            midiOut.sendNoteOff(1, lastMidiNoteNoR, 64);
            triggerRSW.stop();
            triggerRSW.reset();
        }
        if (triggerLSW.elapsed()>3000)
        {
            midiOut.sendNoteOff(1, lastMidiNoteNoL, 64);
            triggerLSW.stop();
            triggerLSW.reset();
        }
    }
    
	//IMPORTANT! - tell ofxLeapMotion that the frame is no longer new.
	leap.markFrameAsOld();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    // enable lighting
    ofEnableDepthTest();
    ofEnableLighting();
    pointLight.enable();
    pointLight2.enable();
    pointLight3.enable();
    
    ofBackgroundGradient(ofColor(200,200,200),ofColor(130,130,130), OF_GRADIENT_BAR);
	
//    ofSetColor(0);
//    ofDrawBitmapString("ofxLeapMotion - Example App\nLeap Connected? " + ofToString(leap.isConnected()), 20, 20);
    
	cam.begin();
    
    // set up the 3D space
	ofPushMatrix();
    ofRotateDeg(90,0,0,1);
    ofSetColor(20);
    ofDrawGridPlane(800,20,false);
    
    // set up the strings
    ofRotateDeg(-90, 0, 0, 1);
    ofTranslate(-stringSpacing*(noStrings-1)/2,0,0);
    //material.begin();
    
    for (int i=0; i<noStrings; i++)
    {
        // turn the string magenta when indexTipR.x is within it's radius
        if ((indexTipR.x > stringsXInOut[i].x) && (indexTipR.x < stringsXInOut[i].y))
        {
                ofSetColor(255,0,255);
        }
        // turn the string cyan when indexTipL.x is within it's radius
        else if ((indexTipL.x > stringsXInOut[i].x) && (indexTipL.x < stringsXInOut[i].y))
        {
            if (playMode == 0)
            {
                ofSetColor(0,255,255);
            }
        }
        else
        {
            ofSetColor(128);
        }
        strings[i].draw();
        ofTranslate(stringSpacing,0,0);
    }
    
    // set up the pedals
    if (playMode == 1)
    {
        ofTranslate(-850,-pedalSpacing*(noPedals-1)/2,0);
        ofRotateDeg(90, 0, 0, 1);
        
        for (int i=0; i<noPedals; i++)
        {
            if ((indexTipL.y > pedalsYInOut[i].x) && (indexTipL.y < pedalsYInOut[i].y))
            {
                ofSetColor(0,255,255);
            }
            else
            {
                ofSetColor(128);
            }
            pedals[i].draw();
            ofTranslate(pedalSpacing,0,0);
        }
    }
    
    //material.end();
	ofPopMatrix();
    
    // Leap Motion stuff
    fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
    
    //cout << "simpleHands.size(): " << simpleHands.size() << endl;
    for(int i = 0; i < simpleHands.size(); i++){
        isLeft = simpleHands[i].isLeft;
        //cout << "isLeft: " << isLeft << endl;
        ofPoint handPos = simpleHands[i].handPos;
        ofPoint handNormal = simpleHands[i].handNormal;
    
//        for(int i = 0; i < hands.size(); i++){
//            bool isLeft = hands[i].isLeft;
//            ofPoint handPos = hands[i].handPos;
//            ofPoint handNormal = hands[i].handNormal;
    
        ofSetColor(208,111,56,150);
        ofDrawSphere(handPos.x,handPos.y,handPos.z,30);
        ofSetColor(255,255,255,150);
        ofDrawArrow(handPos,handPos+100*handNormal);
        
        for (int f=0; f<5; f++) {
            ofPoint mcp = simpleHands[i].fingers[fingerTypes[f]].mcp;  // metacarpal
            ofPoint pip = simpleHands[i].fingers[fingerTypes[f]].pip;  // proximal
            ofPoint dip = simpleHands[i].fingers[fingerTypes[f]].dip;  // distal
            ofPoint tip = simpleHands[i].fingers[fingerTypes[f]].tip;  // fingertip
            
            // copy index tip R to a deque for the trail
            if ((i == 0) && (f == 1) && (isLeft == 0))
            {
                indexTipR = tip;
                indexTipsR.push_front(indexTipR);
                if (indexTipsR.size() > 50)
                {
                    indexTipsR.pop_back();
                }
                //cout << "indexTipsR.size(): " << indexTipsR.size() << endl;
            }
            // copy index tip L to a deque for the trail
            if ((i == 1) && (f == 1) && (isLeft == 1))
            {
                indexTipL = tip;
                indexTipsL.push_front(indexTipL);
                if (indexTipsL.size() > 50)
                {
                    indexTipsL.pop_back();
                }
                //cout << "indexTipsL.size(): " << indexTipsL.size() << endl;
            }
            
            // draw finger joints
            ofSetColor(168, 66, 17, 150);
            ofDrawSphere(mcp.x, mcp.y, mcp.z, 15);
            ofDrawSphere(pip.x, pip.y, pip.z, 14);
            ofDrawSphere(dip.x, dip.y, dip.z, 13);
            ofDrawSphere(tip.x, tip.y, tip.z, 12);
            
            // draw bones
            ofSetColor(143, 70, 29, 150);
            ofSetLineWidth(20);
            ofDrawLine(mcp.x, mcp.y, mcp.z, pip.x, pip.y, pip.z);
            ofDrawLine(pip.x, pip.y, pip.z, dip.x, dip.y, dip.z);
            ofDrawLine(dip.x, dip.y, dip.z, tip.x, tip.y, tip.z);
        }
    }
    
    // trails
//    if (handsDetected == false)
//    {
//        ofSetColor(255, 0, 255, 255);
//        ofDrawSphere(350, 0, 0, 5);
//        ofSetColor(0, 255, 255, 255);
//        ofDrawSphere(-350, 0, 0, 5);
//    }
//    else
//    {
        for (int i=0; i<indexTipsR.size(); i++)
        {
            ofSetColor(255, 0, 255, 255 - 255*i/50);
            ofDrawSphere(indexTipsR[i].x, indexTipsR[i].y, indexTipsR[i].z, 5);
        }
        for (int i=0; i<indexTipsL.size(); i++)
        {
            ofSetColor(0, 255, 255, 255 - 255*i/50);
            ofDrawSphere(indexTipsL[i].x, indexTipsL[i].y, indexTipsL[i].z, 5);
        }
    //}
    
    //material.end();
    ofDisableLighting();
    ofDisableDepthTest();
    
	cam.end();
    
//    // let's see something
//    ofSetColor(0);
//    stringstream text;
//    text << "connected to port " << midiOut.getPort()
//    << " \"" << midiOut.getName() << "\"" << endl
//    << "is virtual?: " << midiOut.isVirtual() << endl << endl
//    << "sending to channel " << channel << endl << endl
//    << "current program: " << currentPgm << endl << endl
//    << "note: " << note << endl
//    << "velocity: " << velocity << endl
//    << "pan: " << pan << endl
//    << "bend: " << bend << endl
//    << "touch: " << touch << endl
//    << "polytouch: " << polytouch;
//    ofDrawBitmapString(text.str(), 20, 70);
//
//    stringstream text2;
//    text2.precision(4);
//    text2 << "indexTipR.xyz: " <<  indexTipR.x << "," << indexTipR.y << "," << indexTipR.z << endl;
//    ofDrawBitmapString(text2.str(), 20, 700);
//    stringstream text3;
//    text3.precision(4);
//    text3 << "indexTipL.xyz: " <<  indexTipL.x << "," << indexTipL.y << "," << indexTipL.z << endl;
//    ofDrawBitmapString(text3.str(), 20, 720);
    
    /// ofxDatGui
    //ofSetColor(255);
    //ofSetBackgroundColor(255, 255, 255);
    //for(int i=0; i<components.size(); i++) components[i]->draw();
    
    std::string scaleDisplay = "current scale: ";
    scaleDisplay += scaleKey[keyNo];
    scaleDisplay += " ";
    scaleDisplay += scaleTypeString[scaleType];
    ofRectangle rect = NewMediaFett18.getStringBoundingBox(scaleDisplay, 0,0);
    ofSetColor(0);
    NewMediaFett18.drawString(scaleDisplay, ofGetWidth()/2-rect.width/2, ofGetHeight()-50);
    
    std::string LMGestureDisplay = "LM gesture: ";
    LMGestureDisplay += LMGestureS[LMGestureNo];
    ofRectangle rect2 = NewMediaFett14.getStringBoundingBox(LMGestureDisplay, 0,0);
     ofSetColor(0);
    NewMediaFett14.drawString(LMGestureDisplay, ofGetWidth()/2-rect2.width/2, ofGetHeight()-20);
    
}

//--------------------------------------------------------------
void ofApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
    // clean up
    midiOut.closePort();
}

/// other oF functions
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    // ignore shift+s since it is used in keyReleased for sysex sending
    if(key == 'S') return;
    
    // send a note on if the key is a letter or a number
    if(isalnum((unsigned char) key)) {
        
        // scale the ascii values to midi velocity range 0-127
        // see an ascii table: http://www.asciitable.com/
        note = ofMap(key, 48, 122, 0, 127);
        velocity = 64;
        midiOut.sendNoteOn(channel, note,  velocity);
        
        // print out both the midi note and the frequency
        ofLogNotice() << "note: " << note
        << " freq: " << ofxMidi::mtof(note) << " Hz";
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    switch(key) {
            
            // send pgm change on arrow keys
        case OF_KEY_UP:
            currentPgm = (int) ofClamp(currentPgm+1, 0, 127);
            midiOut.sendProgramChange(channel, currentPgm);
            break;
        case OF_KEY_DOWN:
            currentPgm = (int) ofClamp(currentPgm-1, 0, 127);
            midiOut << ProgramChange(channel, currentPgm); // stream interface
            break;
            
            // aftertouch
        case '[':
            touch = 64;
            midiOut.sendAftertouch(channel, touch);
            break;
        case ']':
            touch = 127;
            midiOut << Aftertouch(channel, touch); // stream interface
            break;
            
            // poly aftertouch
        case '<':
            polytouch = 64;
            midiOut.sendPolyAftertouch(channel, 64, polytouch);
            break;
        case '>':
            polytouch = 127;
            midiOut << PolyAftertouch(channel, 64, polytouch); // stream interface
            break;
            
            // sysex using raw bytes (use shift + s)
        case 'S': {
            // send a pitch change to Part 1 of a MULTI on an Akai sampler
            // from http://troywoodfield.tripod.com/sysex.html
            //
            // do you have an S2000 to try?
            //
            // note: this is probably not as efficient as the next two methods
            //       since it sends only one byte at a time, instead of all
            //       at once
            //
            midiOut.sendMidiByte(MIDI_SYSEX);
            midiOut.sendMidiByte(0x47); // akai manufacturer code
            midiOut.sendMidiByte(0x00); // channel 0
            midiOut.sendMidiByte(0x42); // MULTI
            midiOut.sendMidiByte(0x48); // using an Akai S2000
            midiOut.sendMidiByte(0x00); // Part 1
            midiOut.sendMidiByte(0x00); // transpose
            midiOut.sendMidiByte(0x01); // Access Multi Parts
            midiOut.sendMidiByte(0x4B); // offset
            midiOut.sendMidiByte(0x00); // offset
            midiOut.sendMidiByte(0x01); // Field size = 1
            midiOut.sendMidiByte(0x00); // Field size = 1
            midiOut.sendMidiByte(0x04); // pitch value = 4
            midiOut.sendMidiByte(0x00); // offset
            midiOut.sendMidiByte(MIDI_SYSEX_END);
            
            // send again using a vector
            //
            // sends all bytes within one message
            //
            vector<unsigned char> sysexMsg;
            sysexMsg.push_back(MIDI_SYSEX);
            sysexMsg.push_back(0x47);
            sysexMsg.push_back(0x00);
            sysexMsg.push_back(0x42);
            sysexMsg.push_back(0x48);
            sysexMsg.push_back(0x00);
            sysexMsg.push_back(0x00);
            sysexMsg.push_back(0x01);
            sysexMsg.push_back(0x4B);
            sysexMsg.push_back(0x00);
            sysexMsg.push_back(0x01);
            sysexMsg.push_back(0x00);
            sysexMsg.push_back(0x04);
            sysexMsg.push_back(0x00);
            sysexMsg.push_back(MIDI_SYSEX_END);
            midiOut.sendMidiBytes(sysexMsg);
            
            // send again with the byte stream interface
            //
            // builds the message, then sends it on FinishMidi()
            //
            midiOut << StartMidi() << MIDI_SYSEX
            << 0x47 << 0x00 << 0x42 << 0x48 << 0x00 << 0x00 << 0x01
            << 0x4B << 0x00 << 0x01 << 0x00 << 0x04 << 0x00
            << MIDI_SYSEX_END << FinishMidi();
            break;
        }
            
            // print the port list
        case '?':
            midiOut.listOutPorts();
            break;
            
            // note off using raw bytes
        case ' ':
            // send with the byte stream interface, noteoff for note 60
            midiOut << StartMidi() << 0x80 << 0x3C << 0x40 << FinishMidi();
            break;
        case 'f':
            //            if (key == 'f') {
            toggleFullscreen();
            break;
        case 'g':
            //            }   else if (key == 32){
            tIndex = tIndex < themes.size()-1 ? tIndex+1 : 0;
            component->setTheme(themes[tIndex]);
            break;
            //            }
        default:
            // send a note off if the key is a letter or a number
            if(isalnum(key)) {
                note = ofMap(key, 48, 122, 0, 127);
                velocity = 0;
                midiOut << NoteOff(channel, note, velocity); // stream interface
            }
            break;
    }
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

//--------------------------------------------------------------
// ofxDatGui stuff
//--------------------------------------------------------------
void ofApp::ofxDatGuiSetup() {
    
    // instantiate and position the gui //
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );

    // add the optional header and footer //
    //gui->addTextInput("INPUT MESSAGE", "# AIR HARP #");
    gui->addFRM();
    gui->addBreak();
    
    // add a folder to group a few components together //
    ofxDatGuiFolder* setKeyFolder = gui->addFolder("SET KEY", ofColor::white);
    //setKeyFolder = new ofxDatGuiFolder("SET KEY", ofColor::white);
    setKeyFolder->addTextInput("KEY", "C");
    //setKeyFolder->attachItem(ofxDatGuiComponent *KeyTI);
    setKeyFolder->addTextInput("OCTAVE", "3");
    gui->addBreak();
    gui->addLabel("SCALE TYPE");
    /// add a dropdown menu //
    vector<std::string> options = {"CHROMATIC", "MAJOR", "MINOR", "PENTATONIC"};
    gui->addDropdown("SET SCALE TYPE", options);
    //setStringsFolder = new ofxDatGuiFolder("STRINGS", ofColor::green);
    ofxDatGuiFolder* setStringsFolder = gui->addFolder("STRINGS", ofColor::green);
    //noStringsTI = new ofxDatGuiTextInput("NO STRINGS", "30");
    setStringsFolder->addTextInput("NO STRINGS", "12");
    setStringsFolder->addTextInput("DIAMETER", "10");
    setStringsFolder->addTextInput("SPACING", "50");
    vector<std::string> options2 = {"TWO HANDS", "BASS PEDALS"};
    gui->addDropdown("SET PLAY MODE", options2);
    gui->addFooter();
    gui->collapse();
    
    gui->onDropdownEvent(this, &ofApp::onDropdownEvent);
    gui->onTextInputEvent(this, &ofApp::onTextInputEvent);
    
    
    
//    int x = 660;
//    int y = 100;
//    int p = 40;
//    ofSetWindowPosition(0, 0);
//    ofSetWindowShape(1400, 900);
//
//    component = new ofxDatGuiButton("Re-initialise Sensor");
//    component->setPosition(x, y);
//    component->onButtonEvent(this, &ofApp::onButtonEvent);
//    components.push_back(component);
//
//    y += component->getHeight() + p;
//    component = new ofxDatGuiToggle("Connect Apple MIDI", true);
//    component->setPosition(x, y);
//    component->onToggleEvent(this, &ofApp::onToggleEvent);
//    components.push_back(component);
//
//    y += component->getHeight() + p;
//    component = new ofxDatGuiToggle("Toggle Ease", false);
//    component->setPosition(x, y);
//    component->onToggleEvent(this, &ofApp::onToggleEvent);
//    components.push_back(component);
//
//    y += component->getHeight();
//    component = new ofxDatGuiSlider("Ease Duration", 0, 1000, 250);
//    component->setPosition(x, y);
//    component->onSliderEvent(this, &ofApp::onSliderEvent);
//    components.push_back(component);
//
////    y += component->getHeight();
////    ofxDatGuiDropdown* dropdown;
////    vector<string> options = {"Linear", "Quadratic", "Cubic", "Quartic", "Quintic", "Sinusoidal", "Exponential", "Circular", "Elastic", "Back", "Bounce"};
////    dropdown = new ofxDatGuiDropdown("ease type", options);
////    dropdown->setPosition(x, y);
////    dropdown->expand();
////    dropdown->onDropdownEvent(this, &ofApp::onDropdownEvent);
////    components.push_back(dropdown);
//
//    //    y += component->getHeight() + p;
//    //    component = new ofxDatGuiWaveMonitor("wave\nmonitor", 3, .5);
//    //    component->setPosition(x, y);
//    //    components.push_back(component);
//
//    y += component->getHeight() + p + 300;
//    component = new ofxDatGuiMatrix("matrix", 21, true);
//    component->setPosition(x, y);
//    component->onMatrixEvent(this, &ofApp::onMatrixEvent);
//    components.push_back(component);
//
//    //    y += component->getHeight() + p;
//    //    component = new ofxDatGuiTextInput("text input", "# open frameworks #");
//    //    component->setPosition(x, y);
//    //    component->onTextInputEvent(this, &ofApp::onTextInputEvent);
//    //    components.push_back(component);
//
//    //    y += component->getHeight() + p;
//    //    component = new ofxDatGuiColorPicker("color picker", ofColor::fromHex(0xFFD00B));
//    //    component->setPosition(x, y);
//    //    component->onColorPickerEvent(this, &ofApp::onColorPickerEvent);
//    //    components.push_back(component);
//
//    y = 100;
//    x += component->getWidth() + p+60;
//
//    component = new ofxDatGuiFRM();
//    component->setPosition(x, y);
//    components.push_back(component);
//
//
//
//    y += component->getHeight() + p;
//    // capture the plotter in a variable so we can feed it values later //
//    plotter = new ofxDatGuiValuePlotter("MIDI Note No", 0, 127);
//    plotter->setSpeed(0.01f);
//    plotter->setDrawMode(ofxDatGuiGraph::LINES);
//    component = plotter;
//    component->setPosition(x, y);
//    components.push_back(component);
//
//    y += component->getHeight();
//    // capture the plotter in a variable so we can feed it values later //
//    plotter2 = new ofxDatGuiValuePlotter("MIDI Pitch Bend", -16384, +16384);
//    plotter2->setSpeed(0.01f);
//    plotter2->setDrawMode(ofxDatGuiGraph::LINES);
//    component = plotter2;
//    component->setPosition(x, y);
//    components.push_back(component);
//
//    //    y += component->getHeight();
//    //    slider = new ofxDatGuiSlider("multiplier", 0, 1, .1);
//    //    component = slider;
//    //    component->setPosition(x, y);
//    //    component->onSliderEvent(this, &ofApp::onSliderEvent);
//    //    components.push_back(component);
//
//    y += component->getHeight();
//    slider2 = new ofxDatGuiSlider("sweep speed", 0.01, 1, 0.3);
//    component = slider2;
//    component->setPosition(x, y);
//    component->onSliderEvent(this, &ofApp::onSliderEvent);
//    components.push_back(component);
//
////    // add a dropdown to select between the four draw modes //
////    y += component->getHeight();
////    vector<string> drawModes = {"lines", "filled", "points", "outline"};
////    dropdown2 = new ofxDatGuiDropdown("draw mode", drawModes);
////    dropdown2->setPosition(x, y);
////    dropdown2->expand();
////    dropdown2->onDropdownEvent(this, &ofApp::onDropdownEvent);
////    components.push_back(dropdown2);
//
//
//
//
//
//
//
//
//    //    y += component->getHeight() + p;
//    //    component = new ofxDatGui2dPad("2d pad");
//    //    component->setPosition(x, y);
//    //    component->on2dPadEvent(this, &ofApp::on2dPadEvent);
//    //    components.push_back(component);
//
//    //  for(int i=0; i<components.size(); i++) components[i]->setOpacity(.25);
//
//
//    // launch the app //
//    mFullscreen = false;
//    refreshWindow();
    
}

//--------------------------------------------------------------
void ofApp::ofxDatGuiUpdate() {
    
    //  append a random value to the plotter within its range //
    //    plotter->setValue(ofRandom(plotter->getMin()*.5, plotter->getMax()*.5));
    //float v = ofRandom(plotter->getMin(), plotter->getMax());
    //plotter->setValue(v);
    
    //float v1 = (1+sin(ang1+=.02f))/2*25+60;
//    plotter->setValue(p1Value);
//    plotter->setSpeed(slider2->getValue());
//    plotter2->setValue(p2Value);
//    plotter2->setSpeed(slider2->getValue());
    
    
    //plotter->setValue(midiMessage.pitch);
    //for(int i=0; i<components.size(); i++) components[i]->update();
    
    //    p2->setValue(v2 * p1->getRange());
    //    p2->setSpeed(gui->getSlider("sweep speed")->getValue());
    
}


/*
 event listeners
 */

//--------------------------------------------------------------
void ofApp::onTextInputEvent(ofxDatGuiTextInputEvent e)
{
    cout << "onTextInputEvent: " << e.target->getLabel() << " " << e.target->getText() << endl;
    
    if (e.target->getLabel() == "NO STRINGS")
    {
        std::string text = e.target->getText();
        int value = ofToInt(text);
        //cout << "value: " << value << endl;
        noStrings = value;
        setupStrings(noStrings);
        setupScale(fundamentalS, scaleType);
    }
    
    if (e.target->getLabel() == "DIAMETER")
    {
        std::string text = e.target->getText();
        int value = ofToInt(text);
        //cout << "value: " << value << endl;
        diameter = value;
        setupStrings(noStrings);
        setupScale(fundamentalS, scaleType);
    }
    
    if (e.target->getLabel() == "SPACING")
    {
        std::string text = e.target->getText();
        int value = ofToInt(text);
        //cout << "value: " << value << endl;
        sSpacing = value;
        setupStrings(noStrings);
        setupScale(fundamentalS, scaleType);
    }
    
    if (e.target->getLabel() == "KEY")
    {
        keyText = e.target->getText();
        for (int i=0; i<12; i++)
        {
            if (keyText == scaleKey[i])
            {
                keyNo = i;
                //cout << "keyNo: " << keyNo << endl;
            }
        }
        fundamentalS = 12 + 12*octave + keyNo;
        setupScale(fundamentalS, scaleType);
        //cout << "fundamentalS: " << fundamentalS << endl;
    }
    
    if (e.target->getLabel() == "OCTAVE")
    {
        std::string text = e.target->getText();
        int value = ofToInt(text);
        octave = value;
        fundamentalS = 12 + 12*octave + keyNo;
        setupScale(fundamentalS, scaleType);
        //cout << "fundamentalS: " << fundamentalS << endl;
    }
    
}

//--------------------------------------------------------------
void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e)
{
    cout << "onDropdownEvent: " << e.child << endl;
    
    if (e.target->getLabel() == "CHROMATIC")
    {
        scaleType = 0;
        setupScale(fundamentalS, scaleType);
    }
    else if (e.target->getLabel() == "MAJOR")
    {
        scaleType = 1;
        setupScale(fundamentalS, scaleType);
    }
    else if (e.target->getLabel() == "MINOR")
    {
        scaleType = 2;
        setupScale(fundamentalS, scaleType);
    }
    else if (e.target->getLabel() == "PENTATONIC")
    {
        scaleType = 3;
        setupScale(fundamentalS, scaleType);
    }
    
    if (e.target->getLabel() == "TWO HANDS")
    {
        playMode = 0;
    }
    else if (e.target->getLabel() == "BASS PEDALS")
    {
        playMode = 1;
    }
    
    //    //cout << e.parent-> << endl;
    //
    //    //if (e.target == "0x1013273b0")
    //    //{
    //    // ease type
    //    //if (e.child == 0) { // Linear
    //
    //
    //    if (e.target->getLabel() == "Linear")
    //    {
    //        midiOut.sendNoteOn(1, 3, 10);
    //    }
    //    else if (e.target->getLabel() == "Quadratic")
    //    {
    //        midiOut.sendNoteOn(1, 3, 20);
    //    }
    //    else if (e.target->getLabel() == "Cubic")
    //    {
    //        midiOut.sendNoteOn(1, 3, 30);
    //    }
    //    else if (e.target->getLabel() == "Quartic")
    //    {
    //        midiOut.sendNoteOn(1, 3, 40);
    //    }
    //    else if (e.target->getLabel() == "Quintic")
    //    {
    //        midiOut.sendNoteOn(1, 3, 50);
    //    }
    //    else if (e.target->getLabel() == "Sinusoidal")
    //    {
    //        midiOut.sendNoteOn(1, 3, 60);
    //    }
    //    else if (e.target->getLabel() == "Exponential")
    //    {
    //        midiOut.sendNoteOn(1, 3, 70);
    //    }
    //    else if (e.target->getLabel() == "Circular")
    //    {
    //        midiOut.sendNoteOn(1, 3, 80);
    //    }
    //    else if (e.target->getLabel() == "Elastic")
    //    {
    //        midiOut.sendNoteOn(1, 3, 90);
    //    }
    //    else if (e.target->getLabel() == "Back")
    //    {
    //        midiOut.sendNoteOn(1, 3, 100);
    //    }
    //    else if (e.target->getLabel() == "Bounce")
    //    {
    //        midiOut.sendNoteOn(1, 3, 110);
    //    }
    //    //}
    //
    //    if (e.target->getLabel() == "lines")
    //    {
    //        plotter->setDrawMode(ofxDatGuiGraph::LINES);
    //        plotter2->setDrawMode(ofxDatGuiGraph::LINES);
    //        dropdown2->setLabel("drawing mode : lines");
    //    }
    //    else if (e.target->getLabel() == "filled")
    //    {
    //        plotter->setDrawMode(ofxDatGuiGraph::FILLED);
    //        plotter2->setDrawMode(ofxDatGuiGraph::FILLED);
    //        dropdown2->setLabel("drawing mode : filled");
    //    }
    //    else if (e.target->getLabel() == "points")
    //    {
    //        plotter->setDrawMode(ofxDatGuiGraph::POINTS);
    //        plotter2->setDrawMode(ofxDatGuiGraph::POINTS);
    //        dropdown2->setLabel("drawing mode : points");
    //    }
    //    else if (e.target->getLabel() == "outline")
    //    {
    //        plotter->setDrawMode(ofxDatGuiGraph::OUTLINE);
    //        plotter2->setDrawMode(ofxDatGuiGraph::OUTLINE);
    //        dropdown2->setLabel("drawing mode : outline");
    //    }
    
    
    
    ////    if (component->getLabel() == "draw mode")
    ////    {
    //        switch (e.child) {
    //            case (int)ofxDatGuiGraph::LINES :
    //                plotter->setDrawMode(ofxDatGuiGraph::LINES);
    //                plotter2->setDrawMode(ofxDatGuiGraph::LINES);
    //                dropdown2->setLabel("drawing mode : lines");
    //                break;
    //            case (int)ofxDatGuiGraph::FILLED :
    //                plotter->setDrawMode(ofxDatGuiGraph::FILLED);
    //                plotter2->setDrawMode(ofxDatGuiGraph::FILLED);
    //                dropdown2->setLabel("drawing mode : filled");
    //                break;
    //            case (int)ofxDatGuiGraph::POINTS :
    //                plotter->setDrawMode(ofxDatGuiGraph::POINTS);
    //                plotter2->setDrawMode(ofxDatGuiGraph::POINTS);
    //                dropdown2->setLabel("drawing mode : points");
    //                break;
    //            case (int)ofxDatGuiGraph::OUTLINE :
    //                plotter->setDrawMode(ofxDatGuiGraph::OUTLINE);
    //                plotter2->setDrawMode(ofxDatGuiGraph::OUTLINE);
    //                dropdown2->setLabel("drawing mode : outline");
    //                break;
    //        }
    ////    }
    
}


////--------------------------------------------------------------
//void ofApp::onButtonEvent(ofxDatGuiButtonEvent e)
//{
//    cout << "onButtonEvent: " << e.target->getLabel() << endl;
//
//    if (e.target->getLabel() == "Re-initialise Sensor") {
//        midiOut.sendNoteOn(1, 0, 127);
//    }
//
//}

////--------------------------------------------------------------
//void ofApp::onToggleEvent(ofxDatGuiToggleEvent e)
//{
//    cout << "onToggleEvent: " << e.target->getLabel() << "::" <<  e.target->getChecked() << endl;
//
//    if ((e.target->getLabel() == "Toggle Ease") && (e.target->getChecked() == true)) {
//        midiOut.sendNoteOn(1, 1, 127);
//    }
//
//    if ((e.target->getLabel() == "Toggle Ease") && (e.target->getChecked() == false)) {
//        midiOut.sendNoteOff(1, 1, 0);
//    }
//
//    if ((e.target->getLabel() == "Connect Apple MIDI") && (e.target->getChecked() == true)) {
//        midiOut.sendNoteOn(1, 4, 127);
//    }
//
//    if ((e.target->getLabel() == "Connect Apple MIDI") && (e.target->getChecked() == false)) {
//        midiOut.sendNoteOff(1, 4, 0);
//    }
//
//
//}

////--------------------------------------------------------------
//void ofApp::onSliderEvent(ofxDatGuiSliderEvent e)
//{
//    cout << "onSliderEvent: " << e.value << "::" << e.scale << endl;
//
//    if (e.target->getLabel() == "Ease Duration") {
//
//        velocity = int(e.scale*127.0);
//        midiOut.sendNoteOn(1, 2, velocity);
//
//    }
//
//
//}


////--------------------------------------------------------------
//void ofApp::onMatrixEvent(ofxDatGuiMatrixEvent e)
//{
//    cout << "onMatrixEvent: " << e.child << "::" << e.enabled << endl;
//}

////--------------------------------------------------------------
//void ofApp::onColorPickerEvent(ofxDatGuiColorPickerEvent e)
//{
//    cout << "onColorPickerEvent: " << e.color << endl;
//}

////--------------------------------------------------------------
//void ofApp::on2dPadEvent(ofxDatGui2dPadEvent e)
//{
//    cout << "on2dPadEvent: " << e.x << "::" << e.y << endl;
//}



//--------------------------------------------------------------
void ofApp::toggleFullscreen()
{
//    mFullscreen = !mFullscreen;
//    components->getToggle("toggle fullscreen")->setChecked(mFullscreen);
//    refreshWindow();
}

//--------------------------------------------------------------
void ofApp::refreshWindow()
{
//    ofSetFullscreen(mFullscreen);
//    if (!mFullscreen) {
//        int width = ofGetScreenWidth() * .8;
//        int height = ofGetScreenHeight() * .8;
//        ofSetWindowShape(width, height);
//        ofSetWindowPosition((ofGetScreenWidth()/2)-(width/2), 0);
//    }
}
