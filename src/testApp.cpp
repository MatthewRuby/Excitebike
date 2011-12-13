#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	ofBackground(0,0,0);
	
	ofSoundStreamSetup(1,1,this, 44100, BUFFER_SIZE, 4);	
	
	inputBufferCopy = new float[BUFFER_SIZE];

	for (int i = 0; i < BUFFER_SIZE; i++) {
		inputBufferCopy[i] = 0.0;
	}
	
	volume = 0.0;
	volSm = 0.0;
	prevVol[0] = 0.0;
	prevVol[1] = 0.0;
	drawVol.setup();
	volMove = 0;
	
	
	spectCenter = 0.0;
	spectCenterSm = 0.0;
	prevSpectCenter[0] = 0.0;
	prevSpectCenter[1] = 0.0;
	drawSpectCenter.setup();
	spectCenterMove = 0;
	
	
	avgMag = 0.0;
	avgMagSm = 0.0;
	prevAvgMag[0] = 0.0;
	prevAvgMag[1] = 0.0;
	drawAvgMag.setup();
	avgMagMove = 0;
	
	
	stdDev = 0.0;
	stdDevSm = 0.0;
	prevStdDev[0] = 0.0;
	prevStdDev[1] = 0.0;
	drawStdDev.setup();
	stdDevMove = 0;
	
	
	flux = 0.0;
	fluxSm = 0.0;
	prevFlux[0] = 0.0;
	prevFlux[1] = 0.0;
	prevMags = new float[BUFFER_SIZE/2];
	drawFlux.setup();
	fluxMove = 0;
	
	
	
//	ard.connect("/dev/tty.usbmodemfa141", 57600);
	ard.connect("/dev/tty.usbmodemfd131", 57600);
	
//	ard.connect("/dev/tty.usbserial-A7006QG2", 57600);
	bSetupArduino = false;
	
	
	bUP = false;
	bDOWN = false;
	bRIGHT = false;
	bLEFT = false;
	
	buttonONE = false;
	buttonTWO = false;
	
	bStart = false;
	bReset = false;
	bPause = false;
	
	
	
	maxVol = 0.0001;
	threshA = 0.1;
	threshB = 0.2;
	
	laneSpectMin = 0.0;
	laneSpectMax = 0.0;
	laneWindow = 0.0;
	laneWidth = 0.0;
	
	lane = 1;
	prevLane = 1;
	DTimer = 0;
	UTimer = 0;

	
}

