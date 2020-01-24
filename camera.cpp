#include "glib.h"
#include "arv.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <png.h> // Requires libpng1.2
#include <assert.h>
#include "camera.h"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>

void arv_save_png(ArvBuffer * buffer, const char * filename);

using namespace std;

static gboolean cancel = FALSE;

static void
set_cancel (int signal)
{
    cancel = TRUE;
}

int filenum = 0;
int calledTimes = 0;
static void
new_buffer_cb (ArvStream *stream, ApplicationData *data)
{
    ArvBuffer *buffer;
    calledTimes++;
    
    buffer = arv_stream_try_pop_buffer (stream);

    if (buffer != NULL) {
	ArvBufferStatus status = arv_buffer_get_status (buffer);
        if (status == ARV_BUFFER_STATUS_SUCCESS){
            data->buffer_count++;
	    data->totalBufferCount++;

	    //cout << "type " << arv_buffer_get_payload_type(buffer) << endl;
	    /* Image processing here */
	    char name[20];
	    sprintf(name, "%d.png", filenum++);

	    //arv_save_png(buffer, name); // segfault

	    //size_t buffer_size;
	    //char * buffer_data = (char*)arv_buffer_get_data(buffer, &buffer_size); // raw data
	    
	    if (data->totalBufferCount == data->maxBufferCount) {
		g_main_loop_quit(data->main_loop);
		data->done = TRUE;
	    }
	}

	//printf("status %d\n", status == ARV_BUFFER_STATUS_TIMEOUT);
	arv_stream_push_buffer (stream, buffer);
    }
}

static gboolean
periodic_task_cb (void *abstract_data)
{
    ApplicationData *data = (ApplicationData*) abstract_data;
    
    printf ("Frame rate = %d Hz\n", data->buffer_count);
    data->buffer_count = 0;

    if (cancel) {
        g_main_loop_quit (data->main_loop);
        return FALSE;
    }

    return !data->done;
}

static void
control_lost_cb (ArvGvDevice *gv_device)
{
    /* Control of the device is lost. Display a message and force application exit */
    printf ("Control lost\n");

    cancel = TRUE;
}

/**
 * Reads image data from an Aravis ArvBuffer and saves a png file to filename
 * TODO: Add error checking and all that stuff (this code is demonstrative)
 */
void arv_save_png(ArvBuffer * buffer, const char * filename)
{
    // TODO: This only works on image buffers
    assert(arv_buffer_get_payload_type(buffer) == ARV_BUFFER_PAYLOAD_TYPE_IMAGE);

    size_t buffer_size;
    char * buffer_data = (char*)arv_buffer_get_data(buffer, &buffer_size); // raw data
    int width; int height;
    arv_buffer_get_image_region(buffer, NULL, NULL, &width, &height); // get width/height
    int bit_depth = ARV_PIXEL_FORMAT_BIT_PER_PIXEL(arv_buffer_get_image_pixel_format(buffer)); // bit(s) per pixel
    //TODO: Deal with non-png compliant pixel formats?
    // EG: ARV_PIXEL_FORMAT_MONO_14 is 14 bits per pixel, so conversion to PNG loses data
    int arv_row_stride = width * bit_depth/8; // bytes per row, for constructing row pointers
    int color_type = PNG_COLOR_TYPE_GRAY; //TODO: Check for other types?
    // boilerplate libpng stuff without error checking (setjmp? Seriously? How many kittens have to die?)
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
    // Actually write image
cout << "here" << endl;
    png_write_image(png_ptr, rows); // segfault line
    cout << "here1" << endl;
    png_write_end(png_ptr, NULL); // cleanup
    png_free(png_ptr, rows);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(f);
}

Camera::Camera(int *status, char *name) {
    arvCamera = arv_camera_new(name);
    if (arvCamera == NULL) {
	char errorMsg[50];
	sprintf(errorMsg, "Camera %s not found", name ? name : "");
	throw std::runtime_error(errorMsg);
    }
    stream = NULL;
    arv_camera_gv_set_packet_size(arvCamera, 1500); // necessary for acceptable performance
    parser = arv_camera_create_chunk_parser(arvCamera);
    arv_camera_set_chunks(arvCamera, "Width,Height");
    arv_camera_set_chunk_mode(arvCamera, true);
    ArvDevice *device = arv_camera_get_device(arvCamera);
    
    gint64 min;
    gint64 max;
    arv_device_get_integer_feature_bounds(device, "Zoom", &min, &max);
    cout << "min " << min << " max " << max << endl;
    arv_device_set_integer_feature_value(device, "Zoom", 2);
    
    GSocketAddress *ip = arv_gv_device_get_device_address(ARV_GV_DEVICE (device));
    gssize ipsize = g_socket_address_get_native_size(ip);
    gpointer res = malloc(ipsize);
    gboolean rc = g_socket_address_to_native(ip, res, ipsize, NULL);
    cout << "success " << rc << endl;
    struct sockaddr *ipstruct = (struct sockaddr*) res;
    cout << "ip data " << ipstruct->sa_data << endl;
    
    char *s = NULL;
    switch(ipstruct->sa_family) {
	case AF_INET: {
	    struct sockaddr_in *addr_in = (struct sockaddr_in *)res;
	    s = (char*) malloc(INET_ADDRSTRLEN);
	    inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
	    break;
	}
	case AF_INET6: {
	    struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)res;
	    s = (char*) malloc(INET6_ADDRSTRLEN);
	    inet_ntop(AF_INET6, &(addr_in6->sin6_addr), s, INET6_ADDRSTRLEN);
	    break;
	}
	default:
	    break;
    }
    printf("IP address: %s\n", s);
    free(s);
    free(res);
    
    cout << "zoom " << arv_device_get_integer_feature_value(device, "Zoom") << endl;
}

