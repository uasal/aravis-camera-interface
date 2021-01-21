#ifndef PACKETMANAGER_H
#define PACKETMANAGER_H

#include <stdlib.h>
#include "../simulation/membuffersimulator.h"
#include "arv.h"
#include <queue>

#define MAX_QUEUE_SIZE 10

using namespace std;

class PacketManager {
public: 
	// constructor/destructor
	PacketManager();
	~PacketManager();

	int writeBufferToMemory(ArvBuffer *buffer);

	int status[NUM_DOWNLINK_BUFFERS];
	
private:
	pthread_t workers[NUM_DOWNLINK_BUFFERS];
	queue<ArvBuffer> bufferQueue;
	MemBufferSimulator memory;

	int writePacket(char *data, size_t size, int addr);
	void readyNextBuffer();
};

#endif
