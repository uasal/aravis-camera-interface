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
	ArvCamera* arvCamera = camera.getArvInstance();
	ArvDevice* device = arv_camera_get_device(arvCamera);
	cout << "prev status " << (arv_device_get_status(device) == ARV_DEVICE_STATUS_SUCCESS) << endl;
	cout << "feature " << arv_device_get_boolean_feature_value(device, "AutoFrameRate") << endl;
	cout << "after status " << (arv_device_get_status(device) == ARV_DEVICE_STATUS_SUCCESS) << endl;

	guint numVals;
	gint64 *featVals = arv_device_get_available_enumeration_feature_values
                               (device,
                                "AutoFrameRate",
                                &numVals);
	cout << "values" << endl;
	for (int i = 0; i < numVals; i++) {
		cout << featVals[i] << endl;	
	}

	//double min, max;
	//arv_device_get_float_feature_bounds(device, "Brightness", &min, &max);
	//cout << "min " << min << " max " << max << endl;

	error = NULL;
	guint32 val;
	gboolean ret = arv_device_read_register (device,
                          0xE8E8,
                          &val,
                          &error);
	if (!ret) cout << "read not successful" << endl;
	else cout << "read successful " << val << endl;

	ret = arv_device_write_register (device,
                          0xE8E8,
                          1,
                          &error);
	if (!ret) {
		cout << "write not successful" << endl;
		cout << error->message << endl;
	}
	else cout << "write successful" << endl;

	ret = arv_device_read_register (device,
                          0xE8E8,
                          &val,
                          &error);
	if (!ret) cout << "read not successful" << endl;
	else cout << "read successful " << val << endl;

	if (snapshotMode) {
		camera.getSnapshot(10000000, 1, &status);
	} else {
		camera.configureAttributes(windowWidth, windowHeight, exposureTime);
		camera.startStream(numFrames, frameRate);
	}
	
	return 0;
}
