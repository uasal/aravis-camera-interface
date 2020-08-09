#include "glib.h"
#include "arv.h"
#include "arvgvstream.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <unistd.h>
#include "hdcamera.h"
#include <iostream>
#include "config.h"
#include <string.h>

void arv_save_png(ArvBuffer * buffer, const char * filename);

using namespace std;

/*
	Parameters: status code, packet size of UDP data packets, optional name identifier for the camera
 */
HDCamera::HDCamera(int *status, int packetSize, char *name) {
	*status = SUCCESS;

	arvCamera = arv_camera_new(name);
	if (!ARV_IS_CAMERA(arvCamera)) {
		*status = ERROR_CAMERA_NOT_FOUND;
	} else {
		device = arv_camera_get_device(arvCamera); // TODO: Error check this line?
		arv_camera_gv_set_packet_size(arvCamera, packetSize);
		arv_camera_set_trigger(arvCamera, "Software");
  	}
}

HDCamera::~HDCamera() {
	g_clear_object(&device);
	g_clear_object(&arvCamera);
}

/*
	Starts stream for the camera.
	
	Parameters: duration for the stream, not equivalent to the video duration, since actual 
	fps depends on camera and ethernet performance

	Returns: status code	
 */
int HDCamera::startStream(guint64 duration) {
	ArvCamera *camera = arvCamera;
    
	ArvStream *stream = arv_camera_create_stream (camera, NULL, NULL);

	if (stream != NULL) {
		arv_camera_set_acquisition_mode (arvCamera, ARV_ACQUISITION_MODE_CONTINUOUS);
		arv_camera_start_acquisition (arvCamera);
		sleep(duration);
		arv_camera_stop_acquisition (arvCamera);

		g_object_unref(stream);
		return SUCCESS;
	} else {
		printf ("Stream thread was unintialized (check if the device is not already used, or if stream was configured)\n");
		return ERROR_STREAM_START_FAILED;
	}
}

/*
	Obtains snapshot for the camera

	Parameters: timeout to wait for the snapshot, needs to be long enough for all packets of the snapshot 
	Returns: status code	
 */
int HDCamera::getSnapshot(guint64 timeout, ArvBuffer **buffer) {
	ArvStream *stream = NULL;
	gint payload = arv_camera_get_payload(arvCamera);

	stream = arv_camera_create_stream(arvCamera, NULL, NULL);
  	if (NULL != stream) {
    	arv_stream_push_buffer(stream, arv_buffer_new(payload, NULL));
    	arv_camera_start_acquisition(arvCamera);
    
    	arv_camera_software_trigger(arvCamera);

    	if (NULL != buffer) {
    		*buffer = arv_stream_timeout_pop_buffer(stream, timeout);
    		if (NULL != *buffer) {
				printf("Buffer was obtained from camera\n");
			}
    		else {
				printf("Buffer was not obtained from camera\n");
			}
    	}
    	else {
      		usleep(timeout);
    	}
    	arv_camera_stop_acquisition(arvCamera);
    	g_object_unref(stream);
		return SUCCESS;
  	}
  	else {
    	return ERROR_SNAPSHOT_FAILED;
  	}
}

/*
	Sets frame rate for camera, returns a status code
*/
int HDCamera::setFrameRate(double frameRate) {
	// must first do a get, otherwise camera doesn't let frame rate write for some reason
	arv_camera_get_frame_rate(arvCamera);

	double min = 10000, max = -1;
	arv_camera_get_frame_rate_bounds(arvCamera, &min, &max);
	if (min > frameRate || max < frameRate) {
		// log error somehow
		printf("Error: frame rate is not within camera bounds [%lf, %lf]\n", min, max);	
		return ERROR_FEATURE_OUT_OF_BOUNDS;
  	}
  	else {
		// log success somehow
		printf("Frame rate successfully passed validation.\n");
  	}

	arv_camera_set_frame_rate(arvCamera, frameRate);

	// make sure value was written
	double readFrameRate = arv_camera_get_frame_rate(arvCamera);
	if (frameRate != readFrameRate) {
 		// log error somehow
		printf("Error: frame rate failed to write\n");
		return ERROR_FEATURE_WRITE_FAILED;
  	}

	// log success
	printf("Frame rate %lf was successfully written to the camera.\n", frameRate);
	return SUCCESS;
}

/*
	Sets gain for camera, returns status code
*/
int HDCamera::setGain(double gain) {
	double min = 10000, max = -1;
  

	// like frame rate, must do an empty get for the gain before the camera lets us change it
	arv_device_get_float_feature_bounds(device, "GainAbs", &min, &max);
  
	if (min > gain || max < gain) {
		// log error somehow
		printf("Error: gain is not within camera bounds [%lf, %lf]\n", min, max);	
		return ERROR_FEATURE_OUT_OF_BOUNDS;
  	}
	else {
		// log success somehow
		printf("Gain successfully passed validation.\n");
  	}

  	arv_device_set_float_feature_value(device, "GainAbs", gain);

	// make sure value was written
	double readFeature = arv_device_get_float_feature_value(device, "GainAbs");
	if (readFeature != gain) {
  		// log error somehow
		printf("Error: gain failed to write\n");
		return ERROR_FEATURE_WRITE_FAILED;
  	}

	// log success
	printf("Gain %lf was successfully written to the camera.\n", gain);
	return SUCCESS;
}

