/**
 * packetmanager.cpp
 * Author: Bohan Li
 *
 * This class implements the interface to the
 * memory buffers on the ASDR. For each buffer
 * on the ASDR, this class maintains a thread
 * that handles IO into the buffer. 
 *
 * The packet manager maintains a queue of image buffers
 * as more buffers are written, selecting an open memory
 * memory buffer as it becomes freed.
 */


#include "packetmanager.h"
#include "../simulation/membuffersimulator.h"
#include "../downlink/packetinterface.h"
#include "../config.h"
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define MEMORY_WAIT_TIME 0

using namespace std;

// here memoryAddrReady contains semaphores that block until
// a buffer is being written to that particular buffer
sem_t pmMutex, memoryAddrReady[NUM_DOWNLINK_BUFFERS];

bool pmQuit = false;
ArvBuffer *currentBuffer[2];
PacketManager *thisPmObj = NULL;
unsigned int packetCounter = 0;

typedef struct {
	int index;
} WorkerArgs;

WorkerArgs wargs[NUM_DOWNLINK_BUFFERS];

/* Handler thread for a buffer indicated by its arg */
void* worker(void *arg) {
	WorkerArgs *args = (WorkerArgs*) arg;
	int rc = -1;
		
	HEADER header;
	vector<DATA> dataPackets;

	while (true) {
		sem_wait(&memoryAddrReady[args->index]);
		sem_wait(&pmMutex);
		
		if (pmQuit) {
			sem_post(&pmMutex);
			return NULL;
		}
		ArvBuffer *buffer = currentBuffer[args->index];
		sem_post(&pmMutex);

		rc = convertBufferToPackets(buffer, &header, &dataPackets, packetCounter);

		char bytes[BUFFER_SIZE];
		memcpy(bytes, &header, BUFFER_SIZE);

		thisPmObj->writePacket(bytes, BUFFER_SIZE, args->index);

		for (int i = 0; i < header.numDataPackets; i++) {
			memcpy(bytes, &dataPackets[i], BUFFER_SIZE);
			thisPmObj->writePacket(bytes, BUFFER_SIZE, args->index);
		}

		cout << "wrote buffer " << header.packetId << endl;

		sem_wait(&pmMutex);
		currentBuffer[args->index] = NULL;
		packetCounter++;
		sem_post(&pmMutex);

		thisPmObj->readyNextBuffer();
	}

	return NULL;
}


PacketManager::PacketManager() {
	sem_init(&pmMutex, 0, 1);
	int rc = -1;

	pmQuit = false;
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		sem_init(&memoryAddrReady[i], 0, 0);
		currentBuffer[i] = NULL;

		wargs[i].index = i;
		thisPmObj = this;
		rc = pthread_create(&workers[i], NULL, worker, (void*) &wargs[i]);
		if (rc != 0) printf("failed\n");
	}
}

PacketManager::~PacketManager() {
	sem_wait(&pmMutex);
	pmQuit = true;
	sem_post(&pmMutex);
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		sem_post(&memoryAddrReady[i]);
	}
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		pthread_join(workers[i], NULL);
	}
	for (int i = 0; i < NUM_DOWNLINK_BUFFERS; i++) {
		sem_destroy(&memoryAddrReady[i]);
	}
	
	sem_destroy(&pmMutex);
}

int PacketManager::writeBufferToMemory(ArvBuffer *buffer) {
	sem_wait(&pmMutex);

	if (bufferQueue.size() >= MAX_QUEUE_SIZE) {
		sem_post(&pmMutex);
		return BUFFER_QUEUE_FULL;
	}

	bufferQueue.push(buffer);
	sem_post(&pmMutex);

	readyNextBuffer();

	return MEM_WRITE_SUCCESS;
}


int PacketManager::writePacket(char *data, size_t size, int addr) {
	int rc = memory.writeToBuffer(data, size, addr);
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
	sem_wait(&pmMutex);

	if (bufferQueue.empty()) {
		sem_post(&pmMutex);
		return;
	}

	int freeIndex = 0;
	for (freeIndex = 0; freeIndex < NUM_DOWNLINK_BUFFERS; freeIndex++) {
		if (NULL == currentBuffer[freeIndex]) break;
	}

	if (NUM_DOWNLINK_BUFFERS == freeIndex) {
		sem_post(&pmMutex);
		return;
	}

	currentBuffer[freeIndex] = bufferQueue.front();
	bufferQueue.pop();
	sem_post(&memoryAddrReady[freeIndex]);

	sem_post(&pmMutex);
}

