/**
 * membuffersimulator.cpp
 * Author: Bohan Li
 * 
 * Class implementing a simulation of ASDR memory buffers.
 * Functions by defining two downlink buffer arrays, for which
 * each has its own thread managing the IO into the array element.
 */

#include "membuffersimulator.h"
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

sem_t mutex, dataReady[NUM_DOWNLINK_BUFFERS];

char *sharedData[NUM_DOWNLINK_BUFFERS];
size_t sharedDataSize[NUM_DOWNLINK_BUFFERS];
bool quit = false;

MemBufferSimulator *thisObj = NULL;

typedef struct {
	int address;
} HandlerArgs;

HandlerArgs args[NUM_DOWNLINK_BUFFERS];

void* handler(void *arg) {
	HandlerArgs *args = (HandlerArgs*) arg;

	while (true) {
		sem_wait(&dataReady[args->address]);
		sem_wait(&mutex);

		if (quit) {
			sem_post(&mutex);
			return NULL;
		}

		sem_post(&mutex);

		sem_wait(&mutex);
		char *data = sharedData[args->address];
		size_t size = sharedDataSize[args->address];
		thisObj->status[0];
		thisObj->status[args->address] = MEM_STATUS_WRITING;
		sem_post(&mutex);
		usleep(thisObj->latency);

		sem_wait(&mutex);
		thisObj->status[args->address] = MEM_STATUS_COMPLETED;
		sem_post(&mutex);

	}

	return NULL;
}

MemBufferSimulator::MemBufferSimulator(long simulatedLatency) {
	sem_init(&mutex, 0, 1);
	latency = simulatedLatency;
	int rc = -1;

	quit = false;
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		sem_init(&dataReady[i], 0, 0);
		status[i] = MEM_STATUS_EMPTY;

		args[i].address = i;
		thisObj = this;
		rc = pthread_create(&handlers[i], NULL, handler, (void*) &args[i]);
		if (rc != 0) printf("failed\n");
	}
}

MemBufferSimulator::~MemBufferSimulator() {
	sem_wait(&mutex);
	quit = true;
	sem_post(&mutex);
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		sem_post(&dataReady[i]);
	}
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		pthread_join(handlers[i], NULL);
	}
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		sem_destroy(&dataReady[i]);
	}
	
	sem_destroy(&mutex);
}

int MemBufferSimulator::writeToBuffer(char *data, size_t size, int addr) {
	if (size > BUFFER_SIZE) {
		printf("Buffer size %d too big\n", size);
		return MEM_WRITE_FAILURE;
	}
	if (addr < 0 || addr >= NUM_DOWNLINK_BUFFERS) {
		printf("Invalid address\n");
		return MEM_WRITE_FAILURE;
	}
	
	sem_wait(&mutex);
	if (MEM_STATUS_NOT_STARTED == status[addr] || MEM_STATUS_WRITING == status[addr]) {
		printf("writing to buffer that is being written\n");
		sem_post(&mutex);
		return MEM_WRITE_FAILURE;
	}
	status[addr] = MEM_STATUS_NOT_STARTED;
	sharedData[addr] = data;
	sharedDataSize[addr] = size;
	sem_post(&dataReady[addr]);
	sem_post(&mutex);
	return MEM_WRITE_SUCCESS;
}

int MemBufferSimulator::getBufferStatus(int addr) {
	if (addr < 0 || addr >= NUM_DOWNLINK_BUFFERS) {
		printf("Invalid address\n");
		return MEM_INVALID_ADDRESS;
	}
	sem_wait(&mutex);
	int retstatus = status[addr];

	if (retstatus == MEM_STATUS_COMPLETED) {
		status[addr] = MEM_STATUS_EMPTY;
	}
	sem_post(&mutex);
	return retstatus;
}
