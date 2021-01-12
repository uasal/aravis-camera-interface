#include "../src/hdcamera.h"
#include "../src/config.h"
#include "../src/downlink/packetinterface.hpp"
#include "../src/simulation/membuffersimulator.h"

#include <stdio.h>
#include <arv.h>
#include <catch2/catch.hpp>

TEST_CASE("packet size validation", "[downlink][packet]") {
	REQUIRE(BUFFER_SIZE == sizeof(HEADER));
	REQUIRE(BUFFER_SIZE == sizeof(DATA));
}

TEST_CASE("downlink", "[camera]") {
	int status = -1;
	HDCamera camera = HDCamera(&status, DEFAULT_ETHERNET_PACKET_SIZE);

	REQUIRE(SUCCESS == status);

	ArvBuffer *buffer = NULL;
	status = camera.getSnapshot(10 * DEFAULT_CAMERA_SNAPSHOT_TIMEOUT, &buffer); // extra time needed, possibly for catch overhead

	REQUIRE(SUCCESS == status);
	REQUIRE(NULL != buffer); // if this fails, there may be firewall issues or more time is needed for the snapshot

	convertBufferToPackets(buffer, NULL, NULL);
}