#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetVerticalSync(true);
    ofBackground(255, 255, 255);
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    // print input ports to console
    midiIn.listInPorts();
    
    // open port by number (you may need to change this)
    midiIn.openPort(1);
    //midiIn.openPort("IAC Pure Data In");    // by name
    midiIn.openPort("Network Session 1");
    //midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port
    
    // don't ignore sysex, timing, & active sense messages,
    // these are ignored by default
    midiIn.ignoreTypes(false, false, false);
    
    // add ofApp as a listener
    midiIn.addListener(this);
    
    // print received messages to the console
    midiIn.setVerbose(true);
    
    // print the available output ports to the console
    midiOut.listOutPorts();
    
    // connect
    midiOut.openPort(0); // by number
    //midiOut.openPort("IAC Driver Pure Data In"); // by name
    midiOut.openPort("Network Session 1"); // by name
    //midiOut.openVirtualPort("ofxMidiOut"); // open a virtual port
    
    channel = 1;
    currentPgm = 0;
    note = 0;
    velocity = 0;
    pan = 0;
    bend = 0;
    touch = 0;
    polytouch = 0;
    
    ofxDatGuiSetup();
    
}

//--------------------------------------------------------------
void ofApp::update() {
    
    ofxDatGuiUpdate();
    
}

//--------------------------------------------------------------
void ofApp::draw() {
    
    /// ofxDatGui
    for(int i=0; i<components.size(); i++) components[i]->draw();
    
    /// ofxMidi
    for(unsigned int i = 0; i < midiMessages.size(); ++i) {
        
        ofxMidiMessage &message = midiMessages[i];
        int x = 10;
        int y = i*40 + 40;
        
        // draw the last recieved message contents to the screen,
        // this doesn't print all the data from every status type
        // but you should get the general idea
        stringstream text;
        text << ofxMidiMessage::getStatusString(message.status);
        while(text.str().length() < 16) { // pad status width
            text << " ";
        }
        
        ofSetColor(127);
        if(message.status < MIDI_SYSEX) {
            text << "chan: " << message.channel;
            if(message.status == MIDI_CONTROL_CHANGE) {
                text << "\tctl: " << message.control;
                ofDrawRectangle(x + ofGetWidth()*0.2, y + 12,
                                ofMap(message.control, 0, 127, 0, ofGetWidth()*0.2), 10);
            }
            else if(message.status == MIDI_PITCH_BEND) {
                text << "\tval: " << message.value;
                ofDrawRectangle(x + ofGetWidth()*0.2, y + 12,
                                ofMap(message.value, 0, MIDI_MAX_BEND, 0, ofGetWidth()*0.2), 10);
            }
            else {
                text << "\tpitch: " << message.pitch;
                ofDrawRectangle(x + ofGetWidth()*0.2, y + 12,
                                ofMap(message.pitch, 0, 127, 0, ofGetWidth()*0.2), 10);
                
                text << "\tvel: " << message.velocity;
                ofDrawRectangle(x + (ofGetWidth()*0.2 * 2), y + 12,
                                ofMap(message.velocity, 0, 127, 0, ofGetWidth()*0.2), 10);
            }
            text << " "; // pad for delta print
        }
        
        text << "delta: " << message.deltatime;
        ofSetColor(0);
        ofDrawBitmapString(text.str(), x, y);
        text.str(""); // clear
    }
    
    // let's see something
    ofSetColor(0);
    stringstream text;
    text << "connected to port " << midiOut.getPort()
    << " \"" << midiOut.getName() << "\"" << endl
    << "is virtual?: " << midiOut.isVirtual() << endl << endl
    << "sending to channel " << channel << endl << endl
    << "current program: " << currentPgm << endl << endl
    << "note: " << note << endl
    << "velocity: " << velocity << endl
    << "pan: " << pan << endl
    << "bend: " << bend << endl
    << "touch: " << touch << endl
    << "polytouch: " << polytouch;
    ofDrawBitmapString(text.str(), 20, 600);
    
}

