#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "fft.h"
#include "FFT_OverTime.h"

#define BUFFER_SIZE 512
#define SMOOTH_BY 0.1

class testApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);

	
//--- Sound Functions and Variables ---
		void audioRequested (float * input, int bufferSize, int nChannels);
		void audioReceived(float * input, int bufferSize, int nChannels);
		
		float * inputBufferCopy;
	
		fft	myfft;
		
		float magnitude[BUFFER_SIZE];
		float phase[BUFFER_SIZE];
		float power[BUFFER_SIZE];

	
		float volume;
		float volSm;
		float prevVol[2];
		FFT_OverTime drawVol;
		int volMove;
	
	
		float spectCenter;
		float spectCenterSm;
		float prevSpectCenter[2];
		FFT_OverTime drawSpectCenter;
		int spectCenterMove;
	
		
		float avgMag;
		float avgMagSm;
		float prevAvgMag[2];
		FFT_OverTime drawAvgMag;
		int avgMagMove;
	
		
		float stdDev;
		float stdDevSm;
		float prevStdDev[2];
		FFT_OverTime drawStdDev;
		int stdDevMove;
	
		
		float * prevMags;
		float flux;
		float fluxSm;
		float prevFlux[2];
		FFT_OverTime drawFlux;
		int fluxMove;

	
//--- Sound Functions and Variables ---
		void setupArduino();
		void updateArduino();

		ofArduino	ard;
		bool		bSetupArduino;
	
		bool		bSetupGame;
		bool		bSetupToggle;
		int			setupCounter;
		int			setupTimer;
		
		
		int				direct;
		bool			bUP;
		bool			bDOWN;
		bool			bRIGHT;
		bool			bLEFT;
		
		bool			buttonONE;
		bool			buttonTWO;
		
		bool			bStart;
		bool			bReset;
		bool			bPause;
	
	
//--- Threshold Variables ---	
		float maxVol;
		float threshA;
		float threshB;
	
		float laneSpectMin;
		float laneSpectMax;
		float laneWindow;
		float laneWidth;
	
		int lane;
		int prevLane;
		
		unsigned long	UTimer;	
		unsigned long	DTimer;
	

};

#endif

