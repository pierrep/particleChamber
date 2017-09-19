#include "ofApp.h"

#ifdef TARGET_RASPBERRY_PI
#include <wiringPi.h>
#define RELAY1 0
#define RELAY2 2
#define PIR1 1
#define PIR2 3
#endif

//--------------------------------------------------------------
void ofApp::exit()
{
    if(dmxDevice->isOpen()) {
		memset( dmxData, 0, DMX_DATA_LENGTH );
		dmxDevice->writeDmx( dmxData, DMX_DATA_LENGTH );
		dmxDevice->close();
    }
    #ifdef TARGET_RASPBERRY_PI
    // Turn on fans
    digitalWrite (RELAY1, LOW);
    digitalWrite (RELAY2, LOW);
    #endif
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(false);
    ofSetFrameRate(80);
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

	int bufferSize = 512;
	sampleRate = 44100;
	soundStream.printDeviceList();
	//soundStream.setDeviceID(1); 	//note some devices are input only and some are output only 
	soundStream.setup(this, 2, 0, sampleRate, bufferSize, 4);
	
    numLights = 6;
    numSequences = 4;
    freq = 0;
    lightOutput = 0;
    sequence = ofRandom(1,numSequences + 1);

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
    ofAddListener(timeline.events().playbackEnded, this, &ofApp::playBackEnded);
    
#ifdef TARGET_RASPBERRY_PI
    wiringPiSetup () ;
    pinMode (RELAY1, OUTPUT);
    pinMode (RELAY2, OUTPUT);
    pinMode(PIR1,INPUT);
    pinMode(PIR2,INPUT);
#endif

	timeline.toggleShow();
	timeline.play();
	
	cout << "New sequence = " << sequence << endl;
}

//--------------------------------------------------------------
void ofApp::playBackEnded(ofxTLPlaybackEventArgs& args) {
    cout << "Playback ended" << endl;

    int newseq = ofRandom(1,numSequences + 1);
    while(newseq == sequence) {
        newseq = ofRandom(1,numSequences + 1);
    }

    sequence = newseq;

    cout << "New sequence = " << sequence << endl;
}

//--------------------------------------------------------------
void ofApp::bangFired(ofxTLBangEventArgs& args){
    ofLogVerbose() << args.track->getName() << " fired: " << args.flag;

    int seqnum = stoi(args.track->getName().substr(3,1),nullptr);
    //cout << "seqnum = " << seqnum << endl;

    if(sequence == seqnum) {

        if(args.track->getName() == ("SEQ"+ofToString(sequence)+"_STROBE"))
        {
            if(!(args.flag.empty())) {
                freq = stof(args.flag);
                phaseAdder = (freq / (float) sampleRate) * TWO_PI;
                cout << "freq=" << freq << " phaseAdder = " << phaseAdder << endl;
            }
        }

        if(args.track->getName() == ("SEQ"+ofToString(sequence)+"_FAN1"))
        {
        #ifdef TARGET_RASPBERRY_PI
            if(args.flag == "ON")
            {
                digitalWrite (RELAY1, HIGH);
            } else {
                digitalWrite (RELAY1, LOW);
            }
        #endif
        }

         if(args.track->getName() == ("SEQ"+ofToString(sequence)+"_FAN2"))
        {
        #ifdef TARGET_RASPBERRY_PI
            if(args.flag == "ON")
            {
                digitalWrite (RELAY2, HIGH);
            } else {
                digitalWrite (RELAY2, LOW);
            }
        #endif
        }

    }
}


//--------------------------------------------------------------
void ofApp::update(){

#ifdef TARGET_RASPBERRY_PI
    pir1 = digitalRead(PIR1);
    pir2 = digitalRead(PIR2);
#endif


    //cout << "PIR1 = " << pir1 << endl;
    //cout << "PIR2 = " << pir2 << endl;

    if((pir1 == 1) || (pir2 == 1)) {
        if(!(timeline.getIsPlaying())) {
            timeline.play();
        }
    }

    //lightOutput = sin(((double) ofGetElapsedTimeMillis()/200.0)*(double)freq) > 0 ? 255:0;
    
    if(freq == 0) lightOutput = 255;

    //cout << "lightOutput = " << lightOutput << endl;


    for(int channel=1; channel <= numLights*2; channel+=2) {
        dmxData[channel] = lightOutput;
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
    
    ofDrawBitmapString(ofToString(ofGetFrameRate()),20,ofGetHeight()-40);
}

//--------------------------------------------------------------
void ofApp::audioOut(float * output, int bufferSize, int nChannels)
{
	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI){
		phase -= TWO_PI;
	}

	for (int i = 0; i < bufferSize; i++){
		phase += phaseAdder;

		lightOutput = sin(phase) > 0.0f?255.0f:0.0f;  
			output[i*nChannels    ] = 0;
			output[i*nChannels + 1] = 0;
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key =='h') {		
		timeline.toggleShow();
		//if(timeline.getIsShowing()) setWindowShape(1600,900); 
		//else setWindowShape(320,240);
	}
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
