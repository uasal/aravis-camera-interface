#include <stdio.h>
#include "../src/simulation/membuffersimulator.h"
#include <unistd.h>
#include <catch2/catch.hpp>
#define MAX_WAIT_SECONDS 5

TEST_CASE("test long latency", "[simulator]") {
	int latency = 2000000; // 2 seconds
	MemBufferSimulator simulator = MemBufferSimulator(latency);


	char data[64000];
	for (int i = 0; i < 64000; i++) {
		data[i] = 0;
	}

	int status0 = -1;
	int status1 = -1;

	simulator.writeToBuffer(data, 64000, 0);
	simulator.writeToBuffer(data, 64000, 1);

	int t = 0;
	for (t = 0; t < MAX_WAIT_SECONDS; t++) {
		status0 = simulator.getBufferStatus(0);
		status1 = simulator.getBufferStatus(1);
		if (status0 == MEM_STATUS_COMPLETED && status1 == MEM_STATUS_COMPLETED) break;
		sleep(1);
	}

	REQUIRE(t >= latency / 1000000);
}

TEST_CASE("test write to existing write", "[simulator][error]") {
	int latency = 2000000; // 2 seconds
	MemBufferSimulator simulator = MemBufferSimulator(latency);

	char data[64000];
	for (int i = 0; i < 64000; i++) {
		data[i] = 0;
	}

	int rc = -1;

	rc = simulator.writeToBuffer(data, 64000, 0);
	REQUIRE(MEM_WRITE_SUCCESS == rc);

	rc = simulator.writeToBuffer(data, 64000, 0);
	REQUIRE(MEM_WRITE_FAILURE == rc);

	rc = simulator.writeToBuffer(data, 64000, 1);
	REQUIRE(MEM_WRITE_SUCCESS == rc);

	rc = simulator.writeToBuffer(data, 64000, 1);
	REQUIRE(MEM_WRITE_FAILURE == rc);

}


TEST_CASE("test get buffer status", "[simulator]") {
	int latency = 1000;
	MemBufferSimulator simulator = MemBufferSimulator(latency);

	char data[64000];
	for (int i = 0; i < 64000; i++) {
		data[i] = 0;
	}

	int status =  -1;
	
	status = simulator.getBufferStatus(0);
	REQUIRE(MEM_STATUS_EMPTY == status);

	simulator.writeToBuffer(data, 64000, 0);

	int t = 0;
	for (t = 0; t < MAX_WAIT_SECONDS; t++) {
		status = simulator.getBufferStatus(0);
		if (MEM_STATUS_COMPLETED == status) break;
		sleep(1);
	}

	status = simulator.getBufferStatus(0);
	REQUIRE(MEM_STATUS_EMPTY == status);

}
