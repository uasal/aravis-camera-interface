#include <time.h>
#include <png.h> // Requires libpng1.2
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <string.h>

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
	unsigned int imageBitDepth;
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

int convertBufferToPackets(ArvBuffer *buffer, HEADER *header, vector<DATA> *dataPackets) {
	if (!ARV_IS_BUFFER(buffer)) return ERROR_BAD_BUFFER;

	size_t bufferSize = 0;
	char * bufferData = (char*)arv_buffer_get_data(buffer, &bufferSize); 
	int width = -1; int height = -1;
	arv_buffer_get_image_region(buffer, NULL, NULL, &width, &height); 
	int bitDepth = ARV_PIXEL_FORMAT_BIT_PER_PIXEL(arv_buffer_get_image_pixel_format(buffer));

	int lastDataPacketSize = DATA_PACKET_DATA_MAX_SIZE;
	int numDataPackets = (bufferSize - HEADER_PACKET_DATA_MAX_SIZE) / DATA_PACKET_DATA_MAX_SIZE;
	if (bufferSize < HEADER_PACKET_DATA_MAX_SIZE) {
		numDataPackets = 0;
	} else if ((lastDataPacketSize = ((bufferSize - HEADER_PACKET_DATA_MAX_SIZE) % DATA_PACKET_DATA_MAX_SIZE)) != 0) {
		numDataPackets++;
	}

	time_t timestamp = time(NULL);
	srand(timestamp);

	header->packetType = PACKET_TYPE_HEADER;
	header->packetId = rand();
	header->timestamp = timestamp;
	header->numDataPackets = numDataPackets;
	header->imageHeight = height;
	header->imageWidth = width;
	header->imageBitDepth = bitDepth;
	strcpy(header->checksum, "I'm a checksum\0"); // TODO: implement actual checksum calculation
	header->dataSize = numDataPackets == 0 ? bufferSize : HEADER_PACKET_DATA_MAX_SIZE;
	memcpy(header->data, bufferData, header->dataSize);

	dataPackets->clear();
	for (int i = 0; i < numDataPackets; i++) {
		DATA data;
		data.packetType = PACKET_TYPE_DATA;
		data.packetId = header->packetId;
		data.packetNum = i + 1;
		data.timestamp = timestamp;
		strcpy(data.checksum, "I'm a checksum\0");
		data.dataSize = i == numDataPackets - 1 ? lastDataPacketSize : DATA_PACKET_DATA_MAX_SIZE;
		memcpy(data.data, bufferData + HEADER_PACKET_DATA_MAX_SIZE + DATA_PACKET_DATA_MAX_SIZE * i, data.dataSize);
		dataPackets->push_back(data);
	}

	return SUCCESS;
}

void reconstructPacketsToBuffer(HEADER header, vector<DATA> dataPackets, char **data, size_t *dataSize) {
	*dataSize = header.imageHeight * header.imageWidth * header.imageBitDepth / 8;
	*data = (char *) malloc(*dataSize * sizeof(char));
	memcpy(*data, header.data, header.dataSize);

	for (int i = 0; i < header.numDataPackets; i++) {
		memcpy(*data + HEADER_PACKET_DATA_MAX_SIZE + DATA_PACKET_DATA_MAX_SIZE * i, dataPackets[i].data, dataPackets[i].dataSize);
	}
}
