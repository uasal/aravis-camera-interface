/*
  Init the crap out of everything.
  Contents on left for == 
  No more void *.
  Move all commented test code in whatever.test.cpp
*/

#include "glib.h"
#include "arv.h"
#include "arvgvstream.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
//#include <png.h> // Requires libpng1.2
#include <assert.h>
#include "camera.h"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "config.h"

void arv_save_png(ArvBuffer * buffer, const char * filename);

using namespace std;

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

Camera::Camera(int *status, int packetSize, char *name) {
  *status = SUCCESS;

	arvCamera = arv_camera_new(name);
  if (arvCamera == NULL) {
    *status = ERROR_CAMERA_NOT_FOUND;
  } else {
		arv_camera_gv_set_packet_size(arvCamera, packetSize);
    arv_camera_set_trigger(arvCamera, "Software");
    /*
		ArvDevice *device = arv_camera_get_device(arvCamera);
		cout << "sensor temperature: " << arv_device_get_float_feature_value(device, "SensorTemperature") << endl;
	  */
  }
}

Camera::~Camera() {
    g_clear_object(&arvCamera);
}

void Camera::configureAttributes(gint windowWidth, gint windowHeight, double exposureTime) {
	arv_camera_set_region(arvCamera, 0, 0, windowWidth, windowHeight);
	if (FEATURE_NOT_DEFINED != exposureTime) arv_camera_set_exposure_time(arvCamera, exposureTime);
}

void Camera::startStream(int maxBufferCount, float frameRate) {
    ArvCamera *camera = arvCamera;

    arv_camera_set_frame_rate (camera, frameRate);
    arv_camera_set_trigger(camera, "Software");

    ArvStream *stream = arv_camera_create_stream(arvCamera, NULL, NULL);
    if (NULL == stream) {
      cout << "stream creation failed" << endl;
      return;
    }
	int i;

    // retrieve image payload (number of bytes per image) 
    gint payload = arv_camera_get_payload (arvCamera);
    
    if (stream != NULL) {
	// Push 50 buffer in the stream input buffer queue 
	for (i = 0; i < 50; i++)
	    arv_stream_push_buffer (stream, arv_buffer_new (payload, NULL));

	arv_camera_set_frame_count (arvCamera, maxBufferCount);
	//arv_camera_set_acquisition_mode (arvCamera, ARV_ACQUISITION_MODE_MULTI_FRAME);
  arv_camera_set_acquisition_mode(arvCamera, ARV_ACQUISITION_MODE_CONTINUOUS);
	// Start the video stream 
	arv_camera_start_acquisition (arvCamera);
	arv_stream_set_emit_signals(stream, TRUE);
  cout << "here" << endl;
   
  while (1) {
    sleep(10);
    cout << "made it here" << endl;
		//arv_camera_software_trigger(camera);
		ArvBuffer *buffer = arv_stream_timeout_pop_buffer (stream, 2000);
		if (!buffer) cout << "no buffer" << endl;
    else cout << "buffer found" << endl;
	}
	//system("ffmpeg -r 10 -f image2 -i %d.png -vcodec libx264 -crf 25 -pix_fmt yuv420p test.mp4");
  arv_camera_stop_acquisition(arvCamera);
    } else
	printf ("Stream thread was unintialized (check if the device is not already used, or if stream was configured)\n");
 
}

void Camera::stopStream() { }
void Camera::freeStream() { }

ArvBuffer* Camera::getSnapshot(guint64 timeout, int toggleDataRetrieval, int *status) {
  ArvBuffer *buffer = NULL;
  ArvStream *stream = NULL;
  gint payload;
  *status = SUCCESS;

  g_return_val_if_fail(ARV_IS_CAMERA(arvCamera), NULL);

  payload = arv_camera_get_payload(arvCamera);
  stream = arv_camera_create_stream(arvCamera, NULL, NULL);
  if (NULL != stream) {
    arv_stream_push_buffer(stream, arv_buffer_new(payload, NULL));
    arv_camera_start_acquisition(arvCamera);
    
    arv_camera_software_trigger(arvCamera);
    cout << "triggered" << endl;
    if (toggleDataRetrieval) {
      buffer = arv_stream_timeout_pop_buffer(stream, timeout);
      if (buffer != NULL) cout << "buffer" << endl;
      else cout << "no buffer" << endl;
    }
    else {
      usleep(timeout);
    }
    arv_camera_stop_acquisition(arvCamera);
    g_object_unref(stream);
  }
  else {
    *status = ERROR_STREAM_CREATION_FAILED;
  }
  return buffer;
}

ArvCamera* Camera::getArvInstance() { return arvCamera; }
 


