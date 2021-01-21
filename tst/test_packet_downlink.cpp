#include "../src/hdcamera.h"
#include "../src/config.h"
#include "../src/downlink/packetinterface.h"
#include "../src/simulation/membuffersimulator.h"

#include "glib.h"
#include <stdio.h>
#include <stdlib.h>
#include <arv.h>
#include <catch2/catch.hpp>
#include <vector>

TEST_CASE("packet size validation", "[packet]") {
	REQUIRE(BUFFER_SIZE == sizeof(HEADER));
	REQUIRE(BUFFER_SIZE == sizeof(DATA));
}

/* Testcases with the [camera] tag require the ethernet camera to be plugged in */

TEST_CASE("standard downlink", "[camera][packet]") {
	int status = -1;
	HDCamera camera = HDCamera(&status, DEFAULT_ETHERNET_PACKET_SIZE);
	REQUIRE(SUCCESS == status);

	status = camera.setRegion(DEFAULT_CAMERA_X_OFFSET, DEFAULT_CAMERA_Y_OFFSET, DEFAULT_CAMERA_WINDOW_WIDTH, DEFAULT_CAMERA_WINDOW_HEIGHT);
	REQUIRE(SUCCESS == status);

	ArvBuffer *buffer = NULL;
	status = camera.getSnapshot(10 * DEFAULT_CAMERA_SNAPSHOT_TIMEOUT, &buffer); // extra time needed, possibly for catch overhead

	REQUIRE(SUCCESS == status);
	REQUIRE(NULL != buffer); // if this fails, there may be firewall issues or more time is needed for the snapshot

	int width = -1; int height = -1;
	arv_buffer_get_image_region(buffer, NULL, NULL, &width, &height); 
	int bitDepth = ARV_PIXEL_FORMAT_BIT_PER_PIXEL(arv_buffer_get_image_pixel_format(buffer));

	size_t bufferSize = 0;
	char * bufferData = (char*)arv_buffer_get_data(buffer, &bufferSize); 

	HEADER header;
	vector<DATA> dataPackets;

	status = convertBufferToPackets(buffer, &header, &dataPackets);
	REQUIRE(SUCCESS == status);
	REQUIRE(header.numDataPackets == dataPackets.size());

	char *reconstructedBuffer = NULL;
	size_t reconstructedBufferSize = 0;

	reconstructPacketsToBuffer(header, dataPackets, &reconstructedBuffer, &reconstructedBufferSize);

	REQUIRE(width * height * bitDepth / 8 == reconstructedBufferSize);
	REQUIRE(reconstructedBufferSize	== bufferSize);

	for (int i = 0; i < bufferSize; i++) {
		REQUIRE(reconstructedBuffer[i] == bufferData[i]);
	}

	g_clear_object(&buffer);
	free(reconstructedBuffer);
}


TEST_CASE("downlink small image", "[camera][packet]") {
	int smallImageWidth = 50,
		smallImageHeight = 60;

	int status = -1;
	HDCamera camera = HDCamera(&status, DEFAULT_ETHERNET_PACKET_SIZE);
	REQUIRE(SUCCESS == status);

	status = camera.setRegion(DEFAULT_CAMERA_X_OFFSET, DEFAULT_CAMERA_Y_OFFSET, smallImageWidth, smallImageHeight);
	REQUIRE(SUCCESS == status);
	
	ArvBuffer *buffer = NULL;
	status = camera.getSnapshot(10 * DEFAULT_CAMERA_SNAPSHOT_TIMEOUT, &buffer); // extra time needed, possibly for catch overhead

	REQUIRE(SUCCESS == status);
	REQUIRE(NULL != buffer); // if this fails, there may be firewall issues or more time is needed for the snapshot

	int width = -1; int height = -1;
	arv_buffer_get_image_region(buffer, NULL, NULL, &width, &height); 
	int bitDepth = ARV_PIXEL_FORMAT_BIT_PER_PIXEL(arv_buffer_get_image_pixel_format(buffer));

	size_t bufferSize = 0;
	char * bufferData = (char*)arv_buffer_get_data(buffer, &bufferSize); 

	REQUIRE(width == smallImageWidth);
	REQUIRE(height == smallImageHeight);

	HEADER header;
	vector<DATA> dataPackets;

	status = convertBufferToPackets(buffer, &header, &dataPackets);
	REQUIRE(SUCCESS == status);
	REQUIRE(header.numDataPackets == dataPackets.size());

	char *reconstructedBuffer = NULL;
	size_t reconstructedBufferSize = 0;

	reconstructPacketsToBuffer(header, dataPackets, &reconstructedBuffer, &reconstructedBufferSize);

	REQUIRE(width * height * bitDepth / 8 == reconstructedBufferSize);
	REQUIRE(reconstructedBufferSize	== bufferSize);

	for (int i = 0; i < bufferSize; i++) {
		REQUIRE(reconstructedBuffer[i] == bufferData[i]);
	}

	g_clear_object(&buffer);
	free(reconstructedBuffer);
}