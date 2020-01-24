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
	Camera(int *status, char *cameraName = NULL);
	~Camera();
	
	// video capture
	void configureStream(float frameRate, gint windowHeight, gint windowWidth);
	void startStream(int maxBufferCount = -1);
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
