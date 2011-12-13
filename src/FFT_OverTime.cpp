/*
 *  FFT_OverTime.cpp
 *  FFT_Time
 *
 *  Created by Matthew Ruby on 2/23/11.
 *  Copyright 2011. All rights reserved.
 *
 */

#include "FFT_OverTime.h"


//------------------------------------------------------------------
void FFT_OverTime::setup() {
	playhead.set(0, 0, 0);
}



//------------------------------------------------------------------
void FFT_OverTime::update(float inputValue, float amplitude) {
	
	playhead.y = (-inputValue * amplitude);
	playhead.x = 1024;
	
	ofPoint tempPt;
	tempPt.set(playhead.x, playhead.y);
	
	if (inputValue > 0) {
		if (pts.size() > 1024) {
			pts.erase(pts.begin());
			pts.push_back(tempPt);
		} else {
			pts.push_back(tempPt);
		}
	}
	
	for (int i = 0; i < pts.size();i++){
		pts[i].x -= 10;
	}
	
	
}



//------------------------------------------------------------------
void FFT_OverTime::draw(float y) {

	ofNoFill();
	ofPushMatrix();

	ofTranslate(0, y, 0);

		ofBeginShape();
	
		for (int i = 0; i < pts.size();i++){
			ofVertex(pts[i].x, pts[i].y);
		}
	
		ofEndShape();

	ofPopMatrix();

}

