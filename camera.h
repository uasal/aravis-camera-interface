#ifndef CAMERA_H
#define CAMERA_H

typedef struct {
    GMainLoop *main_loop;
    int buffer_count;
    int maxBufferCount;
    int totalBufferCount;
    int done;
} ApplicationData;

class Camera {
public: 
	// constructor/destructor
	Camera(int *status, int packetSize, char *name = NULL);
	~Camera();
	
	// video capture
	void startStream(int maxBufferCount, float frameRate);
	void stopStream();
	void freeStream();
	ArvBuffer* getSnapshot(guint64 timeout, int toggleDataRetrieval, int *status);

	// feature writing
	void configureAttributes(gint windowWidth, gint windowHeight, double exposureTime);
	void setFrameRate(double frameRate, int *status);
	void setGain(double gain, int *status);

	// getters
	ArvCamera* getArvInstance();
	
private:
	ArvCamera *arvCamera;
	ArvDevice *device;
};

#endif
