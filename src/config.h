#ifndef CONFIG_H
#define CONFIG_H

// default camera config
#define DEFAULT_ETHERNET_PACKET_SIZE 1500
#define DEFAULT_CAMERA_WINDOW_WIDTH 640
#define DEFAULT_CAMERA_WINDOW_HEIGHT 512
#define DEFAULT_CAMERA_FRAME_RATE 10.0
#define DEFAULT_CAMERA_GAIN 1.0
#define DEFAULT_CAMERA_X_OFFSET 0
#define DEFAULT_CAMERA_Y_OFFSET 0
#define DEFAULT_CAMERA_EXPOSURE_TIME 5000
#define DEFAULT_CAMERA_PIXEL_FORMAT "Mono8"
#define DEFAULT_CAMERA_SNAPSHOT_TIMEOUT 1000000
#define FEATURE_NOT_DEFINED 12345678


// error codes
#define SUCCESS 0
#define ERROR_CAMERA_NOT_FOUND 1
#define ERROR_COMMAND_LINE_PARSE_FAILED 2
#define ERROR_STREAM_START_FAILED 3
#define ERROR_FEATURE_OUT_OF_BOUNDS 4
#define ERROR_FEATURE_WRITE_FAILED 5
#define ERROR_SNAPSHOT_FAILED 6
#define ERROR_STREAM_DURATION_UNDEFINED 7
#define ERROR_IMAGE_WRITE_FAILURE 8
#define ERROR_FILESYSTEM_FAILURE 9
#endif