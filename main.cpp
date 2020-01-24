#include "arv.h"
#include "camera.h"
#include "config.h"
#include <iostream>

using namespace std;

// parameters for camera
gint packetSize = 1500;
gint windowHeight = 1024;
gint windowWidth = 1280;
gint frameRate = -1;
gint numFrames = -1;
gboolean snapshot = FALSE;

static const GOptionEntry cameraCommandOptionEntries[] =
{
	{ "packet-size", 'p', 0, G_OPTION_ARG_INT, &packetSize, "Packet size of ethernet connection, in bytes", NULL },
	{ "window-height", 'h', 0, G_OPTION_ARG_INT, &windowHeight, "Window height of recorded images, in pixels", NULL },
	{ "window-width", 'w', 0, G_OPTION_ARG_INT, &windowWidth, "Window width of recorded images, in pixels", NULL },
	{ "frame-rate", 'f', 0, G_OPTION_ARG_INT, &frameRate, "Frame rate of camera stream", NULL },
	{ "num-frames", 'n', 0, G_OPTION_ARG_INT, &numFrames, "Number of frames for camera to capture", NULL },
	{ "snapshot", 's', 0, G_OPTION_ARG_NONE, &snapshot, "Have camera stream a single snapshot", NULL },
	{ NULL }
};

int main(int argc, char *argv[]) {
	GError *error = NULL;
	GOptionContext *context = g_option_context_new("Camera configuration parameters");
	g_option_context_add_main_entries(context, cameraCommandOptionEntries, NULL);
	
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_print("Could not properly parse command line argument: %s\n", error->message);
		return ERROR_COMMAND_LINE_PARSE_FAILED; 
	}
	cout << packetSize << " " << windowHeight << " " << windowWidth << " " << frameRate << " " << numFrames << " " << snapshot << endl;
	
	Camera camera = Camera(NULL);
	return 0;
}
