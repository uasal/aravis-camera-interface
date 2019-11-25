#ifndef CAMERA_H
#define CAMERA_H

typedef struct {
    GMainLoop *main_loop;
    int buffer_count;
} ApplicationData;

class Camera {
public: 
	// constructor/destructor
	Camera(char *cameraName = NULL);
	~Camera();
	
	// video capture
	void configureStream(float frameRate, gint windowHeight, gint windowWidth);
	void startStream();
	void stopStream();
	void freeStream();
	
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
