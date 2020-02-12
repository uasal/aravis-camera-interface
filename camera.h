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
	
	void configureAttributes(gint windowWidth, gint windowHeight, double exposureTime);
	void startStream(int maxBufferCount, float frameRate);
	void stopStream();
	void freeStream();
	ArvBuffer* getSnapshot();
	
	// getters
	ArvCamera* getArvInstance();
	
private:
	ArvCamera *arvCamera;
	ArvChunkParser *parser;
	
	// for video stream
	ArvStream *stream;
	ApplicationData data;
};

#endif
