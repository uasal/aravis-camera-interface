#include "glib.h"
#include "arv.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <png.h> // Requires libpng1.2
#include <assert.h>
#include "camera.h"

void arv_save_png(ArvBuffer * buffer, const char * filename);

typedef struct {
    GMainLoop *main_loop;
    int buffer_count;
} ApplicationData;

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
        if (arv_buffer_get_status (buffer) == ARV_BUFFER_STATUS_SUCCESS)
            data->buffer_count++;
        /* Image processing here */
        char name[20];
        sprintf(name, "%d.png", filenum++);
        arv_save_png(buffer, name);
        if (filenum == 100) g_main_loop_quit(data->main_loop);
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

    return TRUE;
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
	png_write_image(png_ptr, rows);
	png_write_end(png_ptr, NULL); // cleanup
	png_free(png_ptr, rows);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(f);
}

Camera::Camera(char *name) {
	arvCamera = arv_camera_new(name);
	if (arvCamera == NULL) {
		char errorMsg[50];
		sprintf(errorMsg, "Camera %s not found", name ? name : "");
		throw std::runtime_error("Camera not found.");
	}
}

Camera::~Camera() {
	g_clear_object(&arvCamera);
}

void Camera::saveVideo(float frameRate, int windowWidth, int windowHeight) {
	ArvCamera *camera = arvCamera;
	ArvStream *stream;
	ApplicationData data;

	void (*old_sigint_handler)(int);
    gint payload;
    int i;
    
    /* Set region of interrest to a 200x200 pixel area */
    arv_camera_set_region (camera, 0, 0, windowWidth, windowHeight);
    /* Set frame rate to 10 Hz */
    arv_camera_set_frame_rate (camera, frameRate);
    /* retrieve image payload (number of bytes per image) */
    payload = arv_camera_get_payload (camera);
	
	/* Create a new stream object */
	stream = arv_camera_create_stream (camera, NULL, NULL);
	if (stream != NULL) {
		/* Push 50 buffer in the stream input buffer queue */
		for (i = 0; i < 100; i++)
			arv_stream_push_buffer (stream, arv_buffer_new (payload, NULL));

		/* Start the video stream */
		arv_camera_start_acquisition (camera);
		
		/* Connect the new-buffer signal */
		g_signal_connect (stream, "new-buffer", G_CALLBACK (new_buffer_cb), &data);
		/* And enable emission of this signal (it's disabled by default for performance reason) */
		arv_stream_set_emit_signals (stream, TRUE);
		
		/* Connect the control-lost signal */
		g_signal_connect (arv_camera_get_device (camera), "control-lost",
			G_CALLBACK (control_lost_cb), NULL);

		/* Install the callback for frame rate display */
		g_timeout_add_seconds (1, periodic_task_cb, &data);
		
		/* Create a new glib main loop */
		data.main_loop = g_main_loop_new (NULL, FALSE);
		old_sigint_handler = signal (SIGINT, set_cancel);
		
		/* Run the main loop */
		g_main_loop_run (data.main_loop);
		
		signal (SIGINT, old_sigint_handler);
		g_main_loop_unref (data.main_loop);

		ArvBuffer *buffer;
		int successes = 0;
		int failures = 0;
		printf("made it here\n");
		do {
			
			buffer = arv_stream_timeout_pop_buffer (stream, 10000000);
			if (buffer != NULL) {
				if (arv_buffer_get_status (buffer) == ARV_BUFFER_STATUS_SUCCESS)
					successes++;
				else failures++;
			}
		} while (buffer != NULL);
		printf("successes %d failures %d\n", successes, failures);
		printf("called %d\n", calledTimes);
		/* Stop the video stream */
		arv_camera_stop_acquisition (camera);
		/* Signal must be inhibited to avoid stream thread running after the last unref */
		arv_stream_set_emit_signals (stream, FALSE);
		
		system("ffmpeg -r 10 -f image2 -i %d.png -vcodec libx264 -crf 25 -pix_fmt yuv420p test.mp4");

        g_object_unref (stream);
    } else
		printf ("Can't create stream thread (check if the device is not already used)\n");

}
