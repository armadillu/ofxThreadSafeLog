//
//  ofxThreadSafeLog.h
//  CollectionTable
//
//  Created by Oriol Ferrer Mesià on 17/02/15.
//
//

#ifndef __CollectionTable__ofxThreadSafeLog__
#define __CollectionTable__ofxThreadSafeLog__

#include "ofMain.h"

class ofxThreadSafeLog: public ofThread{

public:

	static ofxThreadSafeLog* one();
	
	void append(const string &logFile, const string & line);
	void close(); //call at end of program

private:


	ofxThreadSafeLog(); // use ofxThreadSafeLog::instance() instead!
	//~ofxThreadSafeLog();


	map<string, vector<string> > 	pendingLines;
	map<string, ofFile*> 			logs;

	vector<string> 					logFilesPendingCreation;
	static ofxThreadSafeLog *		singleton;

	void threadedFunction();

};


#endif /* defined(__CollectionTable__ofxThreadSafeLog__) */
