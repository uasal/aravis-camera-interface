#include "packetmanager.h"
#include "../simulation/membuffersimulator.h"
#include "../downlink/packetinterface.hpp"
#include "../config.h"
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define MEMORY_WAIT_TIME 0

using namespace std;

sem_t mutex, memoryAddrReady[NUM_DOWNLINK_BUFFERS];

bool quit = false;
ArvBuffer *currentBuffer[2];
PacketManager *thisObj = NULL;

typedef struct {
	int index;
} WorkerArgs;

WorkerArgs args[NUM_DOWNLINK_BUFFERS];

void* handler(void *arg) {
	WorkerArgs *args = (WorkerArgs*) arg;
	int rc = -1;
		
	HEADER header;
	vector<DATA> dataPackets;

	while (true) {
		sem_wait(&memoryAddrReady[args->index]);
		sem_wait(&mutex);
		
		if (quit) {
			sem_post(&mutex);
			return NULL;
		}
		ArvBuffer *buffer = currentBuffer[args->index];
		sem_post(&mutex);

		rc = convertBufferToPackets(buffer, &header, &dataPackets);

		char bytes[BUFFER_SIZE];
		memcpy(bytes, header, BUFFER_SIZE);

		thisObj->writePacket(bytes, BUFFER_SIZE, args->index);

		for (int i = 0; i < header->numDataPackets; i++) {
			thisObj->writePacket(dataPackets[i], BUFFER_SIZE, args->index);
		}

		sem_wait(&mutex);
		currentBuffer[args->index] = NULL;
		sem_post(&mutex);

		thisObj->readyNextBuffer();
	}

	return NULL;
}

PacketManager::PacketManager() {
	sem_init(&mutex, 0, 1);
	int rc = -1;

	quit = false;
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		sem_init(&memoryAddrReady[i], 0, 0);
		currentBuffer[i] = NULL;

		args[i].index = i;
		thisObj = this;
		rc = pthread_create(&workers[i], NULL, handler, (void*) &args[i]);
		if (rc != 0) printf("failed\n");
	}
}

PacketManager::~PacketManager() {
	sem_wait(&mutex);
	quit = true;
	sem_post(&mutex);
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		sem_post(&memoryAddrReady[i]);
	}
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		pthread_join(workers[i], NULL);
	}
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		sem_destroy(&memoryAddrReady[i]);
	}
	
	memory = MemBufferSimulator(0);
	sem_destroy(&mutex);
}

int PacketManager::writeBufferToMemory(ArvBuffer *buffer) {
	sem_wait(&mutex);

	if (bufferQueue.size >= MAX_QUEUE_SIZE) {
		sem_post(&mutex);
		return BUFFER_QUEUE_FULL;
	}

	bufferQueue.push(buffer);
	sem_post(&mutex);

	readyNextBuffer();

	return MEM_WRITE_SUCCESS;
}

int PacketManager::writePacket(char *data, size_t size, int addr) {
	rc = memory.writeToBuffer(data, size, addr);
	if (SUCCESS != rc) { // TODO: add more sophisticated error checking
		printf("this is a bug\n");
		return rc;
	}

	while (memory.getBufferStatus(addr) != MEM_STATUS_COMPLETED) { // TODO: update for more details on rincon API
		usleep(MEMORY_WAIT_TIME);	
	}

	return SUCCESS;
}

void PacketManager::readyNextBuffer() {
	sem_wait(&mutex);

	if (bufferQueue.empty()) {
		sem_post(&mutex);
		return;
	}

	int freeIndex = 0;
	for (freeIndex = 0; freeIndex < NUM_DOWNLINK_BUFFERS; freeIndex++) {
		if (NULL == currentBuffer) break;
	}

	if (NUM_DOWNLINK_BUFFERS == freeIndex) {
		sem_post(&mutex);
		return;
	}

	currentBuffer[freeIndex] = bufferQueue.pop();
	sem_post(&memoryAddrReady[freeIndex]);

	sem_post(&mutex);
}

