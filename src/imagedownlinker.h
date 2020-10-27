#ifndef IMAGEDOWNLINKER_H
#define IMAGEDOWNLINKER_H

#include "arv.h"
#include <thread>
#include <vector>

#define NUM_DOWNLINK_BUFFERS 2

#define DOWNLINK_STATUS_EMPTY 10
#define DOWNLINK_STATUS_NOT_STARTED 11
#define DOWNLINK_STATUS_IN_PROGRESS 12

using namespace std;

typedef struct {
	ArvBuffer *arvBuffer;
	int status;
} DownlinkBuffer;

class ImageDownlinker {
public: 
	// constructor/destructor
	ImageDownlinker();
	~ImageDownlinker();

	int addImageToDownlinkQueue(ArvBuffer *buffer);

private:
	int getIndexOfAvailableBuffer

	// fields
	DownlinkBuffer buffers[NUM_DOWNLINK_BUFFERS];
	vector<thread> handlers;
	bool bufferIsAvailable;
};

#endif
