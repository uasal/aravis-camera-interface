#ifndef CONFIG_H
#define CONFIG_H

// default camera config
#define DEFAULT_ETHERNET_PACKET_SIZE 1500
#define DEFAULT_CAMERA_WINDOW_WIDTH 1280
#define DEFAULT_CAMERA_WINDOW_HEIGHT 1024 
#define DEFAULT_CAMERA_FRAME_RATE 10.0
#define DEFAULT_CAMERA_GAIN 1.0
#define FEATURE_NOT_DEFINED -666

// error codes
#define SUCCESS 0
#define ERROR_CAMERA_NOT_FOUND 1
#define ERROR_COMMAND_LINE_PARSE_FAILED 2
#define ERROR_STREAM_CREATION_FAILED 3
#define ERROR_FEATURE_OUT_OF_BOUNDS 4
#define ERROR_FEATURE_WRITE_FAILED 5
#endif
