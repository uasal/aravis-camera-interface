#include "arv.h"
#include "hdcamera.h"
#include "config.h"
#include <inttypes.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

using namespace std;

// parameters for camera
gint ethernetPacketSize = DEFAULT_ETHERNET_PACKET_SIZE;
gint windowHeight = DEFAULT_CAMERA_WINDOW_HEIGHT;
gint windowWidth = DEFAULT_CAMERA_WINDOW_WIDTH;
gdouble frameRate = DEFAULT_CAMERA_FRAME_RATE;
guint64 streamTime = FEATURE_NOT_DEFINED;
gdouble exposureTime = DEFAULT_CAMERA_EXPOSURE_TIME;
gboolean snapshotMode = FALSE;
guint64 snapshotTimeout = DEFAULT_CAMERA_SNAPSHOT_TIMEOUT;
gboolean toggleObtainSnapshotBuffer = FALSE;
gboolean acceptDataFromCamera = FALSE;
gdouble gain = DEFAULT_CAMERA_GAIN;
gint xoffset = DEFAULT_CAMERA_X_OFFSET;
gint yoffset = DEFAULT_CAMERA_Y_OFFSET;
gboolean togglePixelFormatMono16 = FALSE;
gboolean failOnFeatureIOError = FALSE;

/*
	Definition of command line entries. Each describes a command line option to be read into the given 		variable. If an option is not presented, the default value is retained. The second argument (type 		char) indicates the short hand alias for the option.

	Example:

	hdcamera --frame-rate 15 --stream-time 100
	hdcamera -f 15 -d 100

	The above two commands are equivalent.
*/
static const GOptionEntry cameraCommandOptionEntries[] =
{
	{ "packet-size", 'p', 0, G_OPTION_ARG_INT, &ethernetPacketSize, "Packet size of ethernet connection, in bytes", NULL },
	{ "window-height", 'h', 0, G_OPTION_ARG_INT, &windowHeight, "Window height of recorded images, in pixels", NULL },
	{ "window-width", 'w', 0, G_OPTION_ARG_INT, &windowWidth, "Window width of recorded images, in pixels", NULL },
	{ "frame-rate", 'f', 0, G_OPTION_ARG_DOUBLE, &frameRate, "Frame rate of camera stream", NULL },
	{ "stream-time", 'd', 0, G_OPTION_ARG_INT64, &streamTime, "Time that the camera streams for, in seconds (not equivalent to the video duration, which depends on performance of the camera and the datarate of the ethernet connection)", NULL },
	{ "snapshot-mode", 's', 0, G_OPTION_ARG_NONE, &snapshotMode, "Have camera stream a single snapshot", NULL },
	{ "snapshot-timeout", 'o', 0, G_OPTION_ARG_INT64, &snapshotTimeout, "Time that the camera streams the snapshot for, in microseconds, needs to be long enough for the snapshot to be transmitted from the camera", NULL },
	{ "get-buffer", 'b', 0, G_OPTION_ARG_NONE, &toggleObtainSnapshotBuffer, "Have snapshot buffer saved to file system", NULL },
	{ "exposure-time", 't', 0, G_OPTION_ARG_DOUBLE, &exposureTime, "Exposure time, in microseconds", NULL},
	{ "gain", 'g', 0, G_OPTION_ARG_DOUBLE, &gain, "Camera gain", NULL},
	{ "x-offset", 'x', 0, G_OPTION_ARG_INT, &xoffset, "Camera x-offset, in pixels", NULL},
	{ "y-offset", 'y', 0, G_OPTION_ARG_INT, &yoffset, "Camera y-offset, in pixels", NULL},
	{ "mono-16", 'm', 0, G_OPTION_ARG_NONE, &togglePixelFormatMono16, "Makes pixel format mono 16, default is mono 8", NULL },
	{ "fail-on-feature-error", 'e', 0, G_OPTION_ARG_NONE, &failOnFeatureIOError, "Causes program to terminate whenever a feature write fails, default off", NULL},
    { NULL }
};


int main(int argc, char *argv[]) {
	cout << "Starting camera application..." << endl;

	// parse command
	GError *error = NULL;
	GOptionContext *context = g_option_context_new("Camera configuration parameters");
	g_option_context_add_main_entries(context, cameraCommandOptionEntries, NULL);
	
	cout << "Created command line option context..." << endl;
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_print("Could not properly parse command line argument: %s\n", error->message);
		return ERROR_COMMAND_LINE_PARSE_FAILED; 
	}
	
	cout << "Parsed command line context, connecting with camera..." << endl;

	int status = -1;
	HDCamera camera = HDCamera(&status, ethernetPacketSize);
	if (SUCCESS != status) return status;
	
	status = camera.setFrameRate(frameRate);
	if (SUCCESS != status && failOnFeatureIOError) return status;
	
	status = camera.setGain(gain);
	if (SUCCESS != status && failOnFeatureIOError) return status;
	
	status = camera.setRegion(xoffset, yoffset, windowWidth, windowHeight);
	if (SUCCESS != status && failOnFeatureIOError) return status;

	status = camera.setExposureTime(exposureTime);
	if (SUCCESS != status && failOnFeatureIOError) return status;

	const char *pixelFormat = togglePixelFormatMono16 ? "Mono16" : "Mono8";
	status = camera.setPixelFormat(pixelFormat);
	if (SUCCESS != status && failOnFeatureIOError) return status;

	if (snapshotMode) {
		ArvBuffer *buffer = NULL;
		status = camera.getSnapshot(snapshotTimeout, toggleObtainSnapshotBuffer ? &buffer : NULL);
		if (SUCCESS != status) return status;

		if (NULL != buffer) {
			// status = arv_save_png(buffer, "buffer.png");
			if (SUCCESS != status) return status;
		}
	} else {
		if (FEATURE_NOT_DEFINED == streamTime) {
			printf("Error: camera stream duration must be specified for video streaming\n");
			return ERROR_STREAM_DURATION_UNDEFINED;
		}

		status = camera.startStream(streamTime);
		if (SUCCESS != status) return status;
	}
	
	return SUCCESS;
}
