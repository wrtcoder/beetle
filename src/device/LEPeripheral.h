/*
 * LEPeripheral.h
 *
 *  Created on: Mar 28, 2016
 *      Author: james
 */

#ifndef LEPERIPHERAL_H_
#define LEPERIPHERAL_H_

#include <bluetooth/bluetooth.h>
#include <cstdint>
#include <string>
#include <thread>

#include "../sync/BlockingQueue.h"
#include "VirtualDevice.h"
#include "shared.h"

enum AddrType {
	PUBLIC, RANDOM,
};

class LEPeripheral: public VirtualDevice {
public:
	LEPeripheral(Beetle &beetle, bdaddr_t addr, AddrType addrType);
	virtual ~LEPeripheral();

	bdaddr_t getBdaddr() { return bdaddr; };
	AddrType getAddrType() { return bdaddrType; };
protected:
	bool write(uint8_t *buf, int len);
	void startInternal();
private:
	bdaddr_t bdaddr;
	AddrType bdaddrType;

	int sockfd;

	std::thread readThread;
	std::thread writeThread;

	BlockingQueue<queued_write_t> writeQueue;

	void readDaemon();
	void writeDaemon();
};

#endif /* LEPERIPHERAL_H_ */
