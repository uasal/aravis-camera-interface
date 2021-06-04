/**
 * membuffersimulator.h
 * Author: Bohan Li
 * 
 * Header for simulator class for ASDR memory buffer.
 */

#ifndef MEMBUFFERSIMULATOR_H
#define MEMBUFFERSIMULATOR_H

#include <stdlib.h>

#define NUM_DOWNLINK_BUFFERS 2
#define BUFFER_SIZE 64000 // bytes

#define MEM_STATUS_EMPTY -1
#define MEM_STATUS_NOT_STARTED 0
#define MEM_STATUS_WRITING 1
#define MEM_STATUS_COMPLETED 2

#define MEM_WRITE_SUCCESS 0
#define MEM_WRITE_FAILURE 1
#define MEM_INVALID_ADDRESS 2

using namespace std;

class MemBufferSimulator {
public: 
	// constructor/destructor
	MemBufferSimulator(long latency);
	~MemBufferSimulator();

	int writeToBuffer(char *data, size_t size, int addr);
	int getBufferStatus(int addr);

	long latency;
	int status[NUM_DOWNLINK_BUFFERS];
private:
	pthread_t handlers[NUM_DOWNLINK_BUFFERS];
};

#endif
