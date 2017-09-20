#pragma once

#include "ofMain.h"
#include "ofxGenericDmx.h"
#include "ofxTimeline.h"

#define DMX_DATA_LENGTH 513

class ofApp : public ofBaseApp{

	public:
        void exit();
		void setup();
		void update();
		void draw();

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

        ofxTimeline timeline;
        void bangFired(ofxTLBangEventArgs& args);
        void playBackEnded(ofxTLPlaybackEventArgs& args);
		void sendDMX();
		void loadSettings();

        //pointer to our Enntec DMX USB Pro object
        DmxDevice* dmxDevice;

        //our DMX packet (which holds the channel values + 1st byte for the start code)
        unsigned char dmxData[DMX_DATA_LENGTH];

        int numLights;
        int numSequences;
        float freq;
        int lightOutput;

        int pir1, pir2;
        int sequence;
        
        ofXml xml;
        int strobeVal[4];
        int strobe;
         
		
};
