#ifndef PACKETINTERFACE_H
#define PACKETINTERFACE_H

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

int convertBufferToPackets(ArvBuffer *buffer, HEADER *header, vector<DATA> *dataPackets);
void reconstructPacketsToBuffer(HEADER header, vector<DATA> dataPackets, char **data, size_t *dataSize);


#endif