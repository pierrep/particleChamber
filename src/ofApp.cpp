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
	for(int channel=1; channel <= numLights*2; channel+=2) {
        dmxData[channel] = 0;
        dmxData[channel + 1] = 11;
    }
    
    if(dmxDevice->isOpen()) {
		memset( dmxData, 0, DMX_DATA_LENGTH );
		dmxDevice->writeDmx( dmxData, DMX_DATA_LENGTH );
		dmxDevice->close();
    }
    #ifdef TARGET_RASPBERRY_PI
    // Turn off fans
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
		
	loadSettings();

    timeline.setup();
    timeline.setDurationInSeconds(60*8);

    for(int i = 1;i <= numSequences;i++) {
        timeline.addFlags("SEQ"+ofToString(i)+"_FAN1");
        timeline.addFlags("SEQ"+ofToString(i)+"_FAN2");
        timeline.addFlags("SEQ"+ofToString(i)+"_STROBE");
    }

    timeline.enableSnapToOtherKeyframes(false);
    timeline.setLoopType(OF_LOOP_NONE);
    //timeline.setSpacebarTogglePlay(true);

    ofAddListener(timeline.events().bangFired, this, &ofApp::bangFired);
    ofAddListener(timeline.events().playbackEnded, this, &ofApp::playBackEnded);
    
#ifdef TARGET_RASPBERRY_PI
    wiringPiSetup () ;
    pinMode (RELAY1, OUTPUT);
    pinMode (RELAY2, OUTPUT);
    pinMode(PIR1,INPUT);
    pinMode(PIR2,INPUT);
    
    // Turn off fans
    digitalWrite (RELAY1, LOW);
    digitalWrite (RELAY2, LOW);    
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

    if(sequence == seqnum) {

        if(args.track->getName() == ("SEQ"+ofToString(sequence)+"_STROBE"))
        {
            if(!(args.flag.empty())) {
                freq = stof(args.flag);
                cout << "freq=" << freq << endl;
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

	if(!(timeline.getIsPlaying())) {
		if((pir1 == 1) || (pir2 == 1)) {
            timeline.play();
        } else {
			lightOutput = 0;
		}   
    } else {
		if(freq == 0) {
			lightOutput = 255;
			strobe = 11;
		} else {
			lightOutput = 255;
			int index = freq - 1;
			strobe = strobeVal[index];
		}
	}

    for(int channel=1; channel <= numLights*2; channel+=2) {
        dmxData[channel] = lightOutput;
        dmxData[channel + 1] = strobe;
    }

	sendDMX();
}

//--------------------------------------------------------------
void ofApp::draw(){

    timeline.draw();
    
    ofDrawBitmapString(ofToString(ofGetFrameRate()),20,ofGetHeight()-40);
}

//--------------------------------------------------------------
void ofApp::sendDMX() 
{
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
void ofApp::loadSettings()
{
	numLights = 6;
    numSequences = 4;
    freq = 0;
    lightOutput = 0;
    
	if( xml.load("settings.xml") ){
		ofLogNotice() << "Loaded settings.xml";
	} else {
		ofLogError() << "Failed to load settings.xml";
	}

    if(xml.exists("//STROBE1")) {
        strobeVal[0] = xml.getValue<int>("//STROBE1");
    } else {
        strobeVal[0] = 50;
    }

    if(xml.exists("//STROBE2")) {
        strobeVal[1] = xml.getValue<int>("//STROBE2");
    } else {
        strobeVal[1] = 100;
    }

    if(xml.exists("//STROBE3")) {
        strobeVal[2] = xml.getValue<int>("//STROBE3");
    } else {
        strobeVal[2] = 150;
    }

    if(xml.exists("//STROBE4")) {
        strobeVal[3] = xml.getValue<int>("//STROBE4");
    } else {
        strobeVal[3] = 220;
    }

    if(xml.exists("//STARTSEQUENCE")) {
        sequence = xml.getValue<int>("//STARTSEQUENCE");
    } else {
        sequence = 1;		
    }
    //ofRandom(1,numSequences + 1);        
    sequence = 1;
	//cout << "strobeVal:" << strobeVal[0] << " " << strobeVal[1] << " " << strobeVal[2] << " " << strobeVal[3] << " " << endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key =='h') {		
		timeline.toggleShow();
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
