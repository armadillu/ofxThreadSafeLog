//
//  ofxThreadSafeLog.cpp
//  CollectionTable
//
//  Created by Oriol Ferrer Mesià on 17/02/15.
//
//

#include "ofxThreadSafeLog.h"

ofxThreadSafeLog* ofxThreadSafeLog::singleton = NULL;


ofxThreadSafeLog::ofxThreadSafeLog(){
	startThread();
}


void ofxThreadSafeLog::close(){
	stopThread();
	waitForThread();
	lock();
	map<string, ofFile*>::iterator it = logs.begin();
	while(it != logs.end()){
		it->second->close();
		delete it->second;
		++it;
	}
	pendingLines.clear();
	logs.clear();
	logFilesPendingCreation.clear();
	unlock();
}


ofxThreadSafeLog* ofxThreadSafeLog::one(){
	if (!singleton){   // Only allow one instance of class to be generated.
		singleton = new ofxThreadSafeLog();
	}
	return singleton;
}


void ofxThreadSafeLog::append(const string& logFile, const string& line){

	lock();
	map<string,vector<string> >::iterator it = pendingLines.find(logFile);
	if(it == pendingLines.end()){ //new log file!
		logFilesPendingCreation.push_back(logFile);
	}
	pendingLines[logFile].push_back(line);
	unlock();
}


void ofxThreadSafeLog::threadedFunction(){

	while(isThreadRunning()){
		lock();

		// 1 create pending log files (new log!)
		if(logFilesPendingCreation.size()){
			vector<string>::iterator it = logFilesPendingCreation.begin();
			while( it != logFilesPendingCreation.end()){
				ofFile * newLog = new ofFile();
				newLog->open(*it, ofFile::WriteOnly, false); //TODO append mode?
				logs[*it] = newLog;
				++it;
			}
			logFilesPendingCreation.clear();
		}

		// 2 write lines into logs
		map<string,vector<string> >::iterator it = pendingLines.begin();
		while( it != pendingLines.end()){
			string logFile = it->first;
			ofFile * log = logs[logFile];
			vector<string> & lines = it->second;

			vector<string>::iterator it2 = lines.begin();
			while(it2 != lines.end()){
				*log << *it2 << endl;
				++it2;
			}
			lines.clear();
			++it;
		}
		unlock();
		ofSleepMillis(100);
	}
}
