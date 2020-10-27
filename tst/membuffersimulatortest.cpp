#include <stdio.h>
#include "../src/simulation/membuffersimulator.h"
#include <unistd.h>
#include <catch2/catch.hpp>


TEST_CASE("sim1", "[simulator]") {
	
	MemBufferSimulator simulator = MemBufferSimulator(100000);
	char data[64000];
	for (int i = 0; i < 64000; i++) {
		data[i] = 0;
	}
	int status = -1;

	simulator.writeToBuffer(data, 64000, 0);
	while (true) {
		status = simulator.getBufferStatus(0);
		printf("addr 0 %d\n", status);
		if (status == MEM_STATUS_COMPLETED) break;
		sleep(1);
	}

	REQUIRE(3 == 3);
	/*
	simulator.writeToBuffer(data, 64000, 0);
	simulator.writeToBuffer(data, 64000, 1);
	simulator.writeToBuffer(data, 64000, 0);
	simulator.writeToBuffer(data, 64000, 1);
	*/
}

