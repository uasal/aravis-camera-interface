#ifndef CAMERA_H
#define CAMERA_H

class Camera {
public: 
	// constructor/destructor
	Camera(char *cameraName = NULL);
	~Camera();
	
	// video capture
	void saveVideo(float frameRate, int windowHeight, int windowWidth);

private:
	ArvCamera *arvCamera;
	
};

#endif
