#include <time.h>
#include <png.h> // Requires libpng1.2
#include <iostream>
#include <stdlib.h>

#include "arv.h"
#include "../config.h"

// modify these as necessary to fill up all bytes for downlink packet size
#define CHECKSUM_SIZE 32
#define HEADER_PACKET_DATA_MAX_SIZE 63927
#define DATA_PACKET_DATA_MAX_SIZE 63939

#define PACKET_TYPE_HEADER 0
#define PACKET_TYPE_DATA 1

using namespace std;

struct HEADER {
	char packetType;
	int packetId;
	time_t timestamp;
	unsigned int numDataPackets;
	unsigned int imageHeight;
	unsigned int imageWidth;
	unsigned int bitDepth;
	char checksum[CHECKSUM_SIZE];
	unsigned int dataSize;
	char data[HEADER_PACKET_DATA_MAX_SIZE];
};

struct DATA {
	char packetType;
	int packetId;
	unsigned int packetNum;
	time_t timestamp;
	char checksum[CHECKSUM_SIZE];
	unsigned int dataSize;
	char data[DATA_PACKET_DATA_MAX_SIZE];
};

int convertBufferToPackets(ArvBuffer *buffer, HEADER **header, DATA **dataPackets) {
	if (!ARV_IS_BUFFER(buffer)) return ERROR_BAD_BUFFER;

	size_t bufferSize = 0;
	char * bufferData = (char*)arv_buffer_get_data(buffer, &bufferSize); 
	int width = -1; int height = -1;
	arv_buffer_get_image_region(buffer, NULL, NULL, &width, &height); 
	int bitDepth = ARV_PIXEL_FORMAT_BIT_PER_PIXEL(arv_buffer_get_image_pixel_format(buffer)); 

	//cout << "buffer size " << buffer_size << endl;
	//cout << width * height * (bit_depth / 8) << endl;

	int lastDataPacketSize = DATA_PACKET_DATA_MAX_SIZE;
	int numDataPackets = (bufferSize - HEADER_PACKET_DATA_MAX_SIZE) / DATA_PACKET_DATA_MAX_SIZE;
	if (bufferSize < HEADER_PACKET_DATA_MAX_SIZE) {
		numDataPackets = 0;
	} else if ((lastDataPacketSize = ((bufferSize - HEADER_PACKET_DATA_MAX_SIZE) % DATA_PACKET_DATA_MAX_SIZE)) != 0) {
		numDataPackets++;
	}

	time_t timestamp = time(NULL);
	srand(timestamp);

	*header = (HEADER*) malloc(sizeof(HEADER));
	header->packetType = PACKET_TYPE_HEADER;
	header->packetId = rand();
	header->timestamp = timestamp;
	header->numDataPackets = numDataPackets;
	header->imageHeight = height;
	header->imageWidth = width;
	header->bitDepth = bitDepth;
	header->checksum = "I'm a checksum\0";
	header->dataSize = numDataPackets == 0 ? bufferSize : HEADER_PACKET_DATA_MAX_SIZE;
	memcpy(header->data, bufferData, header->dataSize);

	*dataPackets = (DATA*) malloc(numDataPackets * sizeof(DATA));
	for (int i = 0; i < numDataPackets; i++) {
		dataPackets[i].packetType = PACKET_TYPE_DATA;
		dataPackets[i].packetId = header->packetId;
		dataPackets[i].packetNum = i + 1;
		dataPackets[i].timestamp = timestamp;
		dataPackets[i].checksum = "I'm a checksum\0";
		dataPackets[i].dataSize = i == numDataPackets - 1 ? lastDataPacketSize : DATA_PACKET_DATA_MAX_SIZE;
		memcpy(dataPackets[i].data, bufferData + HEADER_PACKET_DATA_MAX_SIZE + DATA_PACKET_DATA_MAX_SIZE * i, dataPackets[i].dataSize);
	}

	return SUCCESS;
}

void reconstructPacketsToBuffer(HEADER header, DATA *dataPackets, char **data, size_t *dataSize) {
	*dataSize = header.height * header.width * header.bitDepth / 8;
	*data = (char *) malloc(*dataSize * sizeof(char));
	memcpy(*data, header.data, header.dataSize);

	for (int i = 0; i < numDataPackets; i++) {
		memcpy(*data + HEADER_PACKET_DATA_MAX_SIZE + DATA_PACKET_DATA_MAX_SIZE * i, dataPackets[i].data, dataPackets[i].dataSize);
	}
}