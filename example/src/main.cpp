#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( ){
	ofAppGLFWWindow win;
	win.setNumSamples(0);
	ofSetupOpenGL(&win, /*1680,1050*/ 1024,768, OF_WINDOW /*OF_FULLSCREEN*/);	// <-------- setup the GL context
	ofRunApp(new ofApp());
}
