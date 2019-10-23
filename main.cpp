// system and aravis includes
#include "glib.h"
#include "arv.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <png.h> // Requires libpng1.2
#include <assert.h>
#include "camera.h"

int main(int argc, char *argv[]){

	/*
	ArvCamera *camera;
	ArvBuffer *buffer;
	std::string filename="test.png";
	camera = arv_camera_new (argc > 1 ? argv[1] : NULL);
	/*
	buffer = arv_camera_acquisition (camera, 0);
	if (ARV_IS_BUFFER (buffer)){
		printf ("Image successfully acquired\n");
		arv_save_png(buffer, filename.c_str());
	}
	else{
		printf ("Failed to acquire a single image\n");
	}
	assert(camera != NULL);
	saveVideo(camera);
	g_clear_object (&camera);
	*/
	//g_clear_object (&buffer);
	Camera camera = Camera();
	camera.saveVideo(10.0, 200, 200);
	return 0;
}