//--------------------------------------------------------------
void ofApp::exit() {
    
    // clean up
    midiIn.closePort();
    midiIn.removeListener(this);
    // clean up
    midiOut.closePort();
    
}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {
    
    midiMessage = msg;
    
    // add the latest message to the message queue
    midiMessages.push_back(msg);
    
    // remove any old messages if we have too many
    while(midiMessages.size() > maxMessages) {
        midiMessages.erase(midiMessages.begin());
    }
    
    midiNoteNo = msg.pitch;
    if ((midiNoteNo >= 60) && (midiNoteNo<=84))
    {
        p1Value = midiNoteNo;
    }
    
    if(msg.status == MIDI_PITCH_BEND)
    {
        p2Value = msg.value;
    }
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    switch(key) {
        case '?':
            midiIn.listInPorts();
            break;
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
        case '/':
            midiOut.listOutPorts();
            break;
            
            // note off using raw bytes
        case ' ':
            // send with the byte stream interface, noteoff for note 60
            midiOut << StartMidi() << 0x80 << 0x3C << 0x40 << FinishMidi();
            break;
            
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
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    
    //    // x pos controls the pan (ctl = 10)
    //    pan = ofMap(x, 0, ofGetWidth(), 0, 127);
    //    midiOut.sendControlChange(channel, 10, pan);
    //
    //    // y pos controls the pitch bend
    //    bend = ofMap(y, 0, ofGetHeight(), 0, MIDI_MAX_BEND);
    //    midiOut.sendPitchBend(channel, bend);
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
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
    
    int x = 660;
    int y = 100;
    int p = 20;
    ofSetWindowPosition(0, 0);
    ofSetWindowShape(1400, 900);
    
    component = new ofxDatGuiButton("Re-initialise Sensor");
    component->setPosition(x, y);
    component->onButtonEvent(this, &ofApp::onButtonEvent);
    components.push_back(component);
    
    y += component->getHeight() + p;
    component = new ofxDatGuiToggle("Connect Apple MIDI", true);
    component->setPosition(x, y);
    component->onToggleEvent(this, &ofApp::onToggleEvent);
    components.push_back(component);
    
    y += component->getHeight() + p;
    component = new ofxDatGuiToggle("Toggle Ease", false);
    component->setPosition(x, y);
    component->onToggleEvent(this, &ofApp::onToggleEvent);
    components.push_back(component);
    
    y += component->getHeight();
    component = new ofxDatGuiSlider("Ease Duration", 0, 1000, 250);
    component->setPosition(x, y);
    component->onSliderEvent(this, &ofApp::onSliderEvent);
    components.push_back(component);
    
    y += component->getHeight();
    ofxDatGuiDropdown* dropdownEaseType;
    vector<string> optionsEaseType = {"Linear", "Quadratic", "Cubic", "Quartic", "Quintic", "Sinusoidal", "Exponential", "Circular", "Elastic", "Back", "Bounce"};
    dropdownEaseType = new ofxDatGuiDropdown("ease type", optionsEaseType);
    dropdownEaseType->setPosition(x, y);
    dropdownEaseType->expand();
    dropdownEaseType->onDropdownEvent(this, &ofApp::onDropdownEvent);
    components.push_back(dropdownEaseType);
    
    //    y += component->getHeight() + p;
    //    component = new ofxDatGuiWaveMonitor("wave\nmonitor", 3, .5);
    //    component->setPosition(x, y);
    //    components.push_back(component);
    
    y += component->getHeight()+p+300;
    component = new ofxDatGuiTextInput("KEY", "C");
    component->setPosition(x, y);
    component->onTextInputEvent(this, &ofApp::onTextInputEvent);
    components.push_back(component);
    
    y += component->getHeight();
    component = new ofxDatGuiTextInput("OCTAVE", "3");
    component->setPosition(x, y);
    component->onTextInputEvent(this, &ofApp::onTextInputEvent);
    components.push_back(component);

    /// add a dropdown menu //
    y += component->getHeight();
    ofxDatGuiDropdown* dropdownScale;
    vector<std::string> optionsScale = {"CHROMATIC", "MAJOR", "MINOR", "PENTATONIC"};
    dropdownScale = new ofxDatGuiDropdown("SET SCALE TYPE", optionsScale);
    dropdownScale->setPosition(x, y);
    dropdownScale->expand();
    dropdownScale->onDropdownEvent(this, &ofApp::onDropdownEvent);
    components.push_back(dropdownScale);
    
//    component = new ofxDatGuiMatrix("matrix", 21, true);
//    component->setPosition(x, y);
//    component->onMatrixEvent(this, &ofApp::onMatrixEvent);
//    components.push_back(component);
    
    //    y += component->getHeight() + p;
    //    component = new ofxDatGuiTextInput("text input", "# open frameworks #");
    //    component->setPosition(x, y);
    //    component->onTextInputEvent(this, &ofApp::onTextInputEvent);
    //    components.push_back(component);
    
    //    y += component->getHeight() + p;
    //    component = new ofxDatGuiColorPicker("color picker", ofColor::fromHex(0xFFD00B));
    //    component->setPosition(x, y);
    //    component->onColorPickerEvent(this, &ofApp::onColorPickerEvent);
    //    components.push_back(component);
    
    y = 100;
    x += component->getWidth() + p+60;
    
    component = new ofxDatGuiFRM();
    component->setPosition(x, y);
    components.push_back(component);
    
    
    
    y += component->getHeight() + p;
    // capture the plotter in a variable so we can feed it values later //
    plotter = new ofxDatGuiValuePlotter("MIDI Note No", 0, 127);
    plotter->setSpeed(0.01f);
    plotter->setDrawMode(ofxDatGuiGraph::LINES);
    component = plotter;
    component->setPosition(x, y);
    components.push_back(component);
    
    y += component->getHeight();
    // capture the plotter in a variable so we can feed it values later //
    plotter2 = new ofxDatGuiValuePlotter("MIDI Pitch Bend", -16384, +16384);
    plotter2->setSpeed(0.01f);
    plotter2->setDrawMode(ofxDatGuiGraph::LINES);
    component = plotter2;
    component->setPosition(x, y);
    components.push_back(component);
    
    //    y += component->getHeight();
    //    slider = new ofxDatGuiSlider("multiplier", 0, 1, .1);
    //    component = slider;
    //    component->setPosition(x, y);
    //    component->onSliderEvent(this, &ofApp::onSliderEvent);
    //    components.push_back(component);
    
    y += component->getHeight();
    slider2 = new ofxDatGuiSlider("sweep speed", 0.01, 1, 0.3);
    component = slider2;
    component->setPosition(x, y);
    component->onSliderEvent(this, &ofApp::onSliderEvent);
    components.push_back(component);
    
    // add a dropdown to select between the four draw modes //
    y += component->getHeight();
    ofxDatGuiDropdown* dropdownDrawModes;
    vector<string> optionsDrawModes = {"lines", "filled", "points", "outline"};
    dropdownDrawModes = new ofxDatGuiDropdown("draw mode", optionsDrawModes);
    dropdownDrawModes->setPosition(x, y);
    dropdownDrawModes->expand();
    dropdownDrawModes->onDropdownEvent(this, &ofApp::onDropdownEvent);
    components.push_back(dropdownDrawModes);
    
    
    
    
    
    
    
    
    //    y += component->getHeight() + p;
    //    component = new ofxDatGui2dPad("2d pad");
    //    component->setPosition(x, y);
    //    component->on2dPadEvent(this, &ofApp::on2dPadEvent);
    //    components.push_back(component);
    
    //  for(int i=0; i<components.size(); i++) components[i]->setOpacity(.25);
    
}

//--------------------------------------------------------------
void ofApp::ofxDatGuiUpdate() {
    
    //  append a random value to the plotter within its range //
    //    plotter->setValue(ofRandom(plotter->getMin()*.5, plotter->getMax()*.5));
    //float v = ofRandom(plotter->getMin(), plotter->getMax());
    //plotter->setValue(v);
    
    //float v1 = (1+sin(ang1+=.02f))/2*25+60;
    plotter->setValue(p1Value);
    plotter->setSpeed(slider2->getValue());
    plotter2->setValue(p2Value);
    plotter2->setSpeed(slider2->getValue());
    
    
    //plotter->setValue(midiMessage.pitch);
    for(int i=0; i<components.size(); i++) components[i]->update();
    
    //    p2->setValue(v2 * p1->getRange());
    //    p2->setSpeed(gui->getSlider("sweep speed")->getValue());
    
}


/*
 event listeners
 */

//--------------------------------------------------------------
void ofApp::onButtonEvent(ofxDatGuiButtonEvent e)
{
    cout << "onButtonEvent: " << e.target->getLabel() << endl;
    
    if (e.target->getLabel() == "Re-initialise Sensor") {
        midiOut.sendNoteOn(1, 0, 127);
    }
    
}

//--------------------------------------------------------------
void ofApp::onToggleEvent(ofxDatGuiToggleEvent e)
{
    cout << "onToggleEvent: " << e.target->getLabel() << "::" <<  e.target->getChecked() << endl;
    
    if ((e.target->getLabel() == "Toggle Ease") && (e.target->getChecked() == true)) {
        midiOut.sendNoteOn(1, 1, 127);
    }
    
    if ((e.target->getLabel() == "Toggle Ease") && (e.target->getChecked() == false)) {
        midiOut.sendNoteOff(1, 1, 0);
    }
    
    if ((e.target->getLabel() == "Connect Apple MIDI") && (e.target->getChecked() == true)) {
        midiOut.sendNoteOn(1, 4, 127);
    }
    
    if ((e.target->getLabel() == "Connect Apple MIDI") && (e.target->getChecked() == false)) {
        midiOut.sendNoteOff(1, 4, 0);
    }
    
    
}

//--------------------------------------------------------------
void ofApp::onSliderEvent(ofxDatGuiSliderEvent e)
{
    cout << "onSliderEvent: " << e.value << "::" << e.scale << endl;
    
    if (e.target->getLabel() == "Ease Duration") {
        
        velocity = int(e.scale*127.0);
        midiOut.sendNoteOn(1, 2, velocity);
        
    }
    
    
}

//--------------------------------------------------------------
void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e)
{
    cout << "onDropdownEvent: " << e.child << endl;
    
    //cout << e.parent-> << endl;
    
    //if (e.target == "0x1013273b0")
    //{
        // ease type
        //if (e.child == 0) { // Linear
            
            
        if (e.target->getLabel() == "Linear")
        {
            midiOut.sendNoteOn(1, 3, 10);
        }
        else if (e.target->getLabel() == "Quadratic")
        {
            midiOut.sendNoteOn(1, 3, 20);
        }
        else if (e.target->getLabel() == "Cubic")
        {
            midiOut.sendNoteOn(1, 3, 30);
        }
        else if (e.target->getLabel() == "Quartic")
        {
            midiOut.sendNoteOn(1, 3, 40);
        }
        else if (e.target->getLabel() == "Quintic")
        {
            midiOut.sendNoteOn(1, 3, 50);
        }
        else if (e.target->getLabel() == "Sinusoidal")
        {
            midiOut.sendNoteOn(1, 3, 60);
        }
        else if (e.target->getLabel() == "Exponential")
        {
            midiOut.sendNoteOn(1, 3, 70);
        }
        else if (e.target->getLabel() == "Circular")
        {
            midiOut.sendNoteOn(1, 3, 80);
        }
        else if (e.target->getLabel() == "Elastic")
        {
            midiOut.sendNoteOn(1, 3, 90);
        }
        else if (e.target->getLabel() == "Back")
        {
            midiOut.sendNoteOn(1, 3, 100);
        }
        else if (e.target->getLabel() == "Bounce")
        {
            midiOut.sendNoteOn(1, 3, 110);
        }
    //}
    
    if (e.target->getLabel() == "lines")
    {
        plotter->setDrawMode(ofxDatGuiGraph::LINES);
        plotter2->setDrawMode(ofxDatGuiGraph::LINES);
        dropdownDrawModes->setLabel("drawing mode : lines");
    }
    else if (e.target->getLabel() == "filled")
    {
        plotter->setDrawMode(ofxDatGuiGraph::FILLED);
        plotter2->setDrawMode(ofxDatGuiGraph::FILLED);
        dropdownDrawModes->setLabel("drawing mode : filled");
    }
    else if (e.target->getLabel() == "points")
    {
        plotter->setDrawMode(ofxDatGuiGraph::POINTS);
        plotter2->setDrawMode(ofxDatGuiGraph::POINTS);
        dropdownDrawModes->setLabel("drawing mode : points");
    }
    else if (e.target->getLabel() == "outline")
    {
        plotter->setDrawMode(ofxDatGuiGraph::OUTLINE);
        plotter2->setDrawMode(ofxDatGuiGraph::OUTLINE);
        dropdownDrawModes->setLabel("drawing mode : outline");
    }
    
    
    if (e.target->getLabel() == "CHROMATIC")
    {
        scaleType = 0;
        //setupScale(fundamentalS, scaleType);
        midiOut.sendNoteOn(1, 5, 10);
    }
    else if (e.target->getLabel() == "MAJOR")
    {
        scaleType = 1;
        //setupScale(fundamentalS, scaleType);
        midiOut.sendNoteOn(1, 5, 20);
    }
    else if (e.target->getLabel() == "MINOR")
    {
        scaleType = 2;
        //setupScale(fundamentalS, scaleType);
        midiOut.sendNoteOn(1, 5, 30);
    }
    else if (e.target->getLabel() == "PENTATONIC")
    {
        scaleType = 3;
        //setupScale(fundamentalS, scaleType);
        midiOut.sendNoteOn(1, 5, 40);
    }
    
    
    
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

//--------------------------------------------------------------
void ofApp::onTextInputEvent(ofxDatGuiTextInputEvent e)
{
    cout << "onTextInputEvent: " << e.target->getLabel() << " " << e.target->getText() << endl;
    
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
        fundamental = 12 + 12*octave + keyNo;
        //setupScale(fundamental, scaleType);
        cout << "fundamental: " << fundamental << endl;
        midiOut.sendNoteOn(1, 6, fundamental);
    }
    
    if (e.target->getLabel() == "OCTAVE")
    {
        std::string text = e.target->getText();
        int value = ofToInt(text);
        octave = value;
        fundamental = 12 + 12*octave + keyNo;
        //setupScale(fundamental, scaleType);
        cout << "fundamental: " << fundamental << endl;
        midiOut.sendNoteOn(1, 6, fundamental);
    }
}

//--------------------------------------------------------------
void ofApp::onMatrixEvent(ofxDatGuiMatrixEvent e)
{
    cout << "onMatrixEvent: " << e.child << "::" << e.enabled << endl;
}

//--------------------------------------------------------------
void ofApp::onColorPickerEvent(ofxDatGuiColorPickerEvent e)
{
    cout << "onColorPickerEvent: " << e.color << endl;
}

//--------------------------------------------------------------
void ofApp::on2dPadEvent(ofxDatGui2dPadEvent e)
{
    cout << "on2dPadEvent: " << e.x << "::" << e.y << endl;
}