//--------------------------------------------------------------
void testApp::update(){
	
	if ( ard.isArduinoReady()){
		
		if (bSetupArduino == false){
			
			setupArduino();
			bSetupArduino = true;
			
		}
		
		updateArduino();
		
		
	//--- Calculate Volume ---
		volSm = SMOOTH_BY * volume + (1 - SMOOTH_BY) * volSm;
		drawVol.update(volSm, 200.0);
		
		if (volSm > maxVol) {
			maxVol = volSm;
		}

		maxVol *= 0.9995;
		
		
	//--- Calculate Average Magnitude ---
		avgMag = 0;
		//	avgMagSm = 0;
		for (int i = 0; i < (int)(BUFFER_SIZE/2); i++){
			avgMag += magnitude[i];
		}
		avgMag /= (int)(BUFFER_SIZE/2);
		avgMagSm = SMOOTH_BY * avgMag + (1 - SMOOTH_BY) * avgMagSm;
		drawAvgMag.update(avgMagSm, 200.0);
		
		
	//--- Store The Previous Magnitude ---
		for (int i = 0; i < (int)(BUFFER_SIZE/2); i++){
			prevMags[i] = magnitude[i];
		}
		
		
	//--- Calculate Standard Deviation Magnitude ---
		stdDev = 0;
		//	stdDevSm = 0;
		for (int i = 0; i < (int)(BUFFER_SIZE/2); i++){
			stdDev += (avgMag - magnitude[i]) * (avgMag - magnitude[i]);
			
		}
		stdDev /= (int)(BUFFER_SIZE/2);
		stdDev = sqrt(stdDev);
		stdDevSm = SMOOTH_BY * stdDev + (1 - SMOOTH_BY) * stdDevSm;
		
		drawStdDev.update(stdDevSm, 200.0);
		
		
	//--- Calculate FFT ---
		static int index=0;
		float avg_power = 0.0f;
		
		myfft.powerSpectrum(0, (int)BUFFER_SIZE/2, inputBufferCopy, BUFFER_SIZE, &magnitude[0], &phase[0], &power[0], &avg_power);
		

		
	//--- Average the Bins Greater than the Standard Deviation to find frequency Center ---
		int div = 0;
		spectCenter = 0;
		for (int i = 0; i < (int)BUFFER_SIZE/2; i++) {
			if (magnitude[i] > stdDevSm) {
				spectCenter += i;
				div++;
			}
		}
		spectCenter /= div;
		
		if (spectCenter > 0) {
			spectCenterSm = SMOOTH_BY * spectCenter + (1 - SMOOTH_BY) * spectCenterSm;
		}
		drawSpectCenter.update(spectCenterSm, 0.5);
		
		if (spectCenterSm > laneSpectMax) {
			laneSpectMax = spectCenterSm;
		}
		laneSpectMax *= 0.999;
		
		if (spectCenterSm < laneSpectMin) {
			laneSpectMin = spectCenterSm;
		}
		laneSpectMin += 0.005;
		
		laneWindow = 0.009 * spectCenterSm + (1 - 0.009) * laneWindow;
		laneWidth = (laneSpectMax - laneSpectMin) / 4;
		
		
	//--- Calculate Flux Magnitude ---
		flux = 0;
		//	fluxSm = 0;
		for (int i = 0; i < (int)(BUFFER_SIZE/2); i++){
			flux += abs(prevMags[i] - magnitude[i]);
		}
		flux /= (int)(BUFFER_SIZE/2);
		
		fluxSm = SMOOTH_BY * flux + (1 - SMOOTH_BY) * fluxSm;
		drawFlux.update(fluxSm, 200.0);
		
	
	//--- Calculate the Direction of Movement ---
		if (volSm >= prevVol[0] && prevVol[0] >= prevVol[1]) {
			volMove++;
		} else if (volSm <= prevVol[0] && prevVol[0] <= prevVol[1]) {
			volMove--;
		} else {
			volMove = 0;
		}
		
		
		if (spectCenterSm >= prevSpectCenter[0] && prevSpectCenter[0] >= prevSpectCenter[1]) {
			volMove++;
		} else if (spectCenterSm <= prevSpectCenter[0] && prevSpectCenter[0] <= prevSpectCenter[1]) {
			spectCenterMove--;
		} else {
			spectCenterMove = 0;
		}
		
		
		if (avgMagSm >= prevAvgMag[0] && prevAvgMag[0] >= prevAvgMag[1]) {
			avgMagMove++;
		} else if (avgMagSm <= prevAvgMag[0] && prevAvgMag[0] <= prevAvgMag[1]) {
			avgMagMove--;
		} else {
			avgMagMove = 0;
		}
		
		
		if (stdDevSm >= prevStdDev[0] && prevStdDev[0] >= prevStdDev[1]) {
			stdDevMove++;
		} else if (stdDevSm <= prevStdDev[0] && prevStdDev[0] <= prevStdDev[1]) {
			stdDevMove--;
		} else {
			stdDevMove = 0;
		}
		
		
		if (fluxSm >= prevFlux[0] && prevFlux[0] >= prevFlux[1]) {
			fluxMove++;
		} else if (fluxSm <= prevFlux[0] && prevFlux[0] <= prevFlux[1]) {
			fluxMove--;
		} else {
			fluxMove = 0;
		}
		

		
		threshA = maxVol * 0.5;
		threshB = maxVol * 0.75;

		
		
		if (!bPause) {		
			
		//--- Reg Gas ---	
			if (volSm > threshA) {
				buttonONE = true;
			} else {
				buttonONE = false;
			}
		//--- Turbo Gas ---	
			if (volSm > threshB) {
				buttonTWO = true;
			} else {
				buttonTWO = false;
			}
			
			
		//--- Pull Back ---	
			if (avgMagMove > 10) {
				bLEFT = true;
			} else {
				bLEFT = false;
			}

		//--- Push Forward ---	
			if (avgMagMove < -10) {
				bRIGHT = true;
			} else {
				bRIGHT = false;
			}
		

			
		//--- Move Up ---
			if (spectCenterSm > laneWindow + laneWidth && bUP == false && bDOWN == false) {
				
				UTimer = ofGetElapsedTimeMillis() + 1000;
				
			}
			
			if (UTimer > ofGetElapsedTimeMillis()) {

				bDOWN = false;
				bUP = true;

			} else if (UTimer + 1000 > ofGetElapsedTimeMillis()) {
				
				bUP = false;
				bDOWN = false;
				
			} else {
				
				bUP = false;
				
			}


		//--- Move Down ---			
			if (spectCenterSm < laneWindow - laneWidth && bDOWN == false && bUP == false) {
				
				DTimer = ofGetElapsedTimeMillis() + 1000;
				
			}
			
			if (DTimer > ofGetElapsedTimeMillis()) {
				
				bUP = false;
				bDOWN = true;

			} else if (DTimer + 1000 > ofGetElapsedTimeMillis()) {
				
				bDOWN = false;
				bUP = false;
				
			} else {
				
				bDOWN = false;
				
			}
			
		}
		
		
		
	//--- Arduino Pin Outputs ---	
		if (buttonONE) {
			ard.sendDigital(4, ARD_HIGH);	// Pin 4 = Button One
		} else {
			ard.sendDigital(4, ARD_LOW);
		}
		
		
		if (buttonTWO) {
			ard.sendDigital(5, ARD_HIGH);	// Pin 5 = Button Two
		} else {
			ard.sendDigital(5, ARD_LOW);
		}
		
		
		if (bStart) {
			ard.sendDigital(7, ARD_HIGH);	// Pin 7 = Start
		} else {
			ard.sendDigital(7, ARD_LOW);
		}
		
		
		if (bReset) {
			ard.sendDigital(8, ARD_HIGH);	// Pin 8 = Reset
		} else {
			ard.sendDigital(8, ARD_LOW);
		}
		
		
		
		
		if (bUP) {
			ard.sendDigital(10, ARD_HIGH);	// Pin 10 = Up
		} else {
			ard.sendDigital(10, ARD_LOW);
		}
		
		if (bDOWN) {
			ard.sendDigital(11, ARD_HIGH);	// Pin 11 = Down
		} else {
			ard.sendDigital(11, ARD_LOW);
		}
		
		if (bLEFT) {
			ard.sendDigital(12, ARD_HIGH);	// Pin 12 = Left
		} else {
			ard.sendDigital(12, ARD_LOW);
		}
		
		if (bRIGHT) {
			ard.sendDigital(9, ARD_HIGH);	// Pin 9 = Right
		} else {
			ard.sendDigital(9, ARD_LOW);
		}
			
		
		
		
	//--- Store Previous Values ---		
		prevVol[1] = prevVol[0];
		prevVol[0] = volSm;
		
		prevSpectCenter[1] = prevSpectCenter[0];
		prevSpectCenter[0] = spectCenterSm;
		
		prevAvgMag[1] = prevAvgMag[0];
		prevAvgMag[0] = avgMagSm;
		
		prevStdDev[1] = prevStdDev[0];
		prevStdDev[0] = stdDevSm;
		
		prevFlux[1] = prevFlux[0];
		prevFlux[0] = fluxSm;
		
		prevLane = lane;
		
	}
}

