#ifndef CAMERA_H
#define CAMERA_H

typedef struct {
    GMainLoop *main_loop;
    int buffer_count;
    int maxBufferCount;
    int totalBufferCount;
    int done;
} ApplicationData;

class HDCamera {
public: 
	// constructor/destructor
	HDCamera(int *status, int packetSize, char *name = NULL);
	~HDCamera();
	
	// image capture
	int startStream(int maxBufferCount);
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
};

#endif
