#include "packetInterface.h"
#include <time.h>
#include <png.h> // Requires libpng1.2
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "arv.h"
#include "../config.h"


// checksum implementation from http://home.thep.lu.se/~bjorn/crc/
uint32_t crc32_for_byte(uint32_t r) {
	for(int j = 0; j < 8; ++j) {
		r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
	}
	return r ^ (uint32_t)0xFF000000L;
}

uint32_t crc32(const void *data, size_t n_bytes) {
	uint32_t crc = 0;
	static uint32_t table[0x100];
	
	if(!*table) {
		for(size_t i = 0; i < 0x100; ++i) {
			table[i] = crc32_for_byte(i);
		}
	}

	for(size_t i = 0; i < n_bytes; ++i) {
		crc = table[(uint8_t) crc ^ ((uint8_t*)data)[i]] ^ crc >> 8;
	}

	return crc;
}

int convertBufferToPackets(ArvBuffer *buffer, HEADER *header, vector<DATA> *dataPackets, unsigned int packetNum) {
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
	header->packetType = PACKET_TYPE_HEADER;
	header->packetId = packetNum;
	cout << "header " << header->packetId << endl;
	header->timestamp = timestamp;
	header->numDataPackets = numDataPackets;
	header->imageHeight = height;
	header->imageWidth = width;
	header->imageBitDepth = bitDepth;
	header->checksum = 0; // TODO: implement actual checksum calculation
	header->dataSize = numDataPackets == 0 ? bufferSize : HEADER_PACKET_DATA_MAX_SIZE;
	memcpy(header->data, bufferData, header->dataSize);

	dataPackets->clear();
	for (int i = 0; i < numDataPackets; i++) {
		DATA data;
		data.packetType = PACKET_TYPE_DATA;
		data.packetId = header->packetId;
		data.packetNum = i + 1;
		data.timestamp = timestamp;
		data.checksum = 0;
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
