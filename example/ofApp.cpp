#include "ofApp.h"
#include "ofxThreadSafeLog.h"


void ofApp::setup(){

	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofEnableAlphaBlending();
	ofBackground(22);

	for(int i = 0; i < NUM_THREADS; i++){
		MyThread * t = new MyThread();
		t->setup(i);
		t->startThread();
		myThreads.push_back(t);
	}
}


void ofApp::update(){

	string logLine = ofGetTimestampString() + " - write from the Main Thread";
	ofxThreadSafeLog::one()->append(LOG_FILENAME, logLine);
}



void ofApp::exit(){
	for(int i = 0; i < NUM_THREADS; i++){
		delete myThreads[i];
	}
	myThreads.clear();
	ofxThreadSafeLog::one()->close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MyThread::~MyThread(){
	stopThread();
	waitForThread();
}


void MyThread::setup(int ID){
	tID = ID;
}


void MyThread::threadedFunction(){

	while (isThreadRunning()) {
		ofSleepMillis(4);
		string logLine = ofGetTimestampString() + " - writing from background thread " + ofToString(tID);
		ofxThreadSafeLog::one()->append(LOG_FILENAME, logLine);
	}
}