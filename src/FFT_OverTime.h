/*
 *  FFT_OverTime.h
 *  FFT_Time
 *
 *  Created by Matthew Ruby on 2/23/11.
 *  Copyright 2011. All rights reserved.
 *
 */
#ifndef _FFT_OverTime
#define _FFT_OverTime


#include "ofMain.h"


class FFT_OverTime {
	
	public:
		void setup();
		void update(float inputValue, float amplitude);
		void draw(float y);
	
		ofPoint playhead;
		vector <ofPoint> pts;
};

#endif
