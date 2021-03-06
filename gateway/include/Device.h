/*
 * Device.h
 *
 *  Created on: Mar 28, 2016
 *      Author: James Hong
 */

#ifndef INCLUDE_DEVICE_H_
#define INCLUDE_DEVICE_H_

#include <atomic>
#include <cstdint>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <set>

#include "BeetleTypes.h"

/* Forward declarations */
class HandleAllocationTable;

class DeviceException : public std::exception {
public:
	DeviceException(std::string msg) : msg(msg) {};
	DeviceException(const char *msg) : msg(msg) {};
	~DeviceException() throw() {};
	const char *what() const throw() { return this->msg.c_str(); };
private:
	std::string msg;
};

class Device {
public:

	enum DeviceType {
		UNKNOWN = -1,
		BEETLE_INTERNAL = 0,
		LE_PERIPHERAL = 1,
		LE_CENTRAL = 2,
		TCP_CLIENT = 3,
		IPC_APPLICATION = 4,
		TCP_CLIENT_PROXY = 5,
		TCP_SERVER_PROXY = 6,
	};

	virtual ~Device();

	/*
	 * Get the device's unique identifier for this connection
	 * instance.
	 */
	device_t getId();

	/*
	 * Returns the device name if it has been set.
	 */
	std::string getName();

	/*
	 * Returns a enum device type.
	 */
	DeviceType getType();

	/*
	 * Returns a enum device type.
	 */
	std::string getTypeStr();

	/*
	 * Return largest, untranslated handle.
	 */
	int getHighestHandle();

	/*
	 * Handles that this device exposes to other devices as a server.
	 */
	std::map<uint16_t, std::shared_ptr<Handle>> handles;
	std::recursive_mutex handlesMutex;

	/*
	 * Handle address offsets that this device is a client to.
	 */
	std::unique_ptr<HandleAllocationTable> hat;
	std::mutex hatMutex;

	/*
	 * Devices that have this device in their client handle space.
	 */
	std::set<device_t> mappedTo;
	std::mutex mappedToMutex;

	/*
	 * Unsubscribe from all of this device's handles.
	 */
	void unsubscribeAll(device_t d);

	/*
	 * Enqueues a response. Returns whether the response was enqueued. Buf is owned by the caller and should not be
	 * freed.
	 */
	virtual void writeResponse(uint8_t *buf, int len) = 0;

	/*
	 * Enqueues a command. Returns whether the command was enqueued. Buf is owned by the caller and should not be
	 * freed.
	 */
	virtual void writeCommand(uint8_t *buf, int len) = 0;

	/*
	 * Enqueues a transaction. The callback is called when the response is received.
	 * The pointers passed to cb do not persist after cb is done. Returns whether
	 * the transaction was enqueued. On error the first argument to cb is NULL.
	 *
	 * Buf is owned by the caller and should not be freed.
	 */
	virtual void writeTransaction(uint8_t *buf, int len, std::function<void(uint8_t*, int)> cb) = 0;

	/*
	 * Blocks until the transaction finishes. Resp is set and must be freed by the caller.
	 * Returns the length of resp. On error, resp is NULL.
	 *
	 * Buf is owned by the caller and should not be freed. Response is written by reference to resp, and length is
	 * returned.
	 */
	virtual int writeTransactionBlocking(uint8_t *buf, int len, uint8_t *&resp) = 0;

	/*
	 * The largest packet this device can receive. Beetle will
	 * only ever advertise the default ATT_MTU, but larger MTU
	 * can allow faster handle discovery.
	 */
	virtual int getMTU() = 0;
protected:
	/*
	 * Cannot instantiate a device directly.
	 */
	Device(Beetle &beetle, HandleAllocationTable *hat = NULL);
	Device(Beetle &beetle, device_t id, HandleAllocationTable *hat = NULL);

	Beetle &beetle;

	std::string name;
	DeviceType type;
private:
	device_t id;

	static std::atomic_int idCounter;
	static const std::string deviceType2Str[];
};

#endif /* INCLUDE_DEVICE_H_ */
