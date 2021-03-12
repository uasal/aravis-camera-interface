#include "arv.h"
#include "hdcamera.h"
#include "config.h"
#include <inttypes.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>
// #include <png.h> // Requires libpng1.2
#include <stdio.h>

// int arv_save_png(ArvBuffer * buffer, const char * filename);

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
	
	int status = -1;
	HDCamera camera = HDCamera(&status, ethernetPacketSize);
	if (SUCCESS != status) return status;
	
	status = camera.setFrameRate(frameRate);
	if (SUCCESS != status) return status;
	
	status = camera.setGain(gain);
	if (SUCCESS != status) return status;
	
	status = camera.setRegion(xoffset, yoffset, windowWidth, windowHeight);
	if (SUCCESS != status) return status;

	status = camera.setExposureTime(exposureTime);
	if (SUCCESS != status) return status;

	const char *pixelFormat = togglePixelFormatMono16 ? "Mono16" : "Mono8";
	status = camera.setPixelFormat(pixelFormat);
	if (SUCCESS != status) return status;

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

/*
int arv_save_png(ArvBuffer * buffer, const char * filename)
{
	size_t buffer_size = 0;
	char * buffer_data = (char*)arv_buffer_get_data(buffer, &buffer_size); 
	int width = -1; int height = -1;
	arv_buffer_get_image_region(buffer, NULL, NULL, &width, &height); 
	int bit_depth = ARV_PIXEL_FORMAT_BIT_PER_PIXEL(arv_buffer_get_image_pixel_format(buffer)); 

	int arv_row_stride = width * bit_depth/8; 
	int color_type = PNG_COLOR_TYPE_GRAY; 

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		printf("Error: creation of png write struct failed\n");
		return ERROR_IMAGE_WRITE_FAILURE;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		printf("Error: creation of png info struct failed\n");
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return ERROR_IMAGE_WRITE_FAILURE;
	}

	FILE * f = fopen(filename, "wb");
	if (!f) {
		printf("Error: file %s failed to open for writing\n", filename);
		return ERROR_FILESYSTEM_FAILURE;
	}

	png_init_io(png_ptr, f);
	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
	PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);
	
	// Need to create pointers to each row of pixels for libpng
	png_bytepp rows = (png_bytepp)(png_malloc(png_ptr, height*sizeof(png_bytep)));
	for (int i = 0; i < height; ++i) {
		rows[i] = (png_bytep)(buffer_data + (height - i)*arv_row_stride);
	}

	png_write_image(png_ptr, rows);

	png_write_end(png_ptr, NULL);
	png_free(png_ptr, rows);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(f);
	return SUCCESS;
}
*/