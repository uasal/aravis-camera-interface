#ifndef CAMERA_H
#define CAMERA_H

#include "glib.h"
#include "arv.h"

// camera-specific
#define MAX_WIDTH 1280
#define MAX_HEIGHT 1024
#define FEATURE_GAIN "GainAbs"



class HDCamera {
public: 
	// constructor/destructor
	HDCamera(int *status, int packetSize, char *name = NULL);
	~HDCamera();
	
	// image capture
	int startStream(guint64 duration);
	int getSnapshot(guint64 timeout, ArvBuffer **buffer);

	// feature writing
	int setFrameRate(double frameRate);
	int setGain(double gain);
	int setRegion(int x, int y, int width, int height);
	int setExposureTime(double exposureTime);
	int setPixelFormat(const char *pixelFormat);
	
	// getters
	ArvCamera* getArvInstance();
	
private:
	ArvCamera *arvCamera;
	ArvDevice *device;
	
	bool created;
};

#endif
