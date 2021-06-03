/**
 * packetinterface.h
 * Author: Bohan Li
 * 
 * Header file defining data format for downlink. Each data buffer
 * will be packaged into a HEADER packet and several DATA packets.
 * The HEADER packet will contain all metadata relating to the image
 * and will define the number of DATA packets.
 */

#ifndef PACKETINTERFACE_H
#define PACKETINTERFACE_H

#include <time.h>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "arv.h"
#include "../config.h"

/*
	TODO: MODIFY THESE FOR INTEGRATION
	
	We assume here that the buffer size is 64000 bytes,
	modify these as necessary to fill up all bytes for downlink packet size
*/
#define HEADER_PACKET_DATA_MAX_SIZE 63959
#define DATA_PACKET_DATA_MAX_SIZE 63963


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
	unsigned int dataSize;
	char data[HEADER_PACKET_DATA_MAX_SIZE];
	uint32_t checksum;
};

struct DATA {
	char packetType;
	int packetId;
	unsigned int packetNum;
	time_t timestamp;
	unsigned int dataSize;
	char data[DATA_PACKET_DATA_MAX_SIZE];
	uint32_t checksum;
};

uint32_t crc32(const void *data, size_t n_bytes);
int convertBufferToPackets(ArvBuffer *buffer, HEADER *header, vector<DATA> *dataPackets, unsigned int packetNum);
void reconstructPacketsToBuffer(HEADER header, vector<DATA> dataPackets, char **data, size_t *dataSize);


#endif