/* 
	Sets region for camera, returns status code.

	Here, x and y are offsets, with the given window width and height 
*/
int HDCamera::setRegion(int x, int y, int width, int height) {
	int min = 10000, max = -1;

	int status = SUCCESS;

	arv_camera_get_x_offset_bounds(arvCamera, &min, &max);
	if (x < min || x > max) {
		printf("Error: x-offset out of bounds [%d, %d]\n", min, max);
		status = ERROR_FEATURE_OUT_OF_BOUNDS;
	}

	arv_camera_get_y_offset_bounds(arvCamera, &min, &max);
	if (y < min || y > max) {
		printf("Error: y-offset out of bounds [%d, %d]\n", min, max);
		status = ERROR_FEATURE_OUT_OF_BOUNDS;
	}
	
	arv_camera_get_width_bounds(arvCamera, &min, &max);
	if (width < min || width > max) {
		printf("Error: width out of bounds [%d, %d]\n", min, max);
		status = ERROR_FEATURE_OUT_OF_BOUNDS;
	}

	arv_camera_get_height_bounds(arvCamera, &min, &max);
	if (height < min || height > max) {
		printf("Error: height out of bounds [%d, %d]\n", min, max);
		status = ERROR_FEATURE_OUT_OF_BOUNDS;
	}

	if (SUCCESS != status) return status;

	printf("Camera region successfully passed validation\n");

	arv_camera_set_region(arvCamera, x, y, width, height);

	int readX, readY, readWidth, readHeight;
	readX = readY = readWidth = readHeight = -1;

	arv_camera_get_region(arvCamera, &readX, &readY, &readWidth, &readHeight);
	if (x == readX && y == readY && width == readWidth && height == readHeight){
		printf("Successfully wrote region (x, y, width, height) = (%d, %d, %d, %d)\n", x, y, width, height);
	} else {
		printf("Error: failed to write region to camera\n");
		status = ERROR_FEATURE_WRITE_FAILED;
	}
	
	return status;
}

/*
	Sets exposure time for camera, returns status code
*/
int HDCamera::setExposureTime(double exposureTime) {
	// must first do a get, otherwise camera doesn't let frame rate write for some reason
	arv_camera_get_exposure_time(arvCamera);

	double min = 10000, max = -1;
	arv_camera_get_exposure_time_bounds(arvCamera, &min, &max);
	if (min > exposureTime || max < exposureTime) {
		// log error somehow
		printf("Error: exposure time is not within camera bounds [%lf, %lf]\n", min, max);
		return ERROR_FEATURE_OUT_OF_BOUNDS;
	}
	else {
		// log success somehow
		printf("Exposure time successfully passed validation.\n");
	}
	
	arv_camera_set_exposure_time(arvCamera, exposureTime);
	
	// make sure value was written
	double readExposureTime = arv_camera_get_exposure_time(arvCamera);
	if (exposureTime != readExposureTime) {
		// log error somehow
		printf("Error: exposure time failed to write\n");
		return ERROR_FEATURE_WRITE_FAILED;
	}

	// log success
	printf("Exposure time %lf was successfully written to the camera.\n", exposureTime);
	return SUCCESS;
}

/*
	Sets pixel format for the camera, returns status code.

	Valid formats for this particular camera are "Mono8" and "Mono16"
*/
int HDCamera::setPixelFormat(const char *pixelFormat) {
	// do empty get: camera doesn't let pixel format write without this get, returns timeout error
	arv_camera_get_pixel_format_as_string(arvCamera);

	guint nPixelFormats = 0;
	const char **pixelFormats = arv_camera_get_available_pixel_formats_as_strings(arvCamera, &nPixelFormats);

	int isPixelFormat = FALSE;
	for (unsigned int i = 0; i < nPixelFormats; i++) {
		printf("%s\n", pixelFormats[i]);
		if (0 == strcmp(pixelFormat, pixelFormats[i])) {
			isPixelFormat = TRUE;
			break;
		}
	}

	if (!isPixelFormat) {
		printf("Error: invalid pixel format [%s]\n", pixelFormat);
		return ERROR_FEATURE_OUT_OF_BOUNDS;
	} else {
		printf("Pixel format passed validation\n");
	}
  
	arv_camera_set_pixel_format_from_string(arvCamera, pixelFormat);

	const char *readPixelFormat = arv_camera_get_pixel_format_as_string(arvCamera);
	if (NULL == readPixelFormat || strcmp(readPixelFormat, pixelFormat) != 0) {
		printf("Error: pixel format failed to write\n");
		return ERROR_FEATURE_WRITE_FAILED;
	}
	printf("Pixel format successfully written to camera\n");
	return SUCCESS;
}

ArvCamera* HDCamera::getArvInstance() { return arvCamera; }
 


