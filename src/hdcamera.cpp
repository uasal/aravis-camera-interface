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

#define BUFFER_CAPACITY 2

using namespace std;

typedef struct {
	GMainLoop *mainLoop;
	int bufferCount;
	int totalBufferCount;
} ApplicationData;

static gboolean cancel = FALSE;

static void setCancel(int signal)
{
	cancel = TRUE;
}

static void newBufferCallback(ArvStream *stream, ApplicationData *data)
{
	ArvBuffer *buffer = arv_stream_try_pop_buffer (stream);
	if (buffer != NULL) {
		if (arv_buffer_get_status (buffer) == ARV_BUFFER_STATUS_SUCCESS)
			data->bufferCount++;
			data->totalBufferCount++;
		/* Image processing here */
		arv_stream_push_buffer (stream, buffer);
	}
}

static gboolean periodicTaskCallback(void *abstractData)
{
	ApplicationData *data = (ApplicationData*) abstractData;

	printf ("Frame rate = %d Hz\n", data->bufferCount);
	data->bufferCount = 0;

	if (cancel) {
		g_main_loop_quit (data->mainLoop);
		return FALSE;
	}

	return TRUE;
}

static gboolean streamEndCallback(void *appData) {
	ApplicationData *data = (ApplicationData*) appData;
	g_main_loop_quit(data->mainLoop);
	
	return FALSE; // should never get here
}

static void controlLostCallback(ArvGvDevice *gvDevice)
{
	/* Control of the device is lost. Display a message and force application exit */
	printf ("Control lost\n");

	cancel = TRUE;
}

/*
	Parameters: status code, packet size of UDP data packets, optional name identifier for the camera
 */
HDCamera::HDCamera(int *status, int packetSize, char *name) {
	*status = SUCCESS;
	created = false;
	
	arvCamera = arv_camera_new(name);
	if (!ARV_IS_CAMERA(arvCamera)) {
		*status = ERROR_CAMERA_NOT_FOUND;
	} else {
		device = arv_camera_get_device(arvCamera); // TODO: Error check this line?
		arv_camera_gv_set_packet_size(arvCamera, packetSize);
		arv_camera_set_trigger(arvCamera, "Software");
		created = true;
  	}
}

HDCamera::~HDCamera() {
	if (created) {		
		g_clear_object(&device);
		g_clear_object(&arvCamera);
	}
}

/*
	Starts stream for the camera.
	
	Parameters: duration for the stream, not equivalent to the video duration, since actual 
	fps depends on camera and ethernet performance

	Returns: status code	
 */
int HDCamera::startStream(guint64 duration) {
	void (*oldSigintHandler)(int) = NULL;
	gint payload = arv_camera_get_payload (arvCamera);

	ArvStream *stream = arv_camera_create_stream (arvCamera, NULL, NULL);
	if (!ARV_IS_STREAM (stream)) {
		printf ("Stream thread was unintialized (check if the device is not already used, or if stream was configured)\n");
		return ERROR_STREAM_START_FAILED;
	}

	for (int i = 0; i < BUFFER_CAPACITY; i++) {
		arv_stream_push_buffer (stream, arv_buffer_new (payload, NULL));
	}

	ApplicationData data = { NULL, 0 };

	arv_camera_start_acquisition (arvCamera);

	// Connect the new-buffer signal
	g_signal_connect (stream, "new-buffer", G_CALLBACK (newBufferCallback), &data);
		
	// enable emission of this signal (it's disabled by default for performance reason) 
	arv_stream_set_emit_signals (stream, TRUE);

	// Connect the control-lost signal 
	g_signal_connect (arv_camera_get_device (arvCamera), "control-lost", G_CALLBACK (controlLostCallback), NULL);

	// Install the callback for frame rate display
	g_timeout_add_seconds (1, periodicTaskCallback, &data);

	// End the stream after the given duration has passed
	g_timeout_add_seconds (duration, streamEndCallback, &data);	

	// Create a new glib main loop 
	data.mainLoop = g_main_loop_new (NULL, FALSE);

	oldSigintHandler = signal (SIGINT, setCancel);
	g_main_loop_run (data.mainLoop);
	signal (SIGINT, oldSigintHandler);

	g_main_loop_unref (data.mainLoop);
	arv_camera_stop_acquisition (arvCamera);

	// Signal must be inhibited to avoid stream thread running after the last unref
	arv_stream_set_emit_signals (stream, FALSE);

	g_object_unref (stream);
	return SUCCESS;
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
	if (NULL == stream) {
		return ERROR_SNAPSHOT_FAILED;
	}

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
	arv_device_get_float_feature_bounds(device, FEATURE_GAIN, &min, &max);
  
	if (min > gain || max < gain) {
		// log error somehow
		printf("Error: gain is not within camera bounds [%lf, %lf]\n", min, max);	
		return ERROR_FEATURE_OUT_OF_BOUNDS;
  	}
	else {
		// log success somehow
		printf("Gain successfully passed validation.\n");
  	}

  	arv_device_set_float_feature_value(device, FEATURE_GAIN, gain);

	// make sure value was written
	double readFeature = arv_device_get_float_feature_value(device, FEATURE_GAIN);
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




