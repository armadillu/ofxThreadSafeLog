#pragma once

#include "ofMain.h"

#define LOG_FILENAME	"log.txt"
#define NUM_THREADS		3

class MyThread: public ofThread{
public:
	~MyThread();
	void setup(int ID);
	void threadedFunction();
private:
	int tID;
};

class testApp : public ofBaseApp, public ofThread{

public:

	void setup();
	void update();
	void draw(){};

	void exit();

	vector<MyThread *> myThreads;
};
