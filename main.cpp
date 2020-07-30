#include "arv.h"
#include "hdcamera.h"
#include "config.h"
#include <inttypes.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include <stdio.h>

using namespace std;

// parameters for camera
gint ethernetPacketSize = DEFAULT_ETHERNET_PACKET_SIZE;
gint windowHeight = DEFAULT_CAMERA_WINDOW_HEIGHT;
gint windowWidth = DEFAULT_CAMERA_WINDOW_WIDTH;
gdouble frameRate = DEFAULT_CAMERA_FRAME_RATE;
gint numFrames = FEATURE_NOT_DEFINED;
gdouble exposureTime = DEFAULT_CAMERA_EXPOSURE_TIME;
gboolean snapshotMode = FALSE;
gboolean acceptDataFromCamera = FALSE;
gdouble gain = DEFAULT_CAMERA_GAIN;
gint xoffset = DEFAULT_CAMERA_X_OFFSET;
gint yoffset = DEFAULT_CAMERA_Y_OFFSET;
gboolean togglePixelFormatMono16 = FALSE;

static const GOptionEntry cameraCommandOptionEntries[] =
{
	{ "packet-size", 'p', 0, G_OPTION_ARG_INT, &ethernetPacketSize, "Packet size of ethernet connection, in bytes", NULL },
	{ "window-height", 'h', 0, G_OPTION_ARG_INT, &windowHeight, "Window height of recorded images, in pixels", NULL },
	{ "window-width", 'w', 0, G_OPTION_ARG_INT, &windowWidth, "Window width of recorded images, in pixels", NULL },
	{ "frame-rate", 'f', 0, G_OPTION_ARG_DOUBLE, &frameRate, "Frame rate of camera stream", NULL },
	{ "num-frames", 'n', 0, G_OPTION_ARG_INT, &numFrames, "Number of frames for camera to capture", NULL },
	{ "snapshot-mode", 's', 0, G_OPTION_ARG_NONE, &snapshotMode, "Have camera stream a single snapshot", NULL },
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

	cout << "made it here" << endl;
	ArvCamera* arvCamera = camera.getArvInstance();
	ArvDevice* device = arv_camera_get_device(arvCamera);
/*
	const char* id = arv_camera_get_device_id(arvCamera);
	const char* vendor = arv_camera_get_vendor_name(arvCamera);
	const char* model = arv_camera_get_model_name(arvCamera);

	arv_camera_set_region(arvCamera,0,0,1280,1024);

	const char* format = "Mono8";
	arv_camera_set_pixel_format_from_string(arvCamera,format);

	const char* pixFormat = arv_camera_get_pixel_format_as_string(arvCamera);
	printf("Pixel format: %s\n", pixFormat);

	guint unsigned_x;
	const char** pixFormats = arv_camera_get_available_pixel_formats_as_display_names(arvCamera,&unsigned_x);
	printf("Pixel formats available: %s, %s\n", pixFormats[0], pixFormats[1]);

	double frame_rate = arv_camera_get_frame_rate(arvCamera);
	printf("frame rate: %lf\n",frame_rate);
	
	double max2,min2;
	arv_camera_get_frame_rate_bounds(arvCamera,&min2,&max2);
	printf("frame rate bounds:  min: %lf, max: %lf\n", min2, max2);

	srand(time(NULL));
 	double val1 = rand() % 10 + min2;
	arv_camera_set_frame_rate(arvCamera, val1);
	frame_rate = arv_camera_get_frame_rate(arvCamera);
	printf("frame rate: %lf\n",frame_rate);

	srand (time(NULL));
	double fr = rand() % 10 + 5.0;
	cout << "set val " << fr << endl;
	arv_camera_set_frame_rate(arvCamera, fr);
	sleep(5);
	cout << "fr " << arv_camera_get_frame_rate(arvCamera) << endl;
	cout << "feat " << arv_camera_get_gain(arvCamera) << endl;
	arv_device_set_float_feature_value(device, "GainAbs", 2);
	cout << "prev status " << (arv_device_get_status(device) == ARV_DEVICE_STATUS_SUCCESS) << endl;
	cout << "feature " << arv_device_get_float_feature_value(device, "GainAbs") << endl;
	cout << "after status " << (arv_device_get_status(device) == ARV_DEVICE_STATUS_SUCCESS) << endl;
	*/
	/*
	guint numVals;
	gint64 *featVals = arv_device_get_available_enumeration_feature_values
                               (device,
                                "AutoFrameRate",
                                &numVals);
	cout << "values" << endl;
	for (int i = 0; i < numVals; i++) {
		cout << featVals[i] << endl;	
	}
	*/
	//double min, max;
	//arv_device_get_float_feature_bounds(device, "Brightness", &min, &max);
	//cout << "min " << min << " max " << max << endl;

	/*
	error = NULL;
	guint32 val;
	gboolean ret;
	double expTime = -3000;

	ret = arv_device_read_register (device,
                          0xE820,
                          &val,
                          &error);
	if (!ret) cout << "read not successful" << endl;
	else cout << "read successful " << val << endl;

	ret = arv_device_write_register (device,
                          0xE820,
                          22,
                          &error);
	if (!ret) {
		cout << "write not successful" << endl;
		cout << error->message << endl;
	}
	else cout << "write successful" << endl;

	expTime = arv_camera_get_exposure_time(arvCamera);
	cout << "exposure time " << expTime << endl;
	arv_camera_set_exposure_time(arvCamera, 4000);
	expTime = arv_camera_get_exposure_time(arvCamera);
	cout << "exposure time " << expTime << endl;

	camera.~Camera();
	exit(0);
	*/
/*
	ret = arv_device_read_register (device,
                          0xE8E8,
                          &val,
                          &error);
	if (!ret) cout << "read not successful" << endl;
	else cout << "read successful " << val << endl;
*/
	if (snapshotMode) {
		status = camera.getSnapshot(10000000, NULL);
	} else {
		status = camera.startStream(30);
	}
	
	return 0;
}


/*
void arv_save_png(ArvBuffer * buffer, const char * filename)
{
    assert(arv_buffer_get_payload_type(buffer) == ARV_BUFFER_PAYLOAD_TYPE_IMAGE);

    size_t buffer_size;
    char * buffer_data = (char*)arv_buffer_get_data(buffer, &buffer_size); 
    int width; int height;
    arv_buffer_get_image_region(buffer, NULL, NULL, &width, &height); 
    int bit_depth = ARV_PIXEL_FORMAT_BIT_PER_PIXEL(arv_buffer_get_image_pixel_format(buffer)); 

    int arv_row_stride = width * bit_depth/8; 
    int color_type = PNG_COLOR_TYPE_GRAY; 

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    FILE * f = fopen(filename, "wb");
    png_init_io(png_ptr, f);
    png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);
	
    // Need to create pointers to each row of pixels for libpng
    png_bytepp rows = (png_bytepp)(png_malloc(png_ptr, height*sizeof(png_bytep)));
    int i =0;
    for (i = 0; i < height; ++i)
	rows[i] = (png_bytep)(buffer_data + (height - i)*arv_row_stride);
    // write image
cout << "here" << endl;
    png_write_image(png_ptr, rows); // segfault line
    cout << "here1" << endl;
    png_write_end(png_ptr, NULL); // cleanup
    png_free(png_ptr, rows);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(f);
}*/