Camera::~Camera() {
    g_clear_object(&arvCamera);
}

void Camera::configureStream(float frameRate, gint windowWidth, gint windowHeight) {
    ArvCamera *camera = arvCamera;
    data.buffer_count = 0;

    gint minH, maxH, minW, maxW;
    arv_camera_get_height_bounds(camera, &minH, &maxH);
    arv_camera_get_width_bounds(camera, &minW, &maxW);
    gint x = maxW/2 - windowWidth/2;
    gint y = maxH/2 - windowHeight/2;
    gint width = maxW/2 + windowWidth/2;
    gint height = maxH/2 + windowHeight/2;
    cout << "windowwidth " << windowWidth << " windowHeight " << windowHeight;
    cout << "x " << x << " y " << y << " width " << width << " height " << height << endl;
    arv_camera_set_region (camera, x, y, width, height);
    /* Set frame rate to 10 Hz */
    arv_camera_set_frame_rate (camera, frameRate);

    /* Create a new stream object */
    stream = arv_camera_create_stream (camera, NULL, NULL);
}

void Camera::startStream(int maxBufferCount) {
    int i;
    cancel = FALSE;
    /* retrieve image payload (number of bytes per image) */
    gint payload = arv_camera_get_payload (arvCamera);
    data.maxBufferCount = maxBufferCount;
    data.totalBufferCount = 0;
    data.done = FALSE;
    if (stream != NULL) {
	/* Push 50 buffer in the stream input buffer queue */
	for (i = 0; i < 50; i++)
	    arv_stream_push_buffer (stream, arv_buffer_new (payload, NULL));

	arv_camera_set_acquisition_mode (arvCamera, ARV_ACQUISITION_MODE_CONTINUOUS);

	/* Start the video stream */
	arv_camera_start_acquisition (arvCamera);
		
	/* Connect the new-buffer signal */
	g_signal_connect (stream, "new-buffer", G_CALLBACK (new_buffer_cb), &data);
	/* And enable emission of this signal (it's disabled by default for performance reason) */
	arv_stream_set_emit_signals (stream, TRUE);
	
	/* Connect the control-lost signal */
	g_signal_connect (arv_camera_get_device (arvCamera), "control-lost",
	G_CALLBACK (control_lost_cb), NULL);

	/* Install the callback for frame rate display */
	g_timeout_add_seconds (1, periodic_task_cb, &data);
		
	/* Create a new glib main loop */
	data.main_loop = g_main_loop_new (NULL, FALSE);
	void (*old_sigint_handler)(int) = signal (SIGINT, set_cancel);

	/* Run the main loop */
	g_main_loop_run (data.main_loop);
	
	// stream cleanup
	cout << "files " << filenum << endl;

	signal (SIGINT, old_sigint_handler);
	g_main_loop_unref (data.main_loop);
	
	// Stop the video stream 
	arv_camera_stop_acquisition (arvCamera);
	// Signal must be inhibited to avoid stream thread running after the last unref 
	arv_stream_set_emit_signals (stream, FALSE);
		
	//system("ffmpeg -r 10 -f image2 -i %d.png -vcodec libx264 -crf 25 -pix_fmt yuv420p test.mp4");
    } else
	printf ("Stream thread was unintialized (check if the device is not already used, or if stream was configured)\n");
}

void Camera::stopStream() { set_cancel(TRUE); }
void Camera::freeStream() { g_object_unref(stream); }

ArvBuffer* Camera::getSnapshot() {
    ArvBuffer *buffer = arv_camera_acquisition(arvCamera, 0);
    if (ARV_IS_BUFFER(buffer)) {
	arv_save_png(buffer, "capture.png");
    }
    return buffer;
}

ArvCamera* Camera::getArvInstance() { return arvCamera; }

/*
 * Private methods
 */
 


