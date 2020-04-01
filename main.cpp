#include "arv.h"
#include "camera.h"
#include "config.h"
#include <iostream>

using namespace std;

// parameters for camera
gint ethernetPacketSize = DEFAULT_ETHERNET_PACKET_SIZE;
gint windowHeight = DEFAULT_CAMERA_WINDOW_HEIGHT;
gint windowWidth = DEFAULT_CAMERA_WINDOW_WIDTH;
gint frameRate = DEFAULT_CAMERA_FRAME_RATE;
gint numFrames = FEATURE_NOT_DEFINED;
gdouble exposureTime = FEATURE_NOT_DEFINED;
gboolean snapshotMode = FALSE;
gboolean acceptDataFromCamera = FALSE;


static const GOptionEntry cameraCommandOptionEntries[] =
{
	{ "packet-size", 'p', 0, G_OPTION_ARG_INT, &ethernetPacketSize, "Packet size of ethernet connection, in bytes", NULL },
	{ "window-height", 'h', 0, G_OPTION_ARG_INT, &windowHeight, "Window height of recorded images, in pixels", NULL },
	{ "window-width", 'w', 0, G_OPTION_ARG_INT, &windowWidth, "Window width of recorded images, in pixels", NULL },
	{ "frame-rate", 'f', 0, G_OPTION_ARG_INT, &frameRate, "Frame rate of camera stream", NULL },
	{ "num-frames", 'n', 0, G_OPTION_ARG_INT, &numFrames, "Number of frames for camera to capture", NULL },
	{ "snapshot-mode", 's', 0, G_OPTION_ARG_NONE, &snapshotMode, "Have camera stream a single snapshot", NULL },
	{ "exposure-time", 't', 0, G_OPTION_ARG_DOUBLE, &exposureTime, "Exposure time, in microseconds", NULL},
	{ NULL }
};

int main(int argc, char *argv[]) {
	
	// parse command
	GError *error = NULL;
	GOptionContext *context = g_option_context_new("Camera configuration parameters");
	g_option_context_add_main_entries(context, cameraCommandOptionEntries, NULL);
	
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_print("Could not properly parse command line argument: %s\n", error->message);
		return ERROR_COMMAND_LINE_PARSE_FAILED; 
	}
	
	int status;
	Camera camera = Camera(&status, ethernetPacketSize);
	if (SUCCESS != status) return status;

	cout << "made it here" << endl;
	if (snapshotMode) {
		camera.getSnapshot(10000000, 1, &status);
	} else {
		camera.configureAttributes(windowWidth, windowHeight, exposureTime);
		camera.startStream(numFrames, frameRate);
	}
	
	return 0;
}