//--------------------------------------------------------------
void testApp::setupArduino(){

	for (int i = 0; i < 13; i++){
		
		ard.sendDigitalPinMode(i, ARD_OUTPUT);

		ard.sendDigital(i, ARD_LOW);
		
	}

}

//--------------------------------------------------------------
void testApp::updateArduino(){

	ard.update();

}


//--------------------------------------------------------------
void testApp::draw(){
	
	if (!ard.isArduinoReady()){
		
		ofDrawBitmapString("arduino not ready", ofGetWidth()/2 - 50, ofGetHeight()/2);

	} else {
		
		ofSetColor(60, 60, 60);
		ofLine(0, 501, ofGetWidth(), 501);
		
		ofFill();
		ofSetColor(200, 200, 200);
		for (int i = 0; i < (int)(BUFFER_SIZE/2); i++){
			ofRect(10+(i*5),ofGetHeight()-10, 3,-magnitude[i] * 4);	
		}
		
		ofSetColor(255, 127, 127);
		ofLine(10 + (spectCenterSm * 5), ofGetHeight(), 10 + (spectCenterSm * 5), ofGetHeight() - 150);
		
		ofSetColor(127, 255, 127);
		ofLine(10 + (laneSpectMin * 5), ofGetHeight(), 10 + (laneSpectMin * 5), ofGetHeight() - 150);
		ofSetColor(127, 255, 127);
		ofLine(10 + (laneSpectMax * 5), ofGetHeight(), 10 + (laneSpectMax * 5), ofGetHeight() - 150);
		
		
		ofSetColor(255, 255, 255);
		ofLine(10 + (laneWindow - laneWidth) * 5, ofGetHeight(), 10 + (laneWindow - laneWidth) * 5, ofGetHeight() - 150);
		ofLine(10 + (laneWindow + laneWidth) * 5, ofGetHeight(), 10 + (laneWindow + laneWidth) * 5, ofGetHeight() - 150);
		
		
		
		ofSetColor(255, 255, 0);
		drawVol.draw(500);
		ofDrawBitmapString("vol = \n" + ofToString(volSm*100, 6), 25, 525);
		ofDrawBitmapString("volMove =\n" + ofToString(volMove, 0), 25, 575);

		
		
		ofSetColor(0, 255, 255);
		drawSpectCenter.draw(500);
		ofDrawBitmapString("spectCenter = \n" + ofToString(spectCenterSm, 6), 175, 525);
		ofDrawBitmapString("spectCenterMove =\n" + ofToString(spectCenterMove, 0), 175, 575);

		
		
		ofSetColor(255, 0, 0);
		drawAvgMag.draw(500);
		ofDrawBitmapString("avgMag =\n" + ofToString(avgMagSm*100, 6), 325, 525);
		ofDrawBitmapString("avgMagMove =\n" + ofToString(avgMagMove, 0), 325, 575);

		
		
		ofSetColor(0, 255, 0);
		drawStdDev.draw(500);
		ofDrawBitmapString("stdDev =\n" + ofToString(stdDevSm*100, 6), 475, 525);
		ofDrawBitmapString("stdDevMove =\n" + ofToString(stdDevMove, 0), 475, 575);

		
		
		ofSetColor(0, 0, 255);
		drawFlux.draw(500);
		ofDrawBitmapString("flux =\n" + ofToString(flux*100, 6), 625, 525);
		ofDrawBitmapString("fluxMove =\n" + ofToString(fluxMove, 0), 625, 575);

	
		
		ofSetColor(255, 255, 0);
		ofDrawBitmapString("maxVol =\n" + ofToString(maxVol*100, 6), 900, 700);
		ofLine(900, 750 - (maxVol * 10000), 950, 750 - (maxVol * 10000));
		ofSetColor(255, 255, 255);
		ofLine(900, 750 - (volSm * 10000), 950, 750 - (volSm * 10000));
		ofSetColor(255, 0, 0);
		ofLine(900, 750 - (threshA * 10000), 950, 750 - (threshA * 10000));
		ofSetColor(255, 0, 255);
		ofLine(900, 750 - (threshB * 10000), 950, 750 - (threshB * 10000));
		
		
		
		if (bDOWN) {
			ofSetColor(0, 255, 0);
			ofDrawBitmapString("DOWN", 475, 650);
		} else {
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("d", 475, 650);
		}
		ofSetColor(255);
		ofDrawBitmapString(ofToString(DTimer, 0), 425, 650);
		
		if (bUP) {
			ofSetColor(0, 255, 0);
			ofDrawBitmapString("UP", 475, 675);
		} else {
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("u", 475, 675);
		}
		ofSetColor(255);
		ofDrawBitmapString(ofToString(UTimer, 0), 425, 675);
		
		if (bLEFT) {
			ofSetColor(0, 255, 0);
			ofDrawBitmapString("LEFT", 550, 650);
		} else {
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("l", 550, 650);
		}
		if (bRIGHT) {
			ofSetColor(0, 255, 0);
			ofDrawBitmapString("RIGHT", 550, 675);
		} else {
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("r", 550, 675);
		}
		
		if (buttonONE) {
			ofSetColor(0, 255, 0);
			ofDrawBitmapString("A Button", 625, 650);
		} else {
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("a", 625, 650);
		}
		if (buttonTWO) {
			ofSetColor(0, 255, 0);
			ofDrawBitmapString("B Button", 625, 675);
		} else {
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("b", 625, 675);
		}
			
		if (bPause) {
			ofSetColor(255, 0, 255);
			ofDrawBitmapString("PAUSE", 600, 700);
		} else {
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("p", 600, 700);
		}

	}	
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){

	if (key == OF_KEY_UP) {
		bUP = true;
	}
	if (key == OF_KEY_RIGHT) {
		bRIGHT = true;
	}
	if (key == OF_KEY_DOWN) {
		bDOWN = true;
	}
	if (key == OF_KEY_LEFT) {
		bLEFT = true;
	}
	
	if (key == 'a') {
		buttonONE = true;
	}
	if (key == 's') {
		buttonTWO = true;
	}
	
	if (key == 'r') {
		bReset = true;
	}
	if (key == 't') {
		bStart = true;
	}
	
	if (key == ' ') {
		bPause = !bPause;
	}
	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

	if (key == OF_KEY_UP) {
		bUP = false;
	}
	if (key == OF_KEY_RIGHT) {
		bRIGHT = false;
	}
	if (key == OF_KEY_DOWN) {
		bDOWN = false;
	}
	if (key == OF_KEY_LEFT) {
		bLEFT = false;
	}
	
	if (key == 'a') {
		buttonONE = false;
	}
	if (key == 's') {
		buttonTWO = false;
	}
	
	if (key == 'r') {
		bReset = false;
	}
	if (key == 't') {
		bStart = false;
	}
	
}


//--------------------------------------------------------------
void testApp::audioRequested (float * output, int bufferSize, int nChannels){
	
	for (int i = 0; i < bufferSize; i++){

		output[i] = inputBufferCopy[i];

	}
	
}


//--------------------------------------------------------------
void testApp::audioReceived(float * input, int bufferSize, int nChannels){	
	
	volume = 0;
	
	for (int i = 0; i < bufferSize; i++){
		
		inputBufferCopy[i] = input[i];
		volume += input[i]*input[i];
		
	}
	
	volume /= bufferSize;
	volume = sqrt(volume);
	
}


