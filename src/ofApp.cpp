#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::exit()
{
    if(dmxDevice->isOpen()) {
    memset( dmxData, 0, DMX_DATA_LENGTH );
    dmxDevice->writeDmx( dmxData, DMX_DATA_LENGTH );
    dmxDevice->close();
    }
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(44);
    ofBackground(38);
    ofSetEscapeQuitsApp(true);

    //zero our DMX value array
    memset( dmxData, 0, DMX_DATA_LENGTH );

    //open the device
    dmxDevice = ofxGenericDmx::openFirstDevice();
    if ( dmxDevice == 0 ) ofLogError() << "No Enttec Device Found";
    else {
        if(dmxDevice->isOpen()) {
            ofLogNotice() << "DMX device opened";
        } else {
            ofLogError() << "DMX device unable to open";
        }
    }

    numLights = 6;
    numSequences = 4;
    freq = 32;

    ofLogNotice() << "ofxGenericDmx addon version: " << ofxGenericDmx::VERSION_MAJOR << "." << ofxGenericDmx::VERSION_MINOR;

    timeline.setup();
    timeline.setDurationInSeconds(60*8);

    for(int i = 1;i <= numSequences;i++) {
        timeline.addFlags("SEQ"+ofToString(i)+"_FAN1");
        timeline.addFlags("SEQ"+ofToString(i)+"_FAN2");
        timeline.addFlags("SEQ"+ofToString(i)+"_STROBE");
    }



    timeline.enableSnapToOtherKeyframes(false);
    timeline.setLoopType(OF_LOOP_NONE);

    ofAddListener(timeline.events().bangFired, this, &ofApp::bangFired);
}

//--------------------------------------------------------------
void ofApp::bangFired(ofxTLBangEventArgs& args){
    ofLogVerbose() << args.track->getName() << " fired: " << args.flag;

    if(args.track->getName() == "SEQ4_STROBE")
    {
        freq = stof(args.flag);
        cout << "freq=" << freq << endl;
    }
}


//--------------------------------------------------------------
void ofApp::update(){

    //asign our colors to the right dmx channels
    //int val = (sin(/500.0f) +1.0f) * 128.0f;

    int val = sin((ofGetElapsedTimeMillis()/200.0f)*(float)freq) > 0 ? 255:0;

    //cout << "val = " << val << endl;


    for(int channel=1; channel <= numLights*2; channel+=2) {
        dmxData[channel] = val;
        dmxData[channel + 1] = 11;
    }

    //force first byte to zero (it is not a channel but DMX type info - start code)
    dmxData[0] = 0;

    if ( ! dmxDevice || ! dmxDevice->isOpen() ) {
        ofLogVerbose() << "Not updating, enttec device is not open.";
    }
    else{
        //send the data to the dmx interface
        dmxDevice->writeDmx( dmxData, DMX_DATA_LENGTH );
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    //ofSetColor(0);
    //ofDrawBitmapString("channels = 1 3 5 7 9 11",20,20);

    timeline.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

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
