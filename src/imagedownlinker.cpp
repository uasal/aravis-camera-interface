#include "arv.h"
#include <thread>
#include <vector>
#include "imagedownlinker.h"

using namespace std;

void handler(ImageDownlinker downlinker) {

}

ImageDownlinker::ImageDownlinker() {
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		buffers[i].arvBuffer = NULL;
		buffers[i].status = DOWNLINK_STATUS_EMPTY;
	}
	bufferIsAvailable = TRUE;
}

ImageDownlinker::~ImageDownlinker() {}

int ImageDownlinker::addImageToDownlinkQueue(ArvBuffer *buffer) {
	
}